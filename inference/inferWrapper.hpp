//
// Created by nw on 11.05.17.
//

#ifndef LIGHTLDA_INFERWRAPPER_HPP
#define LIGHTLDA_INFERWRAPPER_HPP
//
// Created by nw on 11.05.17.
//

#include "common.h"
#include "alias_table.h"
#include "data_stream.h"
#include "data_block.h"
#include "document.h"
#include "meta.h"
#include "util.h"
#include "model.h"
#include "inferer.h"
#include <vector>
#include <iostream>
#include <thread>
// #include <pthread.h>
#include <multiverso/barrier.h>



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

namespace multiverso
{
    namespace lightlda
    {
        class inferWrapper
        {
        public:
            inferWrapper();
            ~inferWrapper();
            void Run(int argc, char** argv);
        private:
            void Inference(std::vector<Inferer*>& inferers);
            static void* InferenceThread(void* arg);
            void InitDocument();
            void DumpDocTopic();
            void DumpDocTopicToMongoDB();
            void DumpDocTopicToFile();
        private:
            /*! \brief training data access */
            IDataStream* m_data_stream;
            /*! \brief training data meta information */
            Meta m_meta;
        };

    } // namespace lightlda
} // namespace multiverso


#endif //LIGHTLDA_INFERWRAPPER_HPP
