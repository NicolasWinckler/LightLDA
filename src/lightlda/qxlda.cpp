
#include "qxlda.hpp"

namespace multiverso
{
    namespace lightlda
    {

        qxlda::qxlda() : m_data_stream(nullptr), m_meta(), m_chrono(true)
        {
        }

        qxlda::~qxlda()
        {
        }

        void qxlda::Run(int argc, char** argv)
        {
            Config::Init(argc, argv);
            AliasTable* alias_table = new AliasTable();
            Barrier* barrier = new Barrier(Config::num_local_workers);
            m_meta.SetMongoParameters(Config::mongo_uri,Config::mongo_DB,Config::mongo_vocabCollection);
            m_meta.Init();
            std::vector<TrainerBase*> trainers;
            for (int32_t i = 0; i < Config::num_local_workers; ++i)
            {
                Trainer* trainer = new Trainer(alias_table, barrier, &m_meta);
                trainers.push_back(trainer);
            }

            ParamLoader* param_loader = new ParamLoader();
            multiverso::Config config;
            config.num_servers = Config::num_servers;
            config.num_aggregator = Config::num_aggregator;
            config.server_endpoint_file = Config::server_file;
            config.output_dir = Config::output_dir;

            std::string logFileNamePrefix;
            if(!Config::output_dir.empty())
                logFileNamePrefix = Config::output_dir + "/LightLDA.";
            else
                logFileNamePrefix = "LightLDA.";


            Multiverso::Init(trainers, param_loader, config, &argc, &argv);

            Log::ResetLogFile(logFileNamePrefix
                              + std::to_string(clock()) + ".log");

            m_data_stream = CreateDataStream();
            InitMultiverso();
            Train();

            Multiverso::Close();

            for (auto& trainer : trainers)
            {
                delete trainer;
            }
            delete param_loader;

            DumpDocTopic();

            delete m_data_stream;
            delete barrier;
            delete alias_table;
            m_data_stream = nullptr;
        }


        void qxlda::Train()
        {
            Multiverso::BeginTrain();
            for (int32_t i = 0; i < Config::num_iterations; ++i)
            {
                Multiverso::BeginClock();
                // Train corpus block by block
                for (int32_t block = 0; block < Config::num_blocks; ++block)
                {
                    m_data_stream->BeforeDataAccess();
                    DataBlock& data_block = m_data_stream->CurrDataBlock();
                    data_block.set_meta(&m_meta.local_vocab(block));
                    int32_t num_slice = m_meta.local_vocab(block).num_slice();
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
                    m_data_stream->EndDataAccess();
                }
                Multiverso::EndClock();
            }
            Multiverso::EndTrain();
        }

        void qxlda::InitMultiverso()
        {
            Multiverso::BeginConfig();
            CreateTable();
            ConfigTable();
            Initialize();
            Multiverso::EndConfig();
        }

        void qxlda::Initialize()
        {
            xorshift_rng rng;
            for (int32_t block = 0; block < Config::num_blocks; ++block)
            {
                m_data_stream->BeforeDataAccess();
                DataBlock& data_block = m_data_stream->CurrDataBlock();
                int32_t num_slice = m_meta.local_vocab(block).num_slice();
                for (int32_t slice = 0; slice < num_slice; ++slice)
                {
                    for (int32_t i = 0; i < data_block.Size(); ++i)
                    {
                        Document* doc = data_block.GetOneDoc(i);
                        int32_t& cursor = doc->Cursor();
                        if (slice == 0) cursor = 0;
                        int32_t last_word = m_meta.local_vocab(block).LastWord(slice);
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
                m_data_stream->EndDataAccess();
            }
        }

        void qxlda::DumpDocTopic()
        {
            std::chrono::time_point<std::chrono::system_clock> start, end;
            start = std::chrono::system_clock::now();
            DumpDocTopicToMongoDB();
            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_time = end-start;
            std::cout << "time taken for dumping the doc-topic model: " << elapsed_time.count() << "s\n";


            //DumpDocTopicToFile();


        }

        void qxlda::DumpDocTopicToMongoDB()
        {
            mongocxx::pool ClientToModel(mongocxx::uri{Config::mongo_uri});
            auto conn = ClientToModel.acquire();
            auto doc_topicCollection = (*conn)[Config::mongo_DB][Config::mongo_docTopicCollection];
            mongocxx::options::update updateOpts;
            updateOpts.upsert(true);
            bsoncxx::builder::stream::document index_builder;
            index_builder << "block_idx" << 1 << "docId" << 1;
            doc_topicCollection.create_index(index_builder.view(), {});


            Row<int32_t> doc_topic_counter(0, Format::Sparse, kMaxDocLength);
            for (int32_t block = 0; block < Config::num_blocks; ++block)
            {

                //std::ofstream fout("doc_topic." + std::to_string(block));
                m_data_stream->BeforeDataAccess();
                DataBlock& data_block = m_data_stream->CurrDataBlock();
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
                m_data_stream->EndDataAccess();
            }
        }

        void qxlda::DumpDocTopicToFile()
        {
            std::string outputdir = Config::output_dir;
            Row<int32_t> doc_topic_counter(0, Format::Sparse, kMaxDocLength);
            for (int32_t block = 0; block < Config::num_blocks; ++block)
            {
                std::ofstream fout(outputdir + "/doc_topic." + std::to_string(block));
                m_data_stream->BeforeDataAccess();
                DataBlock& data_block = m_data_stream->CurrDataBlock();
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
                m_data_stream->EndDataAccess();
            }
        }

        void qxlda::CreateTable()
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

        void qxlda::ConfigTable()
        {
            multiverso::Format dense_format = multiverso::Format::Dense;
            multiverso::Format sparse_format = multiverso::Format::Sparse;
            for (int32_t word = 0; word < Config::num_vocabs; ++word)
            {
                if (m_meta.tf(word) > 0)
                {
                    if (m_meta.tf(word) * kLoadFactor > Config::num_topics)
                    {
                        Multiverso::SetServerRow(kWordTopicTable,
                                                 word, dense_format, Config::num_topics);
                        Multiverso::SetCacheRow(kWordTopicTable,
                                                word, dense_format, Config::num_topics);
                    }
                    else
                    {
                        Multiverso::SetServerRow(kWordTopicTable,
                                                 word, sparse_format, m_meta.tf(word) * kLoadFactor);
                        Multiverso::SetCacheRow(kWordTopicTable,
                                                word, sparse_format, m_meta.tf(word) * kLoadFactor);
                    }
                }
                if (m_meta.local_tf(word) > 0)
                {
                    if (m_meta.local_tf(word) * 2 * kLoadFactor > Config::num_topics)
                        Multiverso::SetAggregatorRow(kWordTopicTable,
                                                     word, dense_format, Config::num_topics);
                    else
                        Multiverso::SetAggregatorRow(kWordTopicTable, word,
                                                     sparse_format, m_meta.local_tf(word) * 2 * kLoadFactor);
                }
            }
        }


    } // namespace lightlda
} // namespace multiverso

