//
// Created by nw on 09.05.17.
//

#ifndef LIGHTLDA_QXLDA_HPP
#define LIGHTLDA_QXLDA_HPP
#include "common.h"
#include "trainer.h"
#include "alias_table.h"
#include "data_stream.h"
#include "data_block.h"
#include "document.h"
#include "meta.h"
#include "util.h"
#include <vector>
#include <iostream>
#include <multiverso/barrier.h>
#include <multiverso/log.h>
#include <multiverso/row.h>

#include <chrono>
#include <ctime>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/pool.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/exception/exception.hpp>

#include <type_traits>

namespace multiverso
{
    namespace lightlda
    {
        class qxlda
        {
        public:
            qxlda();
            ~qxlda();
            void Run(int argc, char** argv);
        private:
            void Train();
            void InitMultiverso();
            void Initialize();
            void DumpDocTopic();
            void DumpDocTopicToMongoDB();
            void DumpDocTopicToFile();
            void CreateTable();
            void ConfigTable();

            /*! \brief training data access */
            IDataStream* m_data_stream;
            /*! \brief training data meta information */
            Meta m_meta;
            bool m_chrono;
        };
    } // namespace lightlda
} // namespace multiverso



#endif //LIGHTLDA_QXLDA_HPP
