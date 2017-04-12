//
// Created by nw on 31.03.17.
//

#ifndef LIGHTLDA_DATABLOCKINTERFACE_H
#define LIGHTLDA_DATABLOCKINTERFACE_H
#include "common.h"
#include "document.h"
#include <multiverso/multiverso.h>
#include "Vocab.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <utility>
namespace multiverso
{
    namespace lightlda
    {
        template< typename IOPolicy>
        class DataBlockInterface :  public IOPolicy
        {
            typedef DataBlockInterface<IOPolicy> self_type;
            friend IOPolicy;
        public:
            DataBlockInterface();
            virtual ~DataBlockInterface();

            /*! \brief Gets the size (number of documents) of data block */
            DocNumber Size() const;
            /*!
             * \brief Gets one document
             * \param index index of document
             * \return pointer to document
             */
            Document* GetOneDoc(int32_t index);

            // mutator and accessor methods
            const LocalVocab& meta() const;
            void set_meta(const LocalVocab* local_vocab);

            // TODO find solution for the "error: expected nested-name-specifier before ‘Args’"
            /*! \brief Reads a block of data into data block from disk */
//            template<typename Args...>
//            void Read(Args&&... args)
//            {
//                IOPolicy::Read(std::forward<Args>(args)...);
//            }
//            /*! \brief Writes a block of data to disk */
//            template<typename Args...>
//            void Write(Args&&... args)
//            {
//                IOPolicy::Write(std::forward<Args>(args)...);
//            }

        private:
            void GenerateDocuments();
            //bool has_read_;
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
            const LocalVocab* vocab_;
            /*! \brief file name in disk */
            //std::string file_name_;
            // No copying allowed
            DataBlockInterface(const DataBlockInterface&);
            void operator=(const DataBlockInterface&);
        };
    } // namespace lightlda
} // namespace multiverso

#include "DataBlockInterfaceImpl.h"
#endif //LIGHTLDA_DATABLOCKINTERFACE_H
