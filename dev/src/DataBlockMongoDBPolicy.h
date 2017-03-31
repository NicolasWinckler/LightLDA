//
// Created by nw on 31.03.17.
//

#ifndef LIGHTLDA_DATABLOCKDBPOLICY_H
#define LIGHTLDA_DATABLOCKDBPOLICY_H

#include "document.h"
#include "common.h"
#include "DataBlockInterface.h"
#include <multiverso/log.h>





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

        class DataBlockMongoDBPolicy
        {

            //friend class DataBlockInterface<DataBlockMongoDBPolicy>;

        public:
            DataBlockMongoDBPolicy() :
                    DataBlockInterface_(nullptr),
                    file_name_(),
                    has_read_(false)
            {

            }
            ~DataBlockMongoDBPolicy(){}



            void Init(DataBlockInterface<DataBlockMongoDBPolicy>* interface)
            {
                DataBlockInterface_ = interface;
            }


            bool HasLoad() const
            {
                return has_read_;
            }

            void Read()
            {
                ReadTrainingData();
            }


            void ReadTrainingData2()
            {

                auto conn = ClientToTrainingData_->acquire();
                auto trainingDataCollection = (*conn)[MongoDBName_][MongoCollectionName_];

                bsoncxx::builder::stream::document filter_builder;
                mongocxx::options::find opts{};
                opts.no_cursor_timeout(true);

                auto trainingCursor = trainingDataCollection.find(filter_builder.view(), opts);

                static_assert( std::is_same< DocNumber, decltype(trainingDataCollection.count()) >, "DocNumber (aka int64_t) returned type of ");

                DataBlockInterface_->num_document_ = trainingDataCollection.count({});


                if (DataBlockInterface_->num_document_ > DataBlockInterface_->max_num_document_)
                {
                    Log::Fatal("Rank %d: Num of documents > max number of documents when reading MongoDB %s, collection %s \n",
                               Multiverso::ProcessRank(), MongoDBName_.c_str(), MongoCollectionName_.c_str());
                }

                // TODO: change BSON format to be consistent with block.0, block.1 etc..
                // loop over <user,[items]> pairs
                for (auto &&doc : trainingCursor)
                {
                    //assume that user are unique
                    std::vector<int> userRow(itemSize);
                    bsoncxx::document::element ele_id;

                    if ((ele_id = doc["visiteditems"]))
                    {
                        bsoncxx::array::view visitedItemsArray{ele_id.get_array().value};
                        // loop over visited items (array)

                        for(const auto& visitedItem : visitedItemsArray)
                        {
                            std::string itemId = mongodbutil::GetElementAsString(visitedItem);
                            if (itemId != "")
                                userRow[m_ProductIdsMap.at(itemId)]++;
                        }// end loop over visited items

                        prefMatrix.push_back(userRow);
                    }

                }// end loop over user

            }



            //////////////////////////
            void ReadTrainingData()
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

                block_file.read(reinterpret_cast<char*>(DataBlockInterface_->offset_buffer_),
                                sizeof(int64_t)* (DataBlockInterface_->num_document_ + 1));

                DataBlockInterface_->corpus_size_ = DataBlockInterface_->offset_buffer_[DataBlockInterface_->num_document_];

                if (DataBlockInterface_->corpus_size_ > DataBlockInterface_->memory_block_size_)
                {
                    Log::Fatal("Rank %d: corpus_size_ > memory_block_size when reading file %s\n",
                               Multiverso::ProcessRank(), file_name_.c_str());
                }

                block_file.read(reinterpret_cast<char*>(DataBlockInterface_->documents_buffer_),
                                sizeof(int32_t)* DataBlockInterface_->corpus_size_);
                block_file.close();

                DataBlockInterface_->GenerateDocuments();
                has_read_ = true;
            }

            void Write()
            {
                std::string temp_file = file_name_ + ".temp";

                std::ofstream block_file(temp_file, std::ios::out | std::ios::binary);

                if (!block_file.good())
                {
                    Log::Fatal("Failed to open file %s\n", temp_file.c_str());
                }

                block_file.write(reinterpret_cast<char*>(&DataBlockInterface_->num_document_),
                                 sizeof(DocNumber));
                block_file.write(reinterpret_cast<char*>(DataBlockInterface_->offset_buffer_),
                                 sizeof(int64_t)* (DataBlockInterface_->num_document_ + 1));
                block_file.write(reinterpret_cast<char*>(DataBlockInterface_->documents_buffer_),
                                 sizeof(int32_t)* DataBlockInterface_->corpus_size_);
                block_file.flush();
                block_file.close();

                AtomicMoveFileExA(temp_file, file_name_);
                has_read_ = false;
            }
            //////////////////////////

        private:
            DataBlockInterface<DataBlockMongoDBPolicy>* DataBlockInterface_;
            std::string MongoDBName_;
            std::string MongoCollectionName_;
            std::unique_ptr<mongocxx::pool> ClientToTrainingData_;
            bool has_read_;
        };
    } // namespace lightlda
} // namespace multiverso


#endif //LIGHTLDA_DATABLOCKDBPOLICY_H
