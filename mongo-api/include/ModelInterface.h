//
// Created by nw on 28.03.17.
//

#ifndef LIGHTLDA_LOCALMODELINTERFACE_H
#define LIGHTLDA_LOCALMODELINTERFACE_H

#include <memory>
#include <string>

#include "common.h"
#include <multiverso/meta.h>

namespace multiverso
{
    template<typename T> class Row;
    class Table;

    namespace lightlda
    {
        namespace dev
        {
            /*! \brief interface for access to model */
            template <typename model_type>
            class ModelInterface : public model_type
            {
            public:
                template <typename... Args>
                ModelInterface(Args&&... args) : model_type(std::forward<Args>(args)...) {}

                virtual ~ModelInterface() {}

                Row<int32_t>& GetWordTopicRow(integer_t word_id)
                {
                    return model_type::GetWordTopicRow(word_id);
                }

                Row<int64_t>& GetSummaryRow()
                {
                    return model_type::GetSummaryRow();
                }

                void AddWordTopicRow(integer_t word_id, integer_t topic_id, int32_t delta)
                {
                    model_type::AddWordTopicRow(word_id,topic_id,delta);
                }

                void AddSummaryRow(integer_t topic_id, int64_t delta)
                {
                    model_type::AddSummaryRow(topic_id,delta);
                }
            };

            /*! \brief model based on local buffer */
/*
            template <typename meta_type, typename loader_type>
            class LocalModel
            {
            public:
                explicit LocalModel(meta_type * meta);

                template <typename... Args>
                void Init(Args&&... args)
                {
                    loader_type::Init(std::forward<Args>(args)...);
                }

                Row<int32_t>& GetWordTopicRow(integer_t word_id);
                Row<int64_t>& GetSummaryRow();
                void AddWordTopicRow(integer_t word_id, integer_t topic_id, int32_t delta);
                void AddSummaryRow(integer_t topic_id, int64_t delta);

            private:
                void CreateTable();
                template <typename... Args>
                void LoadTable(Args&&... args)
                {
                    loader_type::LoadTable(std::forward<Args>(args)...);
                }

                template <typename... Args>
                void LoadWordTopicTable(Args&&... args)
                {
                    loader_type::LoadWordTopicTable(std::forward<Args>(args)...);
                }

                template <typename... Args>
                void LoadSummaryTable(Args&&... args)
                {
                    loader_type::LoadSummaryTable(std::forward<Args>(args)...);
                }

                std::unique_ptr<Table> word_topic_table_;
                std::unique_ptr<Table> summary_table_;
                meta_type* meta_;

                LocalModel(const LocalModel&) = delete;
                void operator=(const LocalModel&) = delete;

            };
*/

            /*! \brief model based on parameter server */

            template <typename trainer_type>
            class PSModel
            {
            public:
                explicit PSModel(trainer_type* trainer) : trainer_(trainer) {}

                Row<int32_t>& GetWordTopicRow(integer_t word_id);
                Row<int64_t>& GetSummaryRow();
                void AddWordTopicRow(integer_t word_id, integer_t topic_id, int32_t delta);
                void AddSummaryRow(integer_t topic_id, int64_t delta);

            private:
                trainer_type* trainer_;

                PSModel(const PSModel&) = delete;
                void operator=(const PSModel&) = delete;
            };


        } // namespace dev
    } // namespace lightlda
} // namespace multiverso

#include "ModelInterfaceImpl.hpp"
#endif //LIGHTLDA_LOCALMODELINTERFACE_H
