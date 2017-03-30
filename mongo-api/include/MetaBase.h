//
// Created by nw on 28.03.17.
//

#ifndef LIGHTLDA_METABASE_H
#define LIGHTLDA_METABASE_H


/*!
 * \file MetaInterface.h
 * \brief This file defines meta information for training dataset
 */


#include <string>
#include <vector>
#include <cstdint>
#include "Vocab.h"

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            /*!
             * \brief Meta containes all the meta information of training data in
             *  current process. It containes 1) all the local vacabs for all data
             *  blocks, 2) the global tf for the training dataset
             */
            template<typename Derived>
            class MetaBase
            {
            public:
                typedef MetaBase<Derived> base_type;
                typedef Derived derived_type;
            //protected:
                //typedef LocalVocab<derived_type> LocalVocab_t;
                // protected constructor to enforce derivation
                MetaBase();

            public:


                ~MetaBase();

                /*! \brief Initialize the MetaBase information (CRTP abstract method, must be implemented)*/
                void Init()
                {
                    static_cast<Derived *>(this)->Init();
                }

                /*! \brief Get the tf of word in the whole dataset */
                int32_t tf(int32_t word) const
                {
                    return tf_[word];
                }

                /*! \brief Get the tf of word in local dataset */
                int32_t local_tf(int32_t word) const
                {
                    return local_tf_[word];
                }

                /*! \brief Get the local vocab based on block id */
                const LocalVocab<derived_type>& local_vocab(int32_t id) const
                {
                    return local_vocabs_[id];
                }

                AliasTableIndex* alias_index(int32_t block, int32_t slice)
                {
                    return alias_index_[block][slice];
                }

            protected:
                /*! \brief Schedule the model and split as slices based on memory */
                void ModelSchedule();

                /*! \brief Schedule the model without vocabulary sliptting */
                void ModelSchedule4Inference();

                /*! \brief Build index for alias table */
                void BuildAliasIndex();

            protected:
                /*! \brief meta information for all data block */
                std::vector<LocalVocab<derived_type>> local_vocabs_;
                /*! \breif tf information for all word in the dataset */
                std::vector<int32_t> tf_;
                /*! \brief local tf information for all word in this machine */
                std::vector<int32_t> local_tf_;

                std::vector<std::vector<AliasTableIndex *> > alias_index_;

            private:
                // No copying allowed
                MetaBase(const MetaBase &);

                void operator=(const MetaBase &);
            };


        } // namespace dev
    } // namespace lightlda
} // namespace multiverso

#include "MetaBaseImpl.h"



#endif //LIGHTLDA_METABASE_H
