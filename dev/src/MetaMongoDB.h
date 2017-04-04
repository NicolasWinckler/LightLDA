//
// Created by nw on 04.04.17.
//

#ifndef LIGHTLDA_METAMONGODB_H
#define LIGHTLDA_METAMONGODB_H


#include "MetaBase.h"
#include <memory>

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

                enum class Vocabulary
                {
                    kId = 0,
                    kGlobal_tf = 1,
                    kLocal_tf = 2
                };
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

                    //TODO: currently this is impl for a single block --> do it for multiple block
                    // -------------------------------------------------
                    // init mongo

                    // some checks...
                    if(MongoUri_.empty())
                        Log::Fatal("Mongo uri is not defined, program will now exit \n");

                    if(MongoDBName_.empty())
                        Log::Fatal("Mongo DataBase Name is not defined, program will now exit \n");

                    if(MongoCollectionName_.empty())
                        Log::Fatal("Mongo Collection Name is not defined, program will now exit \n");

                    if(!ClientToVocabData_)
                        Log::Fatal("Mongo pool is not not valid, program will now exit \n");



                    auto conn = ClientToVocabData_->acquire();
                    auto vocabDataCollection = (*conn)[MongoDBName_][MongoCollectionName_];
                    bsoncxx::builder::stream::document filter_builder;
                    mongocxx::options::find opts{};
                    opts.no_cursor_timeout(true);
                    auto vocabCursor = vocabDataCollection.find(filter_builder.view(), opts);



                    // -------------------------------------------------


                    int32_t block_i = 0;
                    for (auto &&vocabBlock : vocabCursor)
                    //for (int32_t i = 0; i < Config::num_blocks; ++i)
                    {
                        auto& local_vocab = local_vocabs_[block_i];
                        local_vocab.size_ = vocabDataCollection.count({});

                        if(local_vocab.size_ > Config::num_vocabs)
                            Log::Fatal("Not enough allocated memory for the term-frequency buffer. Increase Config::num_vocabs. Program will now exit \n");

                        local_vocab.vocabs_ = new int[local_vocab.size_];
                        local_vocab.own_memory_ = true;


                        std::string blockIdx("vocab.");
                        blockIdx+= std::to_string(block_i);

                        int32_t vocDBIdx = 0;
                        //for (auto &&vocabBlock : vocabCursor)
                        {
                            bsoncxx::document::element voc_ele;
                            //if element "vocab.block_i" found
                            if((voc_ele = vocabBlock[blockIdx.c_str()] ))
                            {
                                bsoncxx::document::element vocabItem_ele;
                                std::string vocab_key("vocabIdx.");
                                vocab_key += std::to_string(vocDBIdx);

                                // if element "vocabIdx.vocDBIdx"
                                if(vocabItem_ele = voc_ele[vocab_key.c_str()])
                                {
                                    bsoncxx::array::view observedVoc{vocabItem_ele.get_array().value};
                                    if(observedVoc.length()!=3)
                                        Log::Fatal("size of Vocab array is different than 3, program will now exit \n");
                                    int32_t voc_id = observedVoc[Vocabulary::kId];
                                    int32_t glob_tf = observedVoc[Vocabulary::kGlobal_tf];
                                    int32_t loc_tf = observedVoc[Vocabulary::kLocal_tf];

                                    local_vocab.vocabs_[vocDBIdx] = voc_id;
                                    tf[vocDBIdx] = glob_tf;
                                    local_tf[vocDBIdx] = loc_tf;


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

                                }

                            }

                            vocDBIdx++;
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
