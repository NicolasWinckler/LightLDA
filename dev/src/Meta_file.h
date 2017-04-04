//
// Created by nw on 28.03.17.
//

#ifndef LIGHTLDA_META_FILE_H
#define LIGHTLDA_META_FILE_H

#include "MetaBase.h"

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            class Meta_file : public MetaBase<Meta_file>
            {
                //typedef MetaBase<Meta_file> base_type;
                // functions members
            public:
                using base_type::tf;
                using base_type::local_tf;
                using base_type::local_vocab;
                using base_type::alias_index;
                using base_type::ModelSchedule;
                using base_type::ModelSchedule4Inference;
                using base_type::BuildAliasIndex;

                // data members
                using base_type::local_vocabs_;
                using base_type::tf_;
                using base_type::local_tf_;
                using base_type::alias_index_;


                Meta_file() : base_type(), fileNameWOExt_("vocab") {}
                virtual ~Meta_file() {}

                void SetFileNameWOExt(const std::string& filename)
                {
                    fileNameWOExt_=filename;
                }

                void Init()
                {
                    tf_.resize(Config::num_vocabs, 0);
                    local_tf_.resize(Config::num_vocabs, 0);
                    int32_t* tf_buffer = new int32_t[Config::num_vocabs];
                    int32_t* local_tf_buffer = new int32_t[Config::num_vocabs];
                    local_vocabs_.resize(Config::num_blocks);
                    for (int32_t i = 0; i < Config::num_blocks; ++i)
                    {
                        auto& local_vocab = local_vocabs_[i];

                        std::string file_name = Config::input_dir +
                                                "/" + fileNameWOExt_ +
                                                "." + std::to_string(i);
                        std::ifstream vocab_file(file_name, std::ios::in|std::ios::binary);

                        if (!vocab_file.good())
                        {
                            Log::Fatal("Failed to open file : %s\n", file_name.c_str());
                        }

                        vocab_file.read(reinterpret_cast<char*>(&local_vocab.size_),
                                        sizeof(int));
                        local_vocab.vocabs_ = new int[local_vocab.size_];
                        local_vocab.own_memory_ = true;
                        vocab_file.read(reinterpret_cast<char*>(local_vocab.vocabs_),
                                        sizeof(int)*  local_vocab.size_);
                        vocab_file.read(reinterpret_cast<char*>(tf_buffer),
                                        sizeof(int)*  local_vocab.size_);
                        vocab_file.read(reinterpret_cast<char*>(local_tf_buffer),
                                        sizeof(int)*  local_vocab.size_);

                        vocab_file.close();

                        for (int32_t i = 0; i < local_vocab.size_; ++i)
                        {
                            if (tf_buffer[i] > tf_[local_vocab.vocabs_[i]])
                            {
                                tf_[local_vocab.vocabs_[i]] = tf_buffer[i];
                            }
                            if (local_tf_buffer[i] > local_tf_[local_vocab.vocabs_[i]])
                            {
                                local_tf_[local_vocab.vocabs_[i]] = local_tf_buffer[i];
                            }
                        }
                    }

                    delete[] local_tf_buffer;
                    delete[] tf_buffer;

                    if(!Config::inference)
                    {
                        ModelSchedule();
                    }
                    else
                    {
                        ModelSchedule4Inference();
                    }
                    BuildAliasIndex();
                }

            private:
                std::string fileNameWOExt_;//file name without extension
            };
        }// namespace dev
    } // namespace lightlda
} // namespace multiverso
#endif //LIGHTLDA_META_FILE_H
