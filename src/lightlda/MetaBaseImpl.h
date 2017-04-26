//
// Created by nw on 28.03.17.
//

#ifndef LIGHTLDA_METABASEIMPL_H
#define LIGHTLDA_METABASEIMPL_H

#include "common.h"

#include <fstream>
#include <multiverso/log.h>


namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            template<typename T>
            MetaBase<T>::MetaBase() {
            }

            template<typename T>
            MetaBase<T>::~MetaBase()
            {
                for (int32_t i = 0; i < alias_index_.size(); ++i)
                {
                    for (int32_t j = 0; j < alias_index_[i].size(); ++j)
                    {
                        delete alias_index_[i][j];
                    }
                }
            }


            template<typename T>
            void MetaBase<T>::ModelSchedule()
            {
                int64_t model_capacity = Config::model_capacity;
                int64_t alias_capacity = Config::alias_capacity;
                int64_t delta_capacity = Config::delta_capacity;

                int32_t model_thresh = Config::num_topics / (2 * kLoadFactor);
                int32_t alias_thresh = (Config::num_topics * 2) / 3;
                int32_t delta_thresh = Config::num_topics / (4 * kLoadFactor);


                // Schedule for each data block
                for (int32_t i = 0; i < Config::num_blocks; ++i) {
                    auto &local_vocab = local_vocabs_[i];
                    int32_t *vocabs = local_vocab.vocabs_;
                    local_vocab.slice_index_.push_back(0);

                    int64_t model_offset = 0;
                    int64_t alias_offset = 0;
                    int64_t delta_offset = 0;
                    for (int32_t j = 0; j < local_vocab.size_; ++j)
                    {
                        int32_t word = vocabs[j];
                        int32_t tf = tf_[word];
                        int32_t local_tf = local_tf_[word];
                        int32_t model_size = (tf > model_thresh) ?
                                             Config::num_topics * sizeof(int32_t) :
                                             tf * kLoadFactor * sizeof(int32_t);
                        model_offset += model_size;

                        int32_t alias_size = (tf > alias_thresh) ?
                                             Config::num_topics * 2 * sizeof(int32_t) :
                                             tf * 3 * sizeof(int32_t);
                        alias_offset += alias_size;

                        int32_t delta_size = (local_tf > delta_thresh) ?
                                             Config::num_topics * sizeof(int32_t) :
                                             local_tf * kLoadFactor * 2 * sizeof(int32_t);
                        delta_offset += delta_size;

                        if (model_offset > model_capacity ||
                            alias_offset > alias_capacity ||
                            delta_offset > delta_capacity)
                        {
                            Log::Info("Actual Model capacity: %d MB, Alias capacity: %d MB, Delta capacity: %dMB\n",
                                      model_offset / 1024 / 1024, alias_offset / 1024 / 1024,
                                      delta_offset / 1024 / 1024);
                            local_vocab.slice_index_.push_back(j);
                            ++local_vocab.num_slices_;
                            model_offset = model_size;
                            alias_offset = alias_size;
                            delta_offset = delta_size;
                        }
                    }
                    local_vocab.slice_index_.push_back(local_vocab.size_);
                    ++local_vocab.num_slices_;
                    Log::Info("INFO: block = %d, the number of slice = %d\n",
                              i, local_vocab.num_slices_);
                }
            }

            template<typename T>
            void MetaBase<T>::ModelSchedule4Inference()
            {
                Config::alias_capacity = 0;
                int32_t alias_thresh = (Config::num_topics * 2) / 3;
                // Schedule for each data block
                for (int32_t i = 0; i < Config::num_blocks; ++i)
                {
                    auto& local_vocab = local_vocabs_[i];
                    int32_t *vocabs = local_vocab.vocabs_;
                    local_vocab.slice_index_.push_back(0);
                    local_vocab.slice_index_.push_back(local_vocab.size_);
                    local_vocab.num_slices_ = 1;
                    int64_t alias_offset = 0;
                    for (int32_t j = 0; j < local_vocab.size_; ++j)
                    {
                        int32_t word = vocabs[j];
                        int32_t tf = tf_[word];
                        int32_t alias_size = (tf > alias_thresh) ?
                                             Config::num_topics * 2 * sizeof(int32_t) :
                                             tf * 3 * sizeof(int32_t);
                        alias_offset += alias_size;
                    }
                    if (alias_offset > Config::alias_capacity)
                    {
                        Config::alias_capacity = alias_offset;
                    }
                }
                Log::Info("Actual Alias capacity: %d MB\n", Config::alias_capacity / 1024 / 1024);
            }

            template<typename T>
            void MetaBase<T>::BuildAliasIndex()
            {
                int32_t alias_thresh = (Config::num_topics * 2) / 3;
                alias_index_.resize(Config::num_blocks);
                // for each block
                for (int32_t i = 0; i < Config::num_blocks; ++i)
                {
                    const auto &vocab = local_vocab(i);
                    alias_index_[i].resize(vocab.num_slice());
                    // for each slice
                    for (int32_t j = 0; j < vocab.num_slice(); ++j)
                    {
                        alias_index_[i][j] = new AliasTableIndex();
                        int64_t offset = 0;
                        for (const int32_t *p = vocab.begin(j); p != vocab.end(j); ++p)
                        {
                            int32_t word = *p;
                            bool is_dense = true;
                            int32_t capacity = Config::num_topics;
                            int64_t size = Config::num_topics * 2;
                            if (tf(word) <= alias_thresh)
                            {
                                is_dense = false;
                                capacity = tf(word);
                                size = tf(word) * 3;
                            }
                            alias_index_[i][j]->PushWord(word, is_dense, offset, capacity);
                            offset += size;
                        }
                    }
                }
            }
        }// namespace dev
    } // namespace lightlda
} // namespace multiverso

#endif //LIGHTLDA_METABASEIMPL_H
