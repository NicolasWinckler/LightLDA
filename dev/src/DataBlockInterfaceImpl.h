//
// Created by nw on 31.03.17.
//

#ifndef LIGHTLDA_DATABLOCKINTERFACEIMPL_H
#define LIGHTLDA_DATABLOCKINTERFACEIMPL_H

#include "document.h"
#include "common.h"

#include <multiverso/log.h>

namespace multiverso
{
    namespace lightlda
    {
        template< typename IOPolicy>
        DataBlockInterface<IOPolicy>::DataBlockInterface() :
                IOPolicy(),
                max_num_document_(0),
                memory_block_size_(0),
                documents_(),
                num_document_(0),
                offset_buffer_(nullptr),
                corpus_size_(0),
                documents_buffer_(nullptr),
                vocab_(nullptr)
        {
            max_num_document_ = Config::max_num_document;
            memory_block_size_ = Config::data_capacity / sizeof(int32_t);

            documents_.resize(max_num_document_);

            try{
                offset_buffer_ = new int64_t[max_num_document_];
            }
            catch (std::bad_alloc& ba) {
                Log::Fatal("Bad Alloc caught: failed memory allocation for offset_buffer in DataBlockInterface\n");
            }

            try{
                documents_buffer_ = new int32_t[memory_block_size_];
            }
            catch (std::bad_alloc& ba) {
                Log::Fatal("Bad Alloc caught: failed memory allocation for documents_buffer in DataBlockInterface\n");
            }

            IOPolicy::Init(this);
        }

        template< typename IOPolicy>
        DataBlockInterface<IOPolicy>::~DataBlockInterface()
        {
            delete[] offset_buffer_;
            delete[] documents_buffer_;
        }


        template< typename IOPolicy>
        Document* DataBlockInterface<IOPolicy>::GetOneDoc(int32_t index)
        {
            return documents_[index].get();
        }

        template< typename IOPolicy>
        const LocalVocab& DataBlockInterface<IOPolicy>::meta() const
        {
            return *vocab_;
        }

        template< typename IOPolicy>
        void DataBlockInterface<IOPolicy>::set_meta(const LocalVocab* local_vocab)
        {
            vocab_ = local_vocab;
        }

        template< typename IOPolicy>
        DocNumber DataBlockInterface<IOPolicy>::Size() const
        {
            return num_document_;
        }

        template< typename IOPolicy>
        void DataBlockInterface<IOPolicy>::GenerateDocuments()
        {
            std::cout << "GenerateDocuments for " << num_document_ << "documents\n";
            for (int32_t index = 0; index < num_document_; ++index)
            {
                if(index < 10)
                    std::cout << "index = "
                              << index
                              << " offset_buffer_[index] = "
                              << offset_buffer_[index]
                              << " offset_buffer_[index + 1] "
                              << offset_buffer_[index + 1]
                              << std::endl;
                documents_[index].reset(new Document(
                        documents_buffer_ + offset_buffer_[index],
                        documents_buffer_ + offset_buffer_[index + 1]));
            }
        }



    } // namespace lightlda
} // namespace multiverso
#endif //LIGHTLDA_DATABLOCKINTERFACEIMPL_H
