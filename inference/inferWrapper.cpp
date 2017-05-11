//
// Created by nw on 11.05.17.
//

#include "inferWrapper.hpp"

namespace multiverso
{
    namespace lightlda
    {
        inferWrapper::inferWrapper() : m_data_stream(nullptr), m_meta()
        {
        }

        inferWrapper::~inferWrapper()
        {
        }

        void inferWrapper::Run(int argc, char** argv)
        {
            std::string logFileNamePrefix;
            if(!Config::output_dir.empty())
                logFileNamePrefix = Config::output_dir + "/LightLDA_infer.";
            else
                logFileNamePrefix = "LightLDA_infer.";
            Log::ResetLogFile(logFileNamePrefix + std::to_string(clock()) + ".log");
            Config::Init(argc, argv);
            //init meta
            m_meta.Init();
            //init model
            LocalModel* model = new LocalModel(&m_meta); model->Init();
            //init document stream
            m_data_stream = CreateDataStream();
            //init documents
            InitDocument();
            //init alias table
            AliasTable* alias_table = new AliasTable();
            //init inferers
            std::vector<Inferer*> inferers;
            Barrier barrier(Config::num_local_workers);
            // pthread_barrier_t barrier;
            // pthread_barrier_init(&barrier, nullptr, Config::num_local_workers);
            for (int32_t i = 0; i < Config::num_local_workers; ++i)
            {
                inferers.push_back(new Inferer(alias_table, m_data_stream,
                                               &m_meta, model,
                                               &barrier, i, Config::num_local_workers));
            }

            //do inference in muti-threads
            Inference(inferers);

            //dump doc topic
            DumpDocTopic();

            //recycle space
            for (auto& inferer : inferers)
            {
                delete inferer;
                inferer = nullptr;
            }
            // pthread_barrier_destroy(&barrier);
            delete m_data_stream;
            delete alias_table;
            delete model;
            m_data_stream = nullptr;
        }

        void inferWrapper::Inference(std::vector<Inferer*>& inferers)
        {
            //pthread_t * threads = new pthread_t[Config::num_local_workers];
            //if(nullptr == threads)
            //{
            //    Log::Fatal("failed to allocate space for worker threads");
            //}
            std::vector<std::thread> threads;
            for(int32_t i = 0; i < Config::num_local_workers; ++i)
            {
                threads.push_back(std::thread(&InferenceThread, inferers[i]));
                //if(pthread_create(threads + i, nullptr, InferenceThread, inferers[i]))
                //{
                //    Log::Fatal("failed to create worker threads");
                //}
            }
            for(int32_t i = 0; i < Config::num_local_workers; ++i)
            {
                // pthread_join(threads[i], nullptr);
                threads[i].join();
            }
            // delete [] threads;
        }

        void* inferWrapper::InferenceThread(void* arg)
        {
            Inferer* inferer = (Inferer*)arg;
            // inference corpus block by block
            for (int32_t block = 0; block < Config::num_blocks; ++block)
            {
                inferer->BeforeIteration(block);
                for (int32_t i = 0; i < Config::num_iterations; ++i)
                {
                    inferer->DoIteration(i);
                }
                inferer->EndIteration();
            }
            return nullptr;
        }

        void inferWrapper::InitDocument()
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
                        }
                    }
                }
                m_data_stream->EndDataAccess();
            }
        }


        void inferWrapper::DumpDocTopic()
        {
            std::chrono::time_point<std::chrono::system_clock> start, end;
            start = std::chrono::system_clock::now();
            DumpDocTopicToMongoDB();
            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_time = end-start;
            std::cout << "time taken for dumping the doc-topic model: " << elapsed_time.count() << "s\n";

        }

        void inferWrapper::DumpDocTopicToMongoDB()
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
                            << "mode" << "inference"
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
                                                 << "mode" << "inference"
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

        void inferWrapper::DumpDocTopicToFile()
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

    } // namespace lightlda
} // namespace multiverso

