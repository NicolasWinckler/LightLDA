//
// Created by nw on 28.03.17.
//

#ifndef LIGHTLDA_VOCAB_H
#define LIGHTLDA_VOCAB_H


#include <string>
#include <vector>
#include <cstdint>
#include <type_traits>

namespace multiverso
{
    namespace lightlda
    {
        namespace dev {
            /*!
             * \brief LocalVocab defines the meta information of a data block.
             *  It containes 1) which words occurs in this block, 2) slice information
             */
            // template parameters for the CRTP class
            template <typename BaseClass, typename DerivedClass>
            class LocalVocab
            {
            public:
                static_assert(std::is_base_of<BaseClass,DerivedClass>::value,"BaseClass must be the base class of DerivedClass");
                friend BaseClass;// template parameters
                friend DerivedClass;// template parameters

                LocalVocab()
                        : num_slices_(0),
                          own_memory_(false),
                          vocabs_(nullptr),
                          size_(0)
                {}

                ~LocalVocab()
                {
                    if (own_memory_)
                    {
                        delete[] vocabs_;
                    }
                }

                /*! \brief Get the last word of current slice */
                int32_t LastWord(int32_t slice) const
                {
                    return vocabs_[slice_index_[slice + 1] - 1];
                }

                /*! \brief Get the number of slice */
                int32_t num_slice() const
                {
                    return num_slices_;
                }

                /*! \brief Get the pointer to first word in this slice */
                const int *begin(int32_t slice) const
                {
                    return vocabs_ + slice_index_[slice];
                }

                /*! \brief Get the pointer to last word + 1 in this slice */
                const int32_t *end(int32_t slice) const
                {
                    return vocabs_ + slice_index_[slice + 1];
                }

            private:
                int32_t num_slices_;
                int32_t *vocabs_;
                int32_t size_;
                bool own_memory_;
                std::vector<int32_t> slice_index_;
            };


            /*! WordEntry struct */
            struct WordEntry {
                bool is_dense;
                int64_t begin_offset;
                int32_t capacity;
            };


            /*! AliasTableIndex class */
            class AliasTableIndex {
            public:
                AliasTableIndex();

                WordEntry &word_entry(int32_t word);

                void PushWord(int32_t word, bool is_dense,
                              int64_t begin_offset, int32_t capacity);

            private:
                std::vector<WordEntry> index_;
                std::vector<int32_t> index_map_;
            };




            // -- inline functions definition area --------------------------------- //
        }// namespace dev
    } // namespace lightlda
} // namespace multiverso
#endif //LIGHTLDA_VOCAB_H
