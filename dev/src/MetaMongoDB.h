//
// Created by nw on 04.04.17.
//

#ifndef LIGHTLDA_METAMONGODB_H
#define LIGHTLDA_METAMONGODB_H


#include "MetaBase.h"
#include <memory>
#include <iostream>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/pool.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>


#include <type_traits>

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            class MetaMongoDB : public MetaBase<MetaMongoDB>
            {
                //typedef MetaBase<MetaMongoDB> base_type;
                // functions members
                typedef std::unique_ptr<mongocxx::pool> MongoPool_ptr;
            public:

                using base_type::tf;
                using base_type::local_tf;
                using base_type::local_vocab;
                using base_type::alias_index;
                using base_type::ModelSchedule;
                using base_type::ModelSchedule4Inference;
                using base_type::BuildAliasIndex;

                // data members
                using base_type::local_vocabs_;
                using base_type::tf_;
                using base_type::local_tf_;
                using base_type::alias_index_;


                MetaMongoDB() : base_type(),
                                fileNameWOExt_("vocab"),
                                MongoDBName_(),
                                MongoCollectionName_(),
                                MongoUri_(),
                                ClientToVocabData_(nullptr){}
                virtual ~MetaMongoDB() {}

                void SetFileNameWOExt(const std::string& filename)
                {
                    fileNameWOExt_=filename;
                }

                void SetMongoParameters(const std::string& uri, const std::string& DBName, const std::string& collectionName)
                {
                    MongoUri_ = uri;
                    MongoDBName_ = DBName;
                    MongoCollectionName_ = collectionName;

                    ClientToVocabData_ = std::move(MongoPool_ptr(new mongocxx::pool(mongocxx::uri{uri})));
                }

                void Init()
                {
                    tf_.resize(Config::num_vocabs, 0);
                    local_tf_.resize(Config::num_vocabs, 0);
                    int32_t* tf_buffer = new int32_t[Config::num_vocabs];
                    int32_t* local_tf_buffer = new int32_t[Config::num_vocabs];
                    local_vocabs_.resize(Config::num_blocks);

                    // -------------------------------------------------
                    // init mongo

                    // some checks...
                    if(MongoUri_.empty())
                        Log::Fatal("[Meta] Mongo uri is not defined, program will now exit \n");

                    if(MongoDBName_.empty())
                        Log::Fatal("[Meta] Mongo DataBase Name is not defined, program will now exit \n");

                    if(MongoCollectionName_.empty())
                        Log::Fatal("[Meta] Mongo Collection Name is not defined, program will now exit \n");

                    if(!ClientToVocabData_)
                        Log::Fatal("[Meta] Mongo pool is not not valid, program will now exit \n");



                    auto conn = ClientToVocabData_->acquire();
                    auto vocabDataCollection = (*conn)[MongoDBName_][MongoCollectionName_];
                    bsoncxx::builder::stream::document filter_builder;
                    mongocxx::options::find opts{};
                    opts.no_cursor_timeout(true);
                    //auto vocabCursor = vocabDataCollection.find(filter_builder.view(), opts);



                    // -------------------------------------------------



                    //for (auto &&vocabBlock : vocabCursor)
                    for (int32_t block_i = 0; block_i < Config::num_blocks; ++block_i)
                    {
                        auto vocabCursorInit = vocabDataCollection.find(
                                bsoncxx::builder::stream::document{} 
                                    << "block_idx" << block_i 
                                    << bsoncxx::builder::stream::finalize, 
                                opts);

                        auto& local_vocab = local_vocabs_[block_i];


                        int32_t vocsize = 0;
                        for (auto &&vocabBlock : vocabCursorInit)
                        {
                            bsoncxx::document::element vocab_ele;
                            if((vocab_ele = vocabBlock["vocab"] ))
                                vocsize = vocabBlock["vocab"]["vocab_size"].get_int32().value;
                        }

                        if(vocsize == 0)
                            Log::Fatal("In MetaMongoDB : problem while loading the local vocabulary size ... \n");

                        local_vocab.size_ = vocsize;

                        std::cout << "local_vocab.size_ = " << local_vocab.size_ << std::endl;
                        if(local_vocab.size_ > Config::num_vocabs)
                            Log::Fatal("Not enough allocated memory for the term-frequency buffer. Increase Config::num_vocabs. Program will now exit \n");

                        local_vocab.vocabs_ = new int[local_vocab.size_];
                        local_vocab.own_memory_ = true;


                        auto vocabCursor = vocabDataCollection.find(
                                bsoncxx::builder::stream::document{}
                                        << "block_idx" << block_i
                                        << bsoncxx::builder::stream::finalize,
                                opts);



                        int32_t vocDBIdx = 0;
                        for (auto &&vocabBlock : vocabCursor)
                        {
                            bsoncxx::document::element vocab_ele;
                            if((vocab_ele = vocabBlock["vocab"] ))
                            {
                                bsoncxx::document::element vocId_ele;
                                if((vocId_ele = vocabBlock["vocab"]["id"] ))
                                {
                                    vocDBIdx = 0;
                                    bsoncxx::array::view vocabId_array{vocId_ele.get_array().value};
                                    for(const auto& vocabId : vocabId_array)
                                    {
                                        //bsoncxx::document::element
                                        if (vocabId && vocabId.type() == bsoncxx::type::k_int32)
                                        {
                                            int32_t voc_id = vocabId.get_int32().value;

                                            //std::cout << "Meta::ok1 voc_id=" << voc_id << "\n";
                                            local_vocab.vocabs_[vocDBIdx] = voc_id;

                                            //std::cout << "Meta::ok2 voc_id=" << voc_id << "\n";
                                            vocDBIdx++;
                                        }

                                    }
                                }

                                std::cout << "Meta::vocab.global_tf\n";
                                bsoncxx::document::element globtf_ele;
                                if((globtf_ele = vocabBlock["vocab"]["global_tf"] ))
                                {
                                    std::cout << "Meta::vocab.global_tf = OK \n";
                                    vocDBIdx = 0;
                                    bsoncxx::array::view globtf_array{globtf_ele.get_array().value};
                                    for(const auto& globtf : globtf_array)
                                    {
                                        if (globtf && globtf.type() == bsoncxx::type::k_int32)
                                        {
                                            int32_t glob_tf = globtf.get_int32().value;
                                            tf_buffer[vocDBIdx] = glob_tf;
                                            vocDBIdx++;
                                        }
                                    }
                                }

                                std::cout << "Meta::vocab.loacal_tf\n";
                                bsoncxx::document::element loctf_ele;
                                if((loctf_ele = vocabBlock["vocab"]["loacal_tf"] ))
                                {
                                    vocDBIdx = 0;
                                    bsoncxx::array::view loctf_array{loctf_ele.get_array().value};
                                    for(const auto& loctf : loctf_array)
                                    {
                                        if (loctf && loctf.type() == bsoncxx::type::k_int32)
                                        {
                                            int32_t loc_tf = loctf.get_int32().value;
                                            local_tf_buffer[vocDBIdx] = loc_tf;
                                            vocDBIdx++;
                                        }

                                    }
                                }
                            }
                        }

                        std::cout << "Meta::outside cursor loop\n";

                        for (int32_t v = 0; v < local_vocab.size_; ++v)
                        {
                            if (tf_buffer[v] > tf_[local_vocab.vocabs_[v]])
                            {
                                tf_[local_vocab.vocabs_[v]] = tf_buffer[v];
                            }
                            if (local_tf_buffer[v] > local_tf_[local_vocab.vocabs_[v]])
                            {
                                local_tf_[local_vocab.vocabs_[v]] = local_tf_buffer[v];
                            }
                        }

                        block_i++;
                    }
                    delete[] local_tf_buffer;
                    delete[] tf_buffer;

                    if(!Config::inference)
                    {
                        ModelSchedule();
                    }
                    else
                    {
                        ModelSchedule4Inference();
                    }
                    BuildAliasIndex();
                }


                /*
                    void InitFile()
                    {
                        tf_.resize(Config::num_vocabs, 0);
                        local_tf_.resize(Config::num_vocabs, 0);
                        int32_t* tf_buffer = new int32_t[Config::num_vocabs];
                        int32_t* local_tf_buffer = new int32_t[Config::num_vocabs];
                        local_vocabs_.resize(Config::num_blocks);
                        for (int32_t i = 0; i < Config::num_blocks; ++i)
                        {
                            auto& local_vocab = local_vocabs_[i];

                            std::string file_name = Config::input_dir +
                                                    "/" + fileNameWOExt_ +
                                                    "." + std::to_string(i);
                            std::ifstream vocab_file(file_name, std::ios::in|std::ios::binary);

                            if (!vocab_file.good())
                            {
                                Log::Fatal("Failed to open file : %s\n", file_name.c_str());
                            }

                            vocab_file.read(reinterpret_cast<char*>(&local_vocab.size_),
                                            sizeof(int));
                            local_vocab.vocabs_ = new int[local_vocab.size_];
                            local_vocab.own_memory_ = true;
                            vocab_file.read(reinterpret_cast<char*>(local_vocab.vocabs_),
                                            sizeof(int)*  local_vocab.size_);
                            vocab_file.read(reinterpret_cast<char*>(tf_buffer),
                                            sizeof(int)*  local_vocab.size_);
                            vocab_file.read(reinterpret_cast<char*>(local_tf_buffer),
                                            sizeof(int)*  local_vocab.size_);

                            vocab_file.close();

                            for (int32_t i = 0; i < local_vocab.size_; ++i)
                            {
                                if (tf_buffer[i] > tf_[local_vocab.vocabs_[i]])
                                {
                                    tf_[local_vocab.vocabs_[i]] = tf_buffer[i];
                                }
                                if (local_tf_buffer[i] > local_tf_[local_vocab.vocabs_[i]])
                                {
                                    local_tf_[local_vocab.vocabs_[i]] = local_tf_buffer[i];
                                }
                            }
                        }

                    delete[] local_tf_buffer;
                    delete[] tf_buffer;

                    if(!Config::inference)
                    {
                        ModelSchedule();
                    }
                    else
                    {
                        ModelSchedule4Inference();
                    }
                    BuildAliasIndex();
                }
                //*/

            private:
                std::string fileNameWOExt_;//file name without extension
                std::string MongoDBName_;
                std::string MongoCollectionName_;
                std::string MongoUri_;
                std::unique_ptr<mongocxx::pool> ClientToVocabData_;
            };
        }// namespace dev
    } // namespace lightlda
} // namespace multiverso
#endif //LIGHTLDA_METAMONGODB_H
