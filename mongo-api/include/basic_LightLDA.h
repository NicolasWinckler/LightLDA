//
// Created by nw on 27.03.17.
//

#ifndef LIGHTLDA_BASIC_LIGHTLDA_H
#define LIGHTLDA_BASIC_LIGHTLDA_H


#include "common.h"
#include "trainer.h"
#include "alias_table.h"
//#include "data_stream.h"
#include "data_block.h"
#include "document.h"
#include "meta.h"
#include "util.h"
#include <vector>
#include <iostream>
#include <multiverso/barrier.h>
#include <multiverso/log.h>
#include <multiverso/row.h>
#include <string>
#include <cstring>
#include <limits.h>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <type_traits>

//#include "DataStreamTypes.h"


namespace multiverso
{
    namespace lightlda
    {

        namespace dev
        {
            //template<typename datastream_type>
            //datastream_type* CreateDataStream();
            template<typename datastream_type, typename U>
            datastream_type* CreateDataStream();

            template<typename datastream_type>
            class basic_LightLDA
            {
            public:
                static void Run(int argc, char** argv);
            private:
                static void Train();

                static void InitMultiverso();

                static void Initialize();

                static void DumpDocTopic(const std::string& outputDir="");

                static void CreateTable();

                static void ConfigTable();
            private:
                /*! \brief training data access */
                static datastream_type* data_stream;
                /*! \brief training data meta information */
                static Meta meta;
            };
        } // namespace dev
    } // namespace lightlda
} // namespace multiverso

#include "basic_LightLDAImpl.h"
#endif //LIGHTLDA_BASIC_LIGHTLDA_H
