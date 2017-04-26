#include "common.h"
#include "trainer.h"
#include "alias_table.h"
#include "data_stream.h"
#include "data_block.h"
#include "document.h"
#include "meta.h"
#include "util.h"
#include <vector>
#include <iostream>
#include <multiverso/barrier.h>
#include <multiverso/log.h>
#include <multiverso/row.h>

#include <chrono>
#include <ctime>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/pool.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/exception/exception.hpp>

namespace multiverso { namespace lightlda
{     
    class LightLDA
    {
    public:
        static void Run(int argc, char** argv)
        {
            Config::Init(argc, argv);
            mongo_uri_ = Config::mongo_uri;
            AliasTable* alias_table = new AliasTable();
            Barrier* barrier = new Barrier(Config::num_local_workers);
            meta.SetMongoParameters(mongo_uri_,"test","vocabCollection");
            meta.Init();
            std::vector<TrainerBase*> trainers;
            for (int32_t i = 0; i < Config::num_local_workers; ++i)
            {
                Trainer* trainer = new Trainer(alias_table, barrier, &meta);
                trainers.push_back(trainer);
            }

            ParamLoader* param_loader = new ParamLoader();
            multiverso::Config config;
            config.num_servers = Config::num_servers;
            config.num_aggregator = Config::num_aggregator;
            config.server_endpoint_file = Config::server_file;
            config.output_dir = Config::output_dir;


            Multiverso::Init(trainers, param_loader, config, &argc, &argv);

            Log::ResetLogFile("LightLDA."
                + std::to_string(clock()) + ".log");

            data_stream = CreateDataStream();
            InitMultiverso();
            Train();

            Multiverso::Close();
            
            for (auto& trainer : trainers)
            {
                delete trainer;
            }
            delete param_loader;
            
            DumpDocTopic();

            delete data_stream;
            delete barrier;
            delete alias_table;
        }
    private:
        static void Train()
        {
            Multiverso::BeginTrain();
            for (int32_t i = 0; i < Config::num_iterations; ++i)
            {
                Multiverso::BeginClock();
                // Train corpus block by block
                for (int32_t block = 0; block < Config::num_blocks; ++block)
                {
                    data_stream->BeforeDataAccess();
                    DataBlock& data_block = data_stream->CurrDataBlock();
                    data_block.set_meta(&meta.local_vocab(block));
                    int32_t num_slice = meta.local_vocab(block).num_slice();
                    std::vector<LDADataBlock> data(num_slice);
                    // Train datablock slice by slice
                    for (int32_t slice = 0; slice < num_slice; ++slice)
                    {
                        LDADataBlock* lda_block = &data[slice];
                        lda_block->set_data(&data_block);
                        lda_block->set_iteration(i);
                        lda_block->set_block(block);
                        lda_block->set_slice(slice);
                        Multiverso::PushDataBlock(lda_block);
                    }
                    Multiverso::Wait();
                    data_stream->EndDataAccess();
                }
                Multiverso::EndClock();
            }
            Multiverso::EndTrain();
        }

        static void InitMultiverso()
        {
            Multiverso::BeginConfig();
            CreateTable();
            ConfigTable();
            Initialize();
            Multiverso::EndConfig();
        }

        static void Initialize()
        {
            xorshift_rng rng;
            for (int32_t block = 0; block < Config::num_blocks; ++block)
            {
                data_stream->BeforeDataAccess();
                DataBlock& data_block = data_stream->CurrDataBlock();
                int32_t num_slice = meta.local_vocab(block).num_slice();
                for (int32_t slice = 0; slice < num_slice; ++slice)
                {
                    for (int32_t i = 0; i < data_block.Size(); ++i)
                    {
                        Document* doc = data_block.GetOneDoc(i);
                        int32_t& cursor = doc->Cursor();
                        if (slice == 0) cursor = 0;
                        int32_t last_word = meta.local_vocab(block).LastWord(slice);
                        for (; cursor < doc->Size(); ++cursor)
                        {
                            if (doc->Word(cursor) > last_word) break;
                            // Init the latent variable
                            if (!Config::warm_start)
                                doc->SetTopic(cursor, rng.rand_k(Config::num_topics));
                            // Init the server table
                            Multiverso::AddToServer<int32_t>(kWordTopicTable,
                                doc->Word(cursor), doc->Topic(cursor), 1);
                            Multiverso::AddToServer<int64_t>(kSummaryRow,
                                0, doc->Topic(cursor), 1);
                        }
                    }
                    Multiverso::Flush();
                }
                data_stream->EndDataAccess();
            }
        }

        static void DumpDocTopic()
        {
            std::chrono::time_point<std::chrono::system_clock> start, end;
            start = std::chrono::system_clock::now();
            DumpDocTopicToMongoDB();
            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_time = end-start;
            std::cout << "time taken for dumping the doc-topic model: " << elapsed_time.count() << "s\n";
            

            //DumpDocTopicToFile();


        }

        static void DumpDocTopicToMongoDB()
        {
            mongocxx::pool ClientToModel(mongocxx::uri{mongo_uri_});
            auto conn = ClientToModel.acquire();
            auto doc_topicCollection = (*conn)["test"]["docTopicModel"];
            mongocxx::options::update updateOpts;
            updateOpts.upsert(true);
            bsoncxx::builder::stream::document index_builder;
            index_builder << "block_idx" << 1 << "docId" << 1;
            doc_topicCollection.create_index(index_builder.view(), {});

            
            Row<int32_t> doc_topic_counter(0, Format::Sparse, kMaxDocLength); 
            for (int32_t block = 0; block < Config::num_blocks; ++block)
            {

                //std::ofstream fout("doc_topic." + std::to_string(block));
                data_stream->BeforeDataAccess();
                DataBlock& data_block = data_stream->CurrDataBlock();
                for (int doc_i = 0; doc_i < data_block.Size(); ++doc_i)
                {
                    auto filter = bsoncxx::builder::stream::document{}
                            << "block_idx" << block
                            << "docId" << doc_i
                            << bsoncxx::builder::stream::finalize;
                    Document* doc = data_block.GetOneDoc(doc_i);
                    doc_topic_counter.Clear();
                    doc->GetDocTopicVector(doc_topic_counter);
                    //fout << doc_i << " ";  // doc id
                    Row<int32_t>::iterator iter = doc_topic_counter.Iterator();
                    bsoncxx::builder::stream::document ucpt_doc{};//ucpt = unormalized cpt

                    auto subdocstream = ucpt_doc << "$set" << bsoncxx::builder::stream::open_document
                                << "block_idx" << block
                                << "docId" <<  doc_i
                                << "ucpt" << bsoncxx::builder::stream::open_array;

                    while (iter.HasNext())
                    {
                        subdocstream
                                << bsoncxx::builder::stream::open_document
                                    << "topic_idx" << iter.Key()
                                    << "topic_count" << iter.Value()
                                << bsoncxx::builder::stream::close_document;
                        iter.Next();
                    }
                    subdocstream << bsoncxx::builder::stream::close_array 
                                 << bsoncxx::builder::stream::close_document;
                    bsoncxx::document::value fUpdate = ucpt_doc << bsoncxx::builder::stream::finalize;
                    doc_topicCollection.update_one(filter.view(), std::move(fUpdate), updateOpts);
                }
                data_stream->EndDataAccess();
            }
        }

        static void DumpDocTopicToFile()
        {
            std::string outputdir = Config::output_dir;
            Row<int32_t> doc_topic_counter(0, Format::Sparse, kMaxDocLength);
            for (int32_t block = 0; block < Config::num_blocks; ++block)
            {
                std::ofstream fout(outputdir + "/doc_topic." + std::to_string(block));
                data_stream->BeforeDataAccess();
                DataBlock& data_block = data_stream->CurrDataBlock();
                for (int i = 0; i < data_block.Size(); ++i)
                {
                    Document* doc = data_block.GetOneDoc(i);
                    doc_topic_counter.Clear();
                    doc->GetDocTopicVector(doc_topic_counter);
                    fout << i << " ";  // doc id
                    Row<int32_t>::iterator iter = doc_topic_counter.Iterator();
                    while (iter.HasNext())
                    {
                        fout << " " << iter.Key()
                             << ":" << iter.Value();
                        iter.Next();
                    }
                    fout << std::endl;
                }
                data_stream->EndDataAccess();
            }
        }

        static void CreateTable()
        {
            int32_t num_vocabs = Config::num_vocabs;
            int32_t num_topics = Config::num_topics;
            Type int_type = Type::Int;
            Type longlong_type = Type::LongLong;
            multiverso::Format dense_format = multiverso::Format::Dense;
            multiverso::Format sparse_format = multiverso::Format::Sparse;

            Multiverso::AddServerTable(kWordTopicTable, num_vocabs,
                num_topics, int_type, dense_format);
            Multiverso::AddCacheTable(kWordTopicTable, num_vocabs,
                num_topics, int_type, dense_format, Config::model_capacity);
            Multiverso::AddAggregatorTable(kWordTopicTable, num_vocabs,
                num_topics, int_type, dense_format, Config::delta_capacity);

            Multiverso::AddTable(kSummaryRow, 1, Config::num_topics,
                longlong_type, dense_format);
        }
        
        static void ConfigTable()
        {
            multiverso::Format dense_format = multiverso::Format::Dense;
            multiverso::Format sparse_format = multiverso::Format::Sparse;
            for (int32_t word = 0; word < Config::num_vocabs; ++word)
            {
                if (meta.tf(word) > 0)
                {
                    if (meta.tf(word) * kLoadFactor > Config::num_topics)
                    {
                        Multiverso::SetServerRow(kWordTopicTable,
                            word, dense_format, Config::num_topics);
                        Multiverso::SetCacheRow(kWordTopicTable,
                            word, dense_format, Config::num_topics);
                    }
                    else
                    {
                        Multiverso::SetServerRow(kWordTopicTable,
                            word, sparse_format, meta.tf(word) * kLoadFactor);
                        Multiverso::SetCacheRow(kWordTopicTable,
                            word, sparse_format, meta.tf(word) * kLoadFactor);
                    }
                }
                if (meta.local_tf(word) > 0)
                {
                    if (meta.local_tf(word) * 2 * kLoadFactor > Config::num_topics)
                        Multiverso::SetAggregatorRow(kWordTopicTable, 
                            word, dense_format, Config::num_topics);
                    else
                        Multiverso::SetAggregatorRow(kWordTopicTable, word, 
                            sparse_format, meta.local_tf(word) * 2 * kLoadFactor);
                }
            }
        }
    private:
        /*! \brief training data access */
        static IDataStream* data_stream;
        /*! \brief training data meta information */
        static Meta meta;
        static std::string mongo_uri_;
    };
    IDataStream* LightLDA::data_stream = nullptr;
    Meta LightLDA::meta;
    std::string LightLDA::mongo_uri_ = "to be initialized";

} // namespace lightlda
} // namespace multiverso


int main(int argc, char** argv)
{
    try {
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        {
            multiverso::lightlda::LightLDA::Run(argc, argv);
        }
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time = end-start;
        //std::time_t end_time = std::chrono::system_clock::to_time_t(end);
        std::cout << "elapsed time for LightLDA computation: " << elapsed_time.count() << "s\n";
    }
    catch(mongocxx::exception& e)
    {
        std::cout << "[mongocxx::exception] exception caught: " << e.what();
    }
    catch(bsoncxx::exception& e)
    {
        std::cout << "[bsoncxx::exception] exception caught: " << e.what();
    }
    catch(std::exception& e) {
        std::cout << "[std::exception] exception caught: " << e.what();
    }

    return 0;
}
