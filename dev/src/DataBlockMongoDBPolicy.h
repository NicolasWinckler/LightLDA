//
// Created by nw on 31.03.17.
//

#ifndef LIGHTLDA_DATABLOCKDBPOLICY_H
#define LIGHTLDA_DATABLOCKDBPOLICY_H

#include "document.h"
#include "common.h"
#include "DataBlockInterface.h"
#include <multiverso/log.h>


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
        struct Token 
        {
            int32_t word_id;
            int32_t topic_id;
        };

        class DataBlockMongoDBPolicy
        {

            typedef std::unique_ptr<mongocxx::pool> MongoPool_ptr;
            //friend class DataBlockInterface<DataBlockMongoDBPolicy>;

        public:
            DataBlockMongoDBPolicy() :
                    //doc_buf_(nullptr),
                    DataBlockInterface_(nullptr),
                    MongoDBName_(),
                    MongoCollectionName_(),
                    MongoUri_(),
                    has_read_(false),
                    kMaxDocLength(8192),
                    ClientToTrainingData_(nullptr)
            {
            }
            virtual ~DataBlockMongoDBPolicy(){}


            void SetFileName(const std::string& filename){/*dummy-temporary function to match current API */}

            void Init(DataBlockInterface<DataBlockMongoDBPolicy>* interface)
            {
                DataBlockInterface_ = interface;
                // some checks...
                if(MongoUri_.empty())
                    Log::Fatal("Mongo uri is not defined, program will now exit \n");

                if(MongoDBName_.empty())
                    Log::Fatal("Mongo DataBase Name is not defined, program will now exit \n");

                if(MongoCollectionName_.empty())
                    Log::Fatal("Mongo Collection Name is not defined, program will now exit \n");

                if(!ClientToTrainingData_)
                    Log::Fatal("Mongo pool is not not valid, program will now exit \n");
            }


            bool HasLoad() const
            {
                return has_read_;
            }

            /* read from source=MongoDB and store to buffer*/
            void Read()
            {
                ReadTrainingData();
            }

            /* write from buffer to destination=MongoDB*/
            void Write()
            {
                //WriteTrainingData();
                for(int64_t docIdx(0); docIdx<DataBlockInterface_->num_document_; docIdx++)
                {
                    int32_t* docBuffer_ptr = nullptr;
                    int64_t offsetStart = DataBlockInterface_->offset_buffer_[docIdx];
                    int64_t offsetEnd = DataBlockInterface_->offset_buffer_[docIdx + 1];

                    docBuffer_ptr = DataBlockInterface_->documents_buffer_ + offsetStart;
                    for(int64_t tokenIdx(offsetStart); tokenIdx < offsetEnd; tokenIdx++)
                    {
                        int32_t wordId = *(docBuffer_ptr + 1 + tokenIdx * 2);
                        int32_t topicId = *(docBuffer_ptr + 2 + tokenIdx * 2);

                        WriteTrainingData(docIdx, wordId, topicId);
                    }
                    //WriteTrainingData(int64_t docId, int32_t wordId, int32_t topicId)
                    
                }
                has_read_ = false;
            }


            void SetMongoParameters(const std::string& uri, const std::string& DBName, const std::string& collectionName)
            {
                MongoUri_ = uri;
                MongoDBName_ = DBName;
                MongoCollectionName_ = collectionName;

                ClientToTrainingData_ = std::move(MongoPool_ptr(new mongocxx::pool(mongocxx::uri{uri})));
            }

            void ReadTrainingData()
            {

                auto conn = ClientToTrainingData_->acquire();
                auto trainingDataCollection = (*conn)[MongoDBName_][MongoCollectionName_];

                bsoncxx::builder::stream::document filter_builder;
                mongocxx::options::find opts{};
                opts.no_cursor_timeout(true);

                auto trainingCursor = trainingDataCollection.find(filter_builder.view(), opts);

                static_assert( std::is_same< DocNumber, decltype(trainingDataCollection.count({})) >::value, "types must be the same");

                DataBlockInterface_->num_document_ = trainingDataCollection.count({});


                if (DataBlockInterface_->num_document_ > DataBlockInterface_->max_num_document_)
                {
                    Log::Fatal("Rank %d: Num of documents > max number of documents when reading MongoDB %s, collection %s \n",
                               Multiverso::ProcessRank(), MongoDBName_.c_str(), MongoCollectionName_.c_str());
                }

                int doc_buf_idx=0;
                int j=0;
                // TODO: change BSON format to be consistent with block.0, block.1 etc..
                // loop over <doc,[words]> pairs
                for (auto &&doc : trainingCursor)
                {
                    bsoncxx::document::element ele_id;
                    
                    if ((ele_id = doc["tokenIds"]))
                    {
                        bsoncxx::array::view observedWordArray{ele_id.get_array().value};
                        // loop over visited items (array)

                        int doc_token_count = 0;
                        std::vector<Token> doc_tokens;

                        //bsoncxx::document::view tokenView;
                        for(const auto& observedWord : observedWordArray)
                        {
                            if (doc_token_count >= kMaxDocLength) break;
                            bsoncxx::document::view tokenView = observedWord.get_document().view();

                            bsoncxx::document::element word_ele{tokenView["wordId"]};
                            bsoncxx::document::element topic_ele{tokenView["topicId"]};

                            int32_t wordId = -1;
                            int32_t topicId = -1;

                            if (word_ele && word_ele.type() == bsoncxx::type::k_int32)
                                wordId = observedWord.get_int32().value;

                            if (topic_ele && topic_ele.type() == bsoncxx::type::k_int32)
                                topicId = observedWord.get_int32().value;

                            if(wordId > 0 && topicId > 0)
                                doc_tokens.push_back({ wordId, topicId });
                            else
                                Log::Fatal("[ERROR] word_id or/and topic_id not found in MongoDB, and is/are required for the LightLDA document buffer.");


                        }// end loop over words

                        std::sort(doc_tokens.begin(), doc_tokens.end(), [](const Token& token1, const Token& token2) 
                        {
                            return token1.word_id < token2.word_id;
                        });
                        
                        DataBlockInterface_->documents_buffer_[doc_buf_idx++] = 0;
                        for (auto& token : doc_tokens)
                        {
                            DataBlockInterface_->documents_buffer_[doc_buf_idx++] = token.word_id;
                            DataBlockInterface_->documents_buffer_[doc_buf_idx++] = token.topic_id;
                        }
                        
                        //DataBlockInterface_->offset_buffer_[j + 1] = DataBlockInterface_->offset_buffer_[j] + doc_buf_idx;
                        DataBlockInterface_->offset_buffer_[j + 1] = doc_buf_idx;
                    }
                    j++;
                }// end loop over doc

                //TODO: the check below is useless yet, fix this
                DataBlockInterface_->corpus_size_ = doc_buf_idx;
                if (DataBlockInterface_->corpus_size_ > DataBlockInterface_->memory_block_size_)
                {
                    Log::Fatal("Rank %d: corpus_size_ > memory_block_size when reading file \n",
                               Multiverso::ProcessRank());
                }
                DataBlockInterface_->GenerateDocuments();
                has_read_ = true;

            }



            //////////////////////////
            void ReadTrainingDataFILE()
            {

//                std::ifstream block_file(file_name_, std::ios::in | std::ios::binary);
//                if (!block_file.good())
//                {
//                    Log::Fatal("Failed to read data %s\n", file_name_.c_str());
//                }
//                block_file.read(reinterpret_cast<char*>(&DataBlockInterface_->num_document_), sizeof(DocNumber));

//                if (DataBlockInterface_->num_document_ > DataBlockInterface_->max_num_document_)
//                {
//                    Log::Fatal("Rank %d: Num of documents > max number of documents when reading file %s\n",
//                               Multiverso::ProcessRank(), file_name_.c_str());
//                }

                //block_file.read(reinterpret_cast<char*>(DataBlockInterface_->offset_buffer_),
                //                sizeof(int64_t)* (DataBlockInterface_->num_document_ + 1));

                //DataBlockInterface_->corpus_size_ = DataBlockInterface_->offset_buffer_[DataBlockInterface_->num_document_];

                //if (DataBlockInterface_->corpus_size_ > DataBlockInterface_->memory_block_size_)
                //{
                //    Log::Fatal("Rank %d: corpus_size_ > memory_block_size when reading file %s\n",
                //               Multiverso::ProcessRank(), file_name_.c_str());
                //}

                //block_file.read(reinterpret_cast<char*>(DataBlockInterface_->documents_buffer_),
                //                sizeof(int32_t)* DataBlockInterface_->corpus_size_);
                //block_file.close();

                //DataBlockInterface_->GenerateDocuments();
                //has_read_ = true;
            }


            void WriteTrainingData(int64_t docId, int32_t wordId, int32_t topicId)
            {
                auto conn = ClientToTrainingData_->acquire();
                //auto recProcCollection = (*conn)[s_RecProcDB][s_RecProcTrainCollection];
                auto trainingDataCollection = (*conn)[MongoDBName_][MongoCollectionName_];

                // upsert options
                mongocxx::options::update updateOpts;
                updateOpts.upsert(true);

                // filter
                auto filter = bsoncxx::builder::stream::document{}
                        << "docId" << docId
                        << bsoncxx::builder::stream::finalize;

                //open doc
                bsoncxx::builder::stream::document doc{};

                doc << "$push"  << bsoncxx::builder::stream::open_document
                    << "tokenIds" << bsoncxx::builder::stream::open_document
                    << "$each"  << bsoncxx::builder::stream::open_array
                        << bsoncxx::builder::stream::open_document
                        << "wordId" << wordId
                        << "topicId" << topicId
                        << bsoncxx::builder::stream::close_document
                    << bsoncxx::builder::stream::close_array << "$slice" << -kMaxDocLength
                    << bsoncxx::builder::stream::close_document
                    << bsoncxx::builder::stream::close_document;

                // update
                bsoncxx::document::value fUpdate = doc << bsoncxx::builder::stream::finalize;
                trainingDataCollection.update_one(filter.view(), std::move(fUpdate), updateOpts);
                //LOG(DEBUG) << "upserted user " << userId;
                has_read_ = false;

            }

            void WriteTrainingDataFile()
            {
//                std::string temp_file = file_name_ + ".temp";
//
//                std::ofstream block_file(temp_file, std::ios::out | std::ios::binary);
//
//                if (!block_file.good())
//                {
//                    Log::Fatal("Failed to open file %s\n", temp_file.c_str());
//                }
//
//                block_file.write(reinterpret_cast<char*>(&DataBlockInterface_->num_document_),
//                                 sizeof(DocNumber));
//                block_file.write(reinterpret_cast<char*>(DataBlockInterface_->offset_buffer_),
//                                 sizeof(int64_t)* (DataBlockInterface_->num_document_ + 1));
//                block_file.write(reinterpret_cast<char*>(DataBlockInterface_->documents_buffer_),
//                                 sizeof(int32_t)* DataBlockInterface_->corpus_size_);
//                block_file.flush();
//                block_file.close();
//
//                AtomicMoveFileExA(temp_file, file_name_);
//                has_read_ = false;
            }
            //////////////////////////

        private:
            //int32_t *doc_buf_;
            DataBlockInterface<DataBlockMongoDBPolicy>* DataBlockInterface_;
            std::string MongoDBName_;
            std::string MongoCollectionName_;
            std::string MongoUri_;
            bool has_read_;
            const int32_t kMaxDocLength;
            std::unique_ptr<mongocxx::pool> ClientToTrainingData_;
        };
    } // namespace lightlda
} // namespace multiverso


#endif //LIGHTLDA_DATABLOCKDBPOLICY_H
