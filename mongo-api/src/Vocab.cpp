//
// Created by nw on 28.03.17.
//

#include "Vocab.h"
#include "common.h"

#include <fstream>
#include <multiverso/log.h>

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            AliasTableIndex::AliasTableIndex()
            {
                index_map_.resize(Config::num_vocabs, -1);
            }

            WordEntry& AliasTableIndex::word_entry(int32_t word)
            {
                if (index_map_[word] == -1)
                {
                    Log::Fatal("Fatal in alias index: word %d not exist\n", word);
                }
                return index_[index_map_[word]];
            }

            void AliasTableIndex::PushWord(int32_t word, bool is_dense,
                                           int64_t begin_offset, int32_t capacity)
            {
                index_map_[word] = static_cast<int>(index_.size());
                index_.push_back({ is_dense, begin_offset, capacity });
            }

        } // namespace dev
    } // namespace lightlda
} // namespace multiverso
