/*!
 * \file data_stream.h
 * \brief Defines interface for data access
 */

#ifndef LIGHTLDA_DATA_STREAM_H_
#define LIGHTLDA_DATA_STREAM_H_

#include "data_block.h"

namespace multiverso { namespace lightlda 
{
    //class DataBlock;
    /*! \brief interface of data stream */
    class IDataStream
    {
    public:
        virtual ~IDataStream() {}
        /*! \brief Should call this method before access a data block */
        virtual void BeforeDataAccess() = 0;
        /*! \brief Should call this method after access a data block */
        virtual void EndDataAccess() = 0;
        /*! 
         * \brief Get one data block 
         * \return reference to data block 
         */
        virtual DataBlock& CurrDataBlock() = 0;
    };

    /*! \brief Factory method to create data stream */
    IDataStream* CreateDataStream();

    class MongoConfig
    {
    public:
        MongoConfig() {}

        MongoConfig(const std::string& uri="mongodb://localhost:27017", 
                    const std::string& dbName="test", 
                    const std::string& collection="trainingDataCollection") : 
                        _mongo_uri(uri), 
                        _mongo_db(dbName), 
                        _mongo_collection(collection)
        {
        }

        virtual ~MongoConfig() {}
    protected:

        std::string _mongo_uri;
        std::string _mongo_db;
        std::string _mongo_collection;

    };
    
} // namespace lightlda
} // namespace multiverso

#endif // LIGHTLDA_DATA_STREAM_H_
