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

#include <chrono>
#include <ctime>

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

            /*! \brief Reads a block of data into data block from disk */

            template <typename... Args>
            void Read(Args&&... args)
            {
                std::chrono::time_point<std::chrono::system_clock> start, end;
                start = std::chrono::system_clock::now();
                IOPolicy::Read(std::forward<Args>(args)...);
                end = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsed_time = end-start;
                std::cout << "time taken for loading training data: " << elapsed_time.count() << "s\n";
            }

            template <typename... Args>
            void Write(Args&&... args)
            {
                std::chrono::time_point<std::chrono::system_clock> start, end;
                start = std::chrono::system_clock::now();
                IOPolicy::Write(std::forward<Args>(args)...);
                end = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsed_time = end-start;
                std::cout << "time taken for writting training data: " << elapsed_time.count() << "s\n";
            }

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
