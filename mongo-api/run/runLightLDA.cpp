//
// Created by nw on 27.03.17.
//


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


#include "DataStreamTypes.h"
#include "basic_LightLDA.h"
#include "DataStreamInterface.h"



namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            int Run(int argc, char** argv)
            {
                multiverso::lightlda::Config::Init(argc, argv);
                if(!Config::db_mode)
                {
                    if (Config::out_of_core && Config::num_blocks != 1)
                    {
                        basic_LightLDA<DefaultDataStream_disk>::Run(argc, argv);
                    }
                    else
                    {
                        basic_LightLDA<DefaultDataStream_memory>::Run(argc, argv);
                    }
                }
                else
                {

                }
                return 0;
            }
        } // namespace dev
    } // namespace lightlda
} // namespace multiverso


int main(int argc, char** argv)
{
    multiverso::lightlda::dev::Run(argc, argv);
    std::cout << "Multiverso completed" << std::endl;

    return 0;
}

