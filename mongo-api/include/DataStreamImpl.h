//
// Created by nw on 27.03.17.
//

#ifndef LIGHTLDA_DATASTREAMIMPL_H
#define LIGHTLDA_DATASTREAMIMPL_H

#include "common.h"
#include "data_block.h"

#include <vector>
#include <thread>

#include <multiverso/double_buffer.h>

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /*! \brief implementation of  DiskDataStream */
            template<typename T>
            class DiskDataStream_impl {
                typedef T block_type;
                typedef DoubleBuffer<block_type> DataBuffer;
            public:
                DiskDataStream_impl(int32_t num_blocks, std::string data_path,
                                    int32_t num_iterations);

                virtual ~DiskDataStream_impl();

                void BeforeDataAccess();

                void EndDataAccess();

                block_type &CurrDataBlock();

            private:
                /*! \brief Background data thread entrance function */
                void DataPreloadMain();

                /*! \brief buffer for data */
                block_type *buffer_0;
                block_type *buffer_1;
                DataBuffer *data_buffer_;
                /*! \brief current block id to be accessed */
                int32_t block_id_;
                /*! \brief number of data blocks in disk */
                int32_t num_blocks_;
                /*! \brief number of training iterations */
                int32_t num_iterations_;
                /*! \brief data path */
                std::string data_path_;
                /*! \brief backend thread for data preload */
                std::thread preload_thread_;
                bool working_;

                // No copying allowed
                DiskDataStream_impl(const DiskDataStream_impl &);

                void operator=(const DiskDataStream_impl &);
            };

            template<typename T>
            DiskDataStream_impl<T>::DiskDataStream_impl(int32_t num_blocks,
                                                        std::string data_path, int32_t num_iterations) :
                    num_blocks_(num_blocks), data_path_(data_path),
                    num_iterations_(num_iterations), working_(false) {
                block_id_ = 0;
                buffer_0 = new block_type();
                buffer_1 = new block_type();
                data_buffer_ = new DataBuffer(1, buffer_0, buffer_1);
                preload_thread_ = std::thread(&DiskDataStream_impl::DataPreloadMain, this);
                while (!working_) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            }

            template<typename T>
            DiskDataStream_impl<T>::~DiskDataStream_impl() {
                preload_thread_.join();
                if (data_buffer_ != nullptr) {
                    delete data_buffer_;
                    data_buffer_ = nullptr;
                    delete buffer_1;
                    buffer_1 = nullptr;
                    delete buffer_0;
                    buffer_0 = nullptr;
                }
            }

            template<typename T>
            T &DiskDataStream_impl<T>::CurrDataBlock() {
                return data_buffer_->WorkerBuffer();
            }

            template<typename T>
            void DiskDataStream_impl<T>::BeforeDataAccess() {
                data_buffer_->Start(1);
            }

            template<typename T>
            void DiskDataStream_impl<T>::EndDataAccess() {
                data_buffer_->End(1);
            }

            template<typename T>
            void DiskDataStream_impl<T>::DataPreloadMain() {
                int32_t block_id = 0;
                std::string block_file = data_path_ + "/block."
                                         + std::to_string(block_id);
                data_buffer_->Start(0);
                data_buffer_->IOBuffer().Read(block_file);
                data_buffer_->End(0);
                working_ = true;
                for (int32_t iter = 0; iter <= num_iterations_; ++iter) {
                    for (int32_t block_id = 0; block_id < num_blocks_; ++block_id) {
                        data_buffer_->Start(0);

                        block_type &data_block = data_buffer_->IOBuffer();
                        if (data_block.HasLoad()) {
                            data_block.Write();
                        }
                        if (iter == num_iterations_ && block_id == num_blocks_ - 1) {
                            break;
                        }
                        // Load New data;
                        int32_t next_block_id = (block_id + 1) % num_blocks_;
                        block_file = data_path_ + "/block." +
                                     std::to_string(next_block_id);
                        data_block.Read(block_file);
                        data_buffer_->End(0);
                    }
                }
            }



            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /*! \brief implementation of  MemoryDataStream */
            template<typename T>
            class MemoryDataStream_impl {
                typedef T block_type;
            public:
                MemoryDataStream_impl(int32_t num_blocks, std::string data_path);

                virtual ~MemoryDataStream_impl();

                void BeforeDataAccess();

                void EndDataAccess();

                block_type &CurrDataBlock();

            private:
                std::vector<block_type *> data_buffer_;
                std::string data_path_;
                int32_t index_;

                // No copying allowed
                MemoryDataStream_impl(const MemoryDataStream_impl &);

                void operator=(const MemoryDataStream_impl &);
            };

            template<typename T>
            MemoryDataStream_impl<T>::MemoryDataStream_impl(int32_t num_blocks, std::string data_path)
                    : data_path_(data_path), index_(0) {
                data_buffer_.resize(num_blocks, nullptr);
                for (int32_t i = 0; i < num_blocks; ++i) {
                    data_buffer_[i] = new block_type();
                    data_buffer_[i]->Read(data_path_ + "/block."
                                          + std::to_string(i));
                }
            }

            template<typename T>
            MemoryDataStream_impl<T>::~MemoryDataStream_impl() {
                for (auto &data : data_buffer_) {
                    data->Write();
                    delete data;
                    data = nullptr;
                }
            }

            template<typename T>
            void MemoryDataStream_impl<T>::BeforeDataAccess() {
                index_ %= data_buffer_.size();
            }

            template<typename T>
            void MemoryDataStream_impl<T>::EndDataAccess() {
                ++index_;
            }

            template<typename T>
            T &MemoryDataStream_impl<T>::CurrDataBlock() {
                return *data_buffer_[index_];
            }

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /*! \brief implementation of  DBDataStream */











        } // namespace dev
    } // namespace lightlda
} // namespace multiverso


#endif //LIGHTLDA_DATASTREAMIMPL_H
