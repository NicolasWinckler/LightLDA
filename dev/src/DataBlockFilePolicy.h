//
// Created by nw on 31.03.17.
//

#ifndef LIGHTLDA_DATABLOCKFILEPOLICY_H
#define LIGHTLDA_DATABLOCKFILEPOLICY_H

#include "document.h"
#include "common.h"
#include "DataBlockInterface.h"
#include <multiverso/log.h>

#include <fstream>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <stdio.h>
#endif

#include <iostream>

namespace
{
    void AtomicMoveFileExA(std::string existing_file, std::string new_file)
    {
#if defined(_WIN32) || defined(_WIN64)
        MoveFileExA(existing_file.c_str(), new_file.c_str(),
            MOVEFILE_REPLACE_EXISTING);
#else
        if (rename(existing_file.c_str(), new_file.c_str()) == -1)
        {
            multiverso::Log::Error("Failed to move tmp file to final location\n");
        }
#endif
    }
}






namespace multiverso
{
    namespace lightlda
    {

        class DataBlockFilePolicy
        {
            //friend class DataBlockInterface<DataBlockFilePolicy>;

        public:
            DataBlockFilePolicy() :
                    DataBlockInterface_(nullptr),
                    file_name_(),
                    has_read_(false)
            {

            }
            ~DataBlockFilePolicy(){}
            void Init(DataBlockInterface<DataBlockFilePolicy>* interface)
            {
                DataBlockInterface_ = interface;
            }

            void SetFileName(std::string file_name)
            {
                file_name_ = file_name;
            }

            bool HasLoad() const
            {
                return has_read_;
            }



            //////////////////////////
            void Read(int32_t block_id)
            {
                //std::string file_name_ = data_path_ + "/block." + std::to_string(block_id);
                std::ifstream block_file(file_name_, std::ios::in | std::ios::binary);
                if (!block_file.good())
                {
                    Log::Fatal("Failed to read data %s\n", file_name_.c_str());
                }
                block_file.read(reinterpret_cast<char*>(&DataBlockInterface_->num_document_), sizeof(DocNumber));

                if (DataBlockInterface_->num_document_ > DataBlockInterface_->max_num_document_)
                {
                    Log::Fatal("Rank %d: Num of documents > max number of documents when reading file %s\n",
                               Multiverso::ProcessRank(), file_name_.c_str());
                }

                block_file.read(reinterpret_cast<char*>(DataBlockInterface_->offset_buffer_),
                                sizeof(int64_t)* (DataBlockInterface_->num_document_ + 1));

                DataBlockInterface_->corpus_size_ = DataBlockInterface_->offset_buffer_[DataBlockInterface_->num_document_];
                std::cout << "------------------------------->  max_num_document_ =" << DataBlockInterface_->max_num_document_ << std::endl;
                std::cout << "------------------------------->  num_document_ =" << DataBlockInterface_->num_document_ << std::endl;
                std::cout << "------------------------------->  offset_buffer_[num_document_] =" << DataBlockInterface_->offset_buffer_[DataBlockInterface_->num_document_] << std::endl;
                
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

            void Write(int32_t block_id)
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
            DataBlockInterface<DataBlockFilePolicy>* DataBlockInterface_;
            std::string file_name_;
            bool has_read_;
        };
    } // namespace lightlda
} // namespace multiverso

#endif //LIGHTLDA_DATABLOCKFILEPOLICY_H
