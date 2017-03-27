//
// Created by nw on 27.03.17.
//

#ifndef LIGHTLDA_DATASTREAMTYPES_H
#define LIGHTLDA_DATASTREAMTYPES_H

#include <type_traits>

#include "DataStreamImpl.h"
#include "DataStreamInterface.h"
#include "common.h"
#include "data_block.h"

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /*! \brief template alias and type definition of data stream */

            namespace mld = multiverso::lightlda::dev;
            template <typename block_type>
            using DataStream_disk = mld::DataStreamInterface<mld::DiskDataStream_impl<block_type> >;

            template <typename block_type>
            using DataStream_memory = mld::DataStreamInterface<mld::MemoryDataStream_impl<block_type> >;

            typedef DataStream_disk<DataBlock> DefaultDataStream_disk;
            typedef DataStream_memory<DataBlock> DefaultDataStream_memory;

            /*! \brief helper template alias for the CreateDataStream factory function */
            template<typename T, typename U>
            using enable_if_match = typename std::enable_if<std::is_same<T,U>::value,int>::type;

            /*! \brief compile-time version of the CreateDataStream factory function */
            template<typename datastream_type, enable_if_match<datastream_type, DefaultDataStream_memory> = 0>
            datastream_type* CreateDataStream()
            {
                return new datastream_type(Config::num_blocks, Config::input_dir);
            }

            template<typename datastream_type, enable_if_match<datastream_type, DefaultDataStream_disk> = 0>
            datastream_type* CreateDataStream()
            {
                return new datastream_type(Config::num_blocks, Config::input_dir, Config::num_iterations);
            }

        } // namespace dev
    } // namespace lightlda
} // namespace multiverso


#endif //LIGHTLDA_DATASTREAMTYPES_H
