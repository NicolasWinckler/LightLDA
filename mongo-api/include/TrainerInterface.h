//
// Created by nw on 29.03.17.
//

#ifndef LIGHTLDA_TRAINERINTERFACE_H
#define LIGHTLDA_TRAINERINTERFACE_H
/*!
 * \file TrainerInterface.h
 * \brief Defines multiverso interface for parameter loading and data training
 */

#include <mutex>

#include <multiverso/multiverso.h>
#include <multiverso/barrier.h>

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            class AliasTable;
            class LDADataBlock;
            class LightDocSampler;
            template<typename T>
            class PSModel;

            /*! \brief Trainer is responsible for training a data block */

            template <typename meta_type>
            class Trainer : public TrainerBase
            {
                typedef Trainer<meta_type> self_type;
                typedef PSModel<self_type> PSModel_type;
            public:
                Trainer(AliasTable* alias, Barrier* barrier, meta_type* meta);
                ~Trainer();
                /*!
                 * \brief Defines Trainning method for a data_block in one iteration
                 * \param data_block pointer to data block base
                 */
                void TrainIteration(DataBlockBase* data_block) override;
                /*!
                 * \brief Evaluates a data block, compute its loss function
                 * \param block pointer to data block
                 */
                void Evaluate(LDADataBlock* block);

                void Dump(int32_t iter, LDADataBlock* lda_data_block);

            private:
                /*! \brief alias table, for alias access */
                AliasTable* alias_;
                /*! \brief sampler for lightlda */
                LightDocSampler* sampler_;
                /*! \brief barrier for thread-sync */
                Barrier* barrier_;
                /*! \brief meta information */
                meta_type* meta_;
                /*! \brief model acceccor */
                PSModel_type* model_;
                static std::mutex mutex_;

                static double doc_llh_;
                static double word_llh_;
            };

            /*!
             * \brief ParamLoader is responsible for parsing a data block and
             *        preload parameters needed by this block
             */
            class ParamLoader : public ParameterLoaderBase
            {
                /*!
                 * \brief Parse a data block to record which parameters (word) is
                 *        needed for training this block
                 */
                void ParseAndRequest(DataBlockBase* data_block) override;
            };

        } // namespace dev
    } // namespace lightlda
} // namespace multiverso


#endif //LIGHTLDA_TRAINERINTERFACE_H
