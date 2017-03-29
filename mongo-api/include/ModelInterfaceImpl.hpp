//
// Created by nw on 29.03.17.
//

#ifndef LIGHTLDA_MODELINTERFACEIMPL_HPP
#define LIGHTLDA_MODELINTERFACEIMPL_HPP

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            template <typename meta_type, typename loader_type>
            LocalModel<meta_type,loader_type>::LocalModel(Meta * meta) : word_topic_table_(nullptr),
                                                  summary_table_(nullptr), meta_(meta)
            {
                CreateTable();
            }


            template <typename meta_type, typename loader_type>
            void LocalModel<meta_type,loader_type>::CreateTable()
            {
                int32_t num_vocabs = Config::num_vocabs;
                int32_t num_topics = Config::num_topics;
                multiverso::Format dense_format = multiverso::Format::Dense;
                multiverso::Format sparse_format = multiverso::Format::Sparse;
                Type int_type = Type::Int;
                Type longlong_type = Type::LongLong;

                word_topic_table_.reset(new Table(kWordTopicTable, num_vocabs, num_topics, int_type, dense_format));
                summary_table_.reset(new Table(kSummaryRow, 1, num_topics, longlong_type, dense_format));
            }

            template <typename meta_type, typename loader_type>
            void LocalModel<meta_type,loader_type>::AddWordTopicRow(integer_t word_id, integer_t topic_id, int32_t delta)
            {
                Log::Fatal("Not implemented yet\n");
            }

            template <typename meta_type, typename loader_type>
            void LocalModel<meta_type,loader_type>::AddSummaryRow(integer_t topic_id, int64_t delta)
            {
                Log::Fatal("Not implemented yet\n");
            }

            template <typename meta_type, typename loader_type>
            Row<int32_t>& LocalModel<meta_type,loader_type>::GetWordTopicRow(integer_t word)
            {
                return *(static_cast<Row<int32_t>*>(word_topic_table_->GetRow(word)));
            }

            template <typename meta_type, typename loader_type>
            Row<int64_t>& LocalModel<meta_type,loader_type>::GetSummaryRow()
            {
                return *(static_cast<Row<int64_t>*>(summary_table_->GetRow(0)));
            }

            ////////////////////////////////////////////////////////////////////////////////

            template <typename trainer_type>
            Row<int32_t>& PSModel<trainer_type>::GetWordTopicRow(integer_t word_id)
            {
                return trainer_->GetRow<int32_t>(kWordTopicTable, word_id);
            }

            template <typename trainer_type>
            Row<int64_t>& PSModel<trainer_type>::GetSummaryRow()
            {
                return trainer_->GetRow<int64_t>(kSummaryRow, 0);
            }

            template <typename trainer_type>
            void PSModel<trainer_type>::AddWordTopicRow(integer_t word_id, integer_t topic_id, int32_t delta)
            {
                trainer_->Add<int32_t>(kWordTopicTable, word_id, topic_id, delta);
            }

            template <typename trainer_type>
            void PSModel<trainer_type>::AddSummaryRow(integer_t topic_id, int64_t delta)
            {
                trainer_->Add<int64_t>(kSummaryRow, 0, topic_id, delta);
            }

        } // namespace dev
    } // namespace lightlda
} // namespace multiverso

#endif //LIGHTLDA_MODELINTERFACEIMPL_HPP
