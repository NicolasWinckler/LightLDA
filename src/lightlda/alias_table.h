/*!
 * \file alias_table.h
 * \brief Defines alias table
 */

#ifndef LIGHTLDA_ALIAS_TABLE_H_
#define LIGHTLDA_ALIAS_TABLE_H_

#include <memory>
#include <mutex>
#include <vector>
// TODO: remove temporary changes
#include "common.h"
#include "model.h"
#include "util.h"
#include "meta.h"

#include <multiverso/lock.h>
#include <multiverso/log.h>
#include <multiverso/row.h>
#include <multiverso/row_iter.h>
#if defined(_WIN32) || defined(_WIN64)
// vs currently not support c++11 keyword thread_local
#define _THREAD_LOCAL __declspec(thread) 
#else 
#define _THREAD_LOCAL thread_local 
#endif

namespace multiverso { namespace lightlda
{
    class ModelBase;
    class xorshift_rng;
    class AliasTableIndex;

    /*!
     * \brief AliasTable is the storage for alias tables used for fast sampling
     *  from lightlda word proposal distribution. It optimize memory usage 
     *  through a hybrid storage by exploiting the sparsity of word proposal.
     *  AliasTable containes two part: 1) a memory pool to store the alias
     *  2) an index table to access each row
     */
    class AliasTable
    {
    public:
        AliasTable();
        ~AliasTable();
        /*!
         * \brief Set the table index. Must call this method before 
         */
        void Init(AliasTableIndex* table_index);
        /*!
         * \brief Build alias table for a word
         * \param word word to bulid
         * \param model access
         * \return success of not
         */
        
        template<typename ModelType>
        int Build(int word, ModelType&& model)
        {       
            if (q_w_proportion_ == nullptr)
                q_w_proportion_ = new std::vector<float>(num_topics_);
            if (q_w_proportion_int_ == nullptr)
                q_w_proportion_int_ = new std::vector<int32_t>(num_topics_);
            if (L_ == nullptr)
                L_ = new std::vector<std::pair<int32_t, int32_t>>(num_topics_);
            if (H_ == nullptr)
                H_ = new std::vector<std::pair<int32_t, int32_t>>(num_topics_);
            // Compute the proportion
            Row<int64_t>& summary_row = model->GetSummaryRow();
            if (word == -1) // build alias row for beta 
            {
                beta_mass_ = 0;
                for (int32_t k = 0; k < num_topics_; ++k)
                {
                    (*q_w_proportion_)[k] = beta_ / (summary_row.At(k) + beta_sum_);
                    beta_mass_ += (*q_w_proportion_)[k];
                }
                AliasMultinomialRNG(num_topics_, beta_mass_, beta_height_, 
                    beta_kv_vector_);
            }
            else // build alias row for word
            {            
                WordEntry& word_entry = table_index_->word_entry(word);
                Row<int32_t>& word_topic_row = model->GetWordTopicRow(word);
                int32_t size = 0;
                mass_[word] = 0;
                if (word_entry.is_dense)
                {
                    size = num_topics_;
                    for (int32_t k = 0; k < num_topics_; ++k)
                    {
                        (*q_w_proportion_)[k] = (word_topic_row.At(k) + beta_)
                            / (summary_row.At(k) + beta_sum_);
                        mass_[word] += (*q_w_proportion_)[k];
                    }
                }
                else // word_entry.is_dense = false
                {
                    word_entry.capacity = word_topic_row.NonzeroSize();
                    int32_t* idx_vector = memory_block_ + word_entry.begin_offset 
                        + 2 * word_entry.capacity;
                    Row<int32_t>::iterator iter = word_topic_row.Iterator();
                    while (iter.HasNext())
                    {
                        int32_t t = iter.Key();
                        int32_t n_tw = iter.Value();
                        int64_t n_t = summary_row.At(t);
                        idx_vector[size] = t;
                        (*q_w_proportion_)[size] = (n_tw) / (n_t + beta_sum_);
                        mass_[word] += (*q_w_proportion_)[size];
                        ++size;
                        iter.Next();
                    }
                    if (size == 0)
                    {
                        Log::Error("Fail to build alias row, capacity of row = %d\n",
                            word_topic_row.NonzeroSize());
                    }
                }
                AliasMultinomialRNG(size, mass_[word], height_[word], 
                    memory_block_ + word_entry.begin_offset);
            }
            return 0;
        }
        /*!
         * \brief sample from word proposal distribution
         * \param word word to sample
         * \param rng random number generator
         * \return sample proposed from the distribution
         */
        int Propose(int word, xorshift_rng& rng);
        /*! \brief Clear the alias table */
        void Clear();
    private:
        void AliasMultinomialRNG(int32_t size, float mass, int32_t& height,
            int32_t* kv_vector);
        int* memory_block_;
        int64_t memory_size_;
        AliasTableIndex* table_index_;

        std::vector<int32_t> height_;
        std::vector<float> mass_;
        int32_t beta_height_;
        float beta_mass_;

        int32_t* beta_kv_vector_;

        // thread local storage used for building alias
        _THREAD_LOCAL static std::vector<float>* q_w_proportion_;
        _THREAD_LOCAL static std::vector<int>* q_w_proportion_int_;
        _THREAD_LOCAL static std::vector<std::pair<int, int>>* L_;
        _THREAD_LOCAL static std::vector<std::pair<int, int>>* H_;

        int num_vocabs_;
        int num_topics_;
        float beta_;
        float beta_sum_;

        // No copying allowed
        AliasTable(const AliasTable&);
        void operator=(const AliasTable&);
    };
} // namespace lightlda
} // namespace multiverso
#endif // LIGHTLDA_ALIAS_TABLE_H_
