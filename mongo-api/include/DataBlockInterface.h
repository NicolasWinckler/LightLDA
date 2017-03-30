//
// Created by nw on 30.03.17.
//

#ifndef LIGHTLDA_DATABLOCKINTERFACE_H
#define LIGHTLDA_DATABLOCKINTERFACE_H



#include "common.h"

#include <multiverso/multiverso.h>

#include <memory>
#include <string>
#include <vector>

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {

            /*!
            * \brief DataBlock is the an unit of the training dataset,
            *  it correspond to a data block file in disk.
            */
            template<typename DataBlockPolicy, typename meta_type>
            class DataBlockInterface : public DataBlockPolicy
            {
                typedef LocalVocab<meta_type> LocalVocab_t;

            public:
                DataBlockInterface();
                ~DataBlockInterface();
                /*! \brief Reads a block of data into data block from disk */
                template<typename Args...>
                void Read(Args&&... args)
                {
                    DataBlockPolicy::Read(std::forward<Args>(args)...);
                }
                /*! \brief Writes a block of data to disk */
                template<typename Args...>
                void Write(Args&&... args)
                {
                    DataBlockPolicy::Write(std::forward<Args>(args)...);
                }

                bool HasLoad() const;

                /*! \brief Gets the size (number of documents) of data block */
                DocNumber Size() const;
                /*!
                 * \brief Gets one document
                 * \param index index of document
                 * \return pointer to document
                 */
                Document* GetOneDoc(int32_t index);

                // mutator and accessor methods
                const LocalVocab_t& meta() const;
                void set_meta(const LocalVocab_t* local_vocab);
            private:
                void GenerateDocuments();
                bool has_read_;
                /*! \brief size of memory pool for document offset */
                int64_t max_num_document_;
                /*! \brief size of memory pool for documents */
                int64_t memory_block_size_;
                /*! \brief index to each document */
                std::vector<std::shared_ptr<Document>> documents_;
                /*! \brief number of document in this block */
                DocNumber num_document_;
                /*! \brief memory pool to store the document offset */
                int64_t* offset_buffer_;
                /*! \brief actual memory size used */
                int64_t corpus_size_;
                /*! \brief memory pool to store the documents */
                int32_t* documents_buffer_;
                /*! \brief meta(vocabs) information of current data block */
                const LocalVocab_t* vocab_;
                /*! \brief file name in disk */
                std::string file_name_;
                // No copying allowed
                DataBlockInterface(const DataBlockInterface&);
                void operator=(const DataBlockInterface&);
            };

        } // namespace dev
    } // namespace lightlda
} // namespace multiverso
#endif //LIGHTLDA_DATABLOCKINTERFACE_H
