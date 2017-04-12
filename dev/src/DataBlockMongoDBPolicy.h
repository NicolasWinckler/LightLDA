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

#include "MongoHelper.h"
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
                    doc_buf_idx_(0),
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
                
            }


            bool HasLoad() const
            {
                return has_read_;
            }

            /* read from source=MongoDB and store to buffer*/
            void Read(int32_t block_idx)
            {
                CheckDBParameters();
                auto conn = ClientToTrainingData_->acquire();
                auto trainingDataCollection = (*conn)[MongoDBName_][MongoCollectionName_];


                // filter
                auto filter = bsoncxx::builder::stream::document{}
                        << "block_idx" << block_idx
                        << bsoncxx::builder::stream::finalize;

                DataBlockInterface_->num_document_ = trainingDataCollection.count(filter.view());


                static_assert( std::is_same< DocNumber, decltype(trainingDataCollection.count(filter.view())) >::value, "types must be the same");

                if (DataBlockInterface_->num_document_ > DataBlockInterface_->max_num_document_)
                {
                    Log::Fatal("Rank %d: Num of documents > max number of documents when reading MongoDB %s, collection %s \n",
                               Multiverso::ProcessRank(), MongoDBName_.c_str(), MongoCollectionName_.c_str());
                }

                doc_buf_idx_=0;
                DataBlockInterface_->offset_buffer_[0] = 0;
                for(int64_t docId(0); docId<DataBlockInterface_->num_document_; docId++ )
                {
                    ReadTrainingData(block_idx,docId);
                }


                //TODO: the check below is useless yet, fix this
                DataBlockInterface_->corpus_size_ = doc_buf_idx_;
                if (DataBlockInterface_->corpus_size_ > DataBlockInterface_->memory_block_size_)
                {
                    Log::Fatal("Rank %d: corpus_size_ > memory_block_size when reading file \n",
                               Multiverso::ProcessRank());
                }
                DataBlockInterface_->GenerateDocuments();
                has_read_ = true;
            }

            /* write from buffer to destination=MongoDB*/
            void Write(int32_t block_idx)
            {
                CheckDBParameters();
                //WriteTrainingData();
                for(int64_t docIdx(0); docIdx<DataBlockInterface_->num_document_; docIdx++)
                {
                    int32_t* docBuffer_ptr = nullptr;
                    int64_t offsetStart = DataBlockInterface_->offset_buffer_[docIdx];
                    int64_t offsetEnd = DataBlockInterface_->offset_buffer_[docIdx + 1];

                    docBuffer_ptr = DataBlockInterface_->documents_buffer_ + offsetStart;
                    std::vector<Token> doc_tokens;
                    for(int64_t tokenIdx(offsetStart); tokenIdx < offsetEnd; tokenIdx++)
                    {
                        int32_t wordId = *(docBuffer_ptr + 1 + tokenIdx * 2);
                        int32_t topicId = *(docBuffer_ptr + 2 + tokenIdx * 2);
                        doc_tokens.push_back({wordId,topicId});
                        //WriteTrainingData(docIdx, wordId, topicId);
                    }
                    //WriteTrainingData(int64_t docId, int32_t wordId, int32_t topicId)
                    WriteTrainingData(block_idx, docIdx, doc_tokens);
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

            void ReadTrainingData(int32_t block_idx, int64_t docId)
            {

                auto conn = ClientToTrainingData_->acquire();
                auto trainingDataCollection = (*conn)[MongoDBName_][MongoCollectionName_];

                //*
                //if(docId % 100 == 0)
                    std::cout << "read training data in DB " << MongoDBName_
                              << " collection " << MongoCollectionName_
                              << " block id " << block_idx
                              << " doc id " << docId << std::endl;
                //*/

                // filter
                auto filter = bsoncxx::builder::stream::document{}
                        << "block_idx" << block_idx
                        << "docId" << docId
                        << bsoncxx::builder::stream::finalize;
                mongocxx::options::find opts{};
                opts.projection(bsoncxx::builder::stream::document{} << "tokenIds" << 1 << bsoncxx::builder::stream::finalize);
                opts.no_cursor_timeout(true);

                auto trainingCursor = trainingDataCollection.find(filter.view(), opts);

                for (auto &&doc : trainingCursor)
                {
                    bsoncxx::document::element ele_id;
                    
                    if ((ele_id = doc["tokenIds"]) )
                    {
                        bsoncxx::array::view observedWordArray{ele_id.get_array().value};
                        // loop over visited items (array)

                        int doc_token_count = 0;
                        std::vector<Token> doc_tokens;

                        //bsoncxx::document::view tokenView;
                        for(const auto& observedWord : observedWordArray)
                        {
                            if(observedWord.type() == type::k_document)
                            {
                                if (doc_token_count >= kMaxDocLength) break;
                                //bsoncxx::document::view tokenView = observedWord.get_document().value;
                                bsoncxx::document::view tokenView = observedWord.get_document().view();

                                bsoncxx::document::element word_ele = tokenView["wordId"];
                                bsoncxx::document::element topic_ele = tokenView["topicId"];

                                int32_t wordId = -1;
                                int32_t topicId = -1;

                                if (word_ele && word_ele.type() == bsoncxx::type::k_int32)
                                    wordId = word_ele.get_int32().value;

                                if (topic_ele && topic_ele.type() == bsoncxx::type::k_int32)
                                    topicId = topic_ele.get_int32().value;



                                if(wordId >= 0 && topicId >= 0)
                                {
                                    //std::cout << "word id = " << wordId << "  topicId" << topicId << std::endl;
                                    doc_tokens.push_back({ wordId, topicId });
                                }
                                //else
                                //    Log::Fatal("[ERROR] word_id or/and topic_id not found in MongoDB, and is/are required for the LightLDA document buffer.\n");
                            }

                        }// end loop over words

                        //db array already sorted at writting side
                        //*
                         std::sort(doc_tokens.begin(), doc_tokens.end(), [](const Token& token1, const Token& token2)
                        {
                            return token1.word_id < token2.word_id;
                        });
                        //*/

                        
                        DataBlockInterface_->documents_buffer_[doc_buf_idx_++] = 0;
                        //*
                        for (auto& token : doc_tokens)
                        {
                            DataBlockInterface_->documents_buffer_[doc_buf_idx_++] = token.word_id;
                            DataBlockInterface_->documents_buffer_[doc_buf_idx_++] = token.topic_id;
                        }//*/
                        
                        //DataBlockInterface_->offset_buffer_[docId + 1] = DataBlockInterface_->offset_buffer_[j] + doc_buf_idx_;
                        DataBlockInterface_->offset_buffer_[docId + 1] = doc_buf_idx_;
                    }
                }// end loop over doc



            }



            //////////////////////////
            void WriteTrainingData(int32_t block_idx, int64_t docId, std::vector<Token>& doc_tokens)
            {
                auto conn = ClientToTrainingData_->acquire();
                //auto recProcCollection = (*conn)[s_RecProcDB][s_RecProcTrainCollection];
                auto trainingDataCollection = (*conn)[MongoDBName_][MongoCollectionName_];

                // upsert options
                mongocxx::options::update updateOpts;
                updateOpts.upsert(true);

                // filter
                auto filter = bsoncxx::builder::stream::document{}
                        << "block_idx" << block_idx
                        << "docId" << docId
                        << bsoncxx::builder::stream::finalize;


                auto token_array = bsoncxx::builder::stream::array{};
                token_array << make_doctokens_convertor(&doc_tokens);
                //open doc
                bsoncxx::builder::stream::document doc{};

                // code below ok when creating the collection for the first time,
                doc << "$set" << bsoncxx::builder::stream::open_document
                    << "block_idx" << block_idx
                    << "docId" << docId
                    << "tokenIds"
                    << bsoncxx::builder::stream::open_array
                    << bsoncxx::builder::concatenate(token_array.view())
                    << bsoncxx::builder::stream::close_array
                    << bsoncxx::builder::stream::close_document;

                // update
                bsoncxx::document::value fUpdate = doc << bsoncxx::builder::stream::finalize;
                trainingDataCollection.update_one(filter.view(), std::move(fUpdate), updateOpts);

                has_read_ = false;

            }

            void CheckDBParameters()
            {
                // some checks...
                if(MongoUri_.empty())
                    Log::Fatal("[DataBlock] Mongo uri is not defined, program will now exit \n");

                if(MongoDBName_.empty())
                    Log::Fatal("[DataBlock] Mongo DataBase Name is not defined, program will now exit \n");

                if(MongoCollectionName_.empty())
                    Log::Fatal("[DataBlock] Mongo Collection Name is not defined, program will now exit \n");

                if(!ClientToTrainingData_)
                    Log::Fatal("[DataBlock] Mongo pool is not not valid, program will now exit \n");
            }

            //////////////////////////

        private:
            //int32_t *doc_buf_;
            DataBlockInterface<DataBlockMongoDBPolicy>* DataBlockInterface_;
            std::string MongoDBName_;
            std::string MongoCollectionName_;
            std::string MongoUri_;
            int32_t doc_buf_idx_;
            bool has_read_;
            const int32_t kMaxDocLength;
            std::unique_ptr<mongocxx::pool> ClientToTrainingData_;
        };
    } // namespace lightlda
} // namespace multiverso


#endif //LIGHTLDA_DATABLOCKDBPOLICY_H
