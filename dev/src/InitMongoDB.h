//
// Created by nw on 06.04.17.
//

#ifndef LIGHTLDA_INITMONGODB_H
#define LIGHTLDA_INITMONGODB_H

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>


//

#include <memory>

#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/pool.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include "MongoHelper.h"

struct Token;
class InitMongoDB
{
    typedef std::unique_ptr<mongocxx::pool> MongoPool_ptr;
public:
    InitMongoDB() :
            TrainingDataDBName_("mydb"),
            TrainingDataCollectionName_(),
            TrainingDataMongoUri_(),
            VocabDBName_("mydb"),
            VocabCollectionName_(),
            VocabMongoUri_(),
            kMaxDocLength(8192),
            kMaxVocab(1000000),
            ClientToTrainingData_(nullptr),
            ClientToVocab_(nullptr)
    {}

    virtual ~InitMongoDB()
    {}

    void SetTrainingDataDBParameters(const std::string& uri, const std::string& DBName, const std::string& collectionName)
    {
        TrainingDataMongoUri_ = uri;
        TrainingDataDBName_ = DBName;
        TrainingDataCollectionName_ = collectionName;

        ClientToTrainingData_ = std::move(MongoPool_ptr(new mongocxx::pool(mongocxx::uri{TrainingDataMongoUri_})));
    }

    void SetVocabDBParameters(const std::string& uri, const std::string& DBName, const std::string& collectionName)
    {
        VocabMongoUri_ = uri;
        VocabDBName_ = DBName;
        VocabCollectionName_ = collectionName;

        ClientToVocab_ = std::move(MongoPool_ptr(new mongocxx::pool(mongocxx::uri{VocabMongoUri_})));
    }

    void WriteTrainingData(int64_t docId, std::vector<Token>& doc_tokens)
    {
        auto conn = ClientToTrainingData_->acquire();
        //auto recProcCollection = (*conn)[s_RecProcDB][s_RecProcTrainCollection];
        auto trainingDataCollection = (*conn)[TrainingDataDBName_][TrainingDataCollectionName_];

        // upsert options
        mongocxx::options::update updateOpts;
        updateOpts.upsert(true);

        // filter
        auto filter = bsoncxx::builder::stream::document{}
                << "docId" << docId
                << bsoncxx::builder::stream::finalize;


        auto token_array = bsoncxx::builder::stream::array{};
        token_array << make_doctokens_convertor(&doc_tokens);
        //open doc
        bsoncxx::builder::stream::document doc{};

        // code below ok when creating the collection for the first time,
        doc << "$push"  << bsoncxx::builder::stream::open_document
            << "tokenIds" << bsoncxx::builder::stream::open_document
            << "$each"  << bsoncxx::builder::stream::open_array
            << bsoncxx::builder::concatenate(token_array.view())
            << bsoncxx::builder::stream::close_array << "$slice" << -kMaxDocLength
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::close_document;

        // update
        bsoncxx::document::value fUpdate = doc << bsoncxx::builder::stream::finalize;
        trainingDataCollection.update_one(filter.view(), std::move(fUpdate), updateOpts);
        //LOG(DEBUG) << "upserted user " << userId;

    }

    void WriteTrainingDataTemp(int64_t docId, int32_t wordId, int32_t topicId)
    {
        auto conn = ClientToTrainingData_->acquire();
        //auto recProcCollection = (*conn)[s_RecProcDB][s_RecProcTrainCollection];
        auto trainingDataCollection = (*conn)[TrainingDataDBName_][TrainingDataCollectionName_];

        // upsert options
        mongocxx::options::update updateOpts;
        updateOpts.upsert(true);

        // filter
        auto filter = bsoncxx::builder::stream::document{}
                << "docId" << docId
                << bsoncxx::builder::stream::finalize;

        //open doc
        bsoncxx::builder::stream::document doc{};

        // code below ok when creating the collection for the first time,
        doc << "$push"  << bsoncxx::builder::stream::open_document
            << "tokenIds" << bsoncxx::builder::stream::open_document
            << "$each"  << bsoncxx::builder::stream::open_array
            << bsoncxx::builder::stream::open_document
            << "wordId" << wordId
            << "topicId" << topicId
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::close_array << "$slice" << -kMaxDocLength
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::close_document;

        // update
        bsoncxx::document::value fUpdate = doc << bsoncxx::builder::stream::finalize;
        trainingDataCollection.update_one(filter.view(), std::move(fUpdate), updateOpts);
        //LOG(DEBUG) << "upserted user " << userId;

    }

    void WriteVocabTemp(int32_t block_idx, int32_t wordId, int32_t global_tf, int32_t local_tf)
    {

        auto conn = ClientToVocab_->acquire();
        auto vocabCollection = (*conn)[VocabDBName_][VocabCollectionName_];

        // upsert options
        mongocxx::options::update updateOpts;
        updateOpts.upsert(true);

        // filter
        auto filter = bsoncxx::builder::stream::document{}
                << "block_idx" << block_idx
                << bsoncxx::builder::stream::finalize;

        //open doc
        bsoncxx::builder::stream::document doc{};

        // code below ok when creating the collection for the first time,
        doc << "block_idx" << block_idx
            << "vocab" << bsoncxx::builder::stream::open_document
                << "$push"  << bsoncxx::builder::stream::open_document
                    << "id" << bsoncxx::builder::stream::open_document
                        << "$each"  << bsoncxx::builder::stream::open_array
                            <<  wordId
                        << bsoncxx::builder::stream::close_array << "$slice" << -kMaxVocab
                    << bsoncxx::builder::stream::close_document
                << bsoncxx::builder::stream::close_document;


        doc << "$push"  << bsoncxx::builder::stream::open_document
            << "global_tf" << bsoncxx::builder::stream::open_document
            << "$each"  << bsoncxx::builder::stream::open_array
            << global_tf
            << bsoncxx::builder::stream::close_array << "$slice" << -kMaxVocab
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::close_document;

        doc << "$push"  << bsoncxx::builder::stream::open_document
            << "local_tf" << bsoncxx::builder::stream::open_document
            << "$each"  << bsoncxx::builder::stream::open_array
            << local_tf
            << bsoncxx::builder::stream::close_array << "$slice" << -kMaxVocab
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::close_document;

        // update
        bsoncxx::document::value fUpdate = doc << bsoncxx::builder::stream::finalize;
        vocabCollection.update_one(filter.view(), std::move(fUpdate), updateOpts);
        //LOG(DEBUG) << "upserted user " << userId;

    }



    void WriteVocab(int32_t block_idx,
                    std::unordered_map<int32_t ,int32_t >& global_tf,
                    std::unordered_map<int32_t ,int32_t >& local_tf)
    {

        std::cout << "[in obj] ok1\n";
        auto conn = ClientToVocab_->acquire();
        auto vocabCollection = (*conn)[VocabDBName_][VocabCollectionName_];

        // upsert options
        mongocxx::options::update updateOpts;
        updateOpts.upsert(true);

        // filter
        auto filter = bsoncxx::builder::stream::document{}
                << "block_idx" << block_idx
                << bsoncxx::builder::stream::finalize;


        auto localtf_arr = builder::stream::array{};
        auto globaltf_arr = builder::stream::array{};
        auto id_arr = builder::stream::array{};



        localtf_arr         << tf_map_convertor<mongoHelper::LocalMap>(&local_tf,&global_tf);

        globaltf_arr << tf_map_convertor<mongoHelper::GlobalMap>(&local_tf,&global_tf);

        id_arr << tf_map_convertor<mongoHelper::Index>(&local_tf,&global_tf);

        //////

        bsoncxx::builder::stream::document tempdoc{};

        auto finalDoc = tempdoc << "$set" << bsoncxx::builder::stream::open_document
                                << "block_idx" << block_idx
                                /* open vocab obj */
                                << "vocab" << bsoncxx::builder::stream::open_document
                                //<< bsoncxx::builder::concatenate(doc.view())
                                /* id array */
                                << "id"
                                << bsoncxx::builder::stream::open_array
                                << bsoncxx::builder::concatenate(id_arr.view())
                                << bsoncxx::builder::stream::close_array
                                /*global_tf array */
                                << "global_tf"
                                << bsoncxx::builder::stream::open_array
                                << bsoncxx::builder::concatenate(globaltf_arr.view())
                                << bsoncxx::builder::stream::close_array
                                /*local_tf array */
                                << "local_tf"
                                << bsoncxx::builder::stream::open_array
                                << bsoncxx::builder::concatenate(localtf_arr.view())
                                << bsoncxx::builder::stream::close_array
                                /* close vocab obj */
                                << bsoncxx::builder::stream::close_document
                                /* close $set */
                                << bsoncxx::builder::stream::close_document
                                /* gives a view */
                                << bsoncxx::builder::stream::finalize;

        vocabCollection.update_one(filter.view(), std::move(finalDoc), updateOpts);

    }



private:

    std::string TrainingDataDBName_;
    std::string TrainingDataCollectionName_;
    std::string TrainingDataMongoUri_;

    std::string VocabDBName_;
    std::string VocabCollectionName_;
    std::string VocabMongoUri_;


    const int32_t kMaxDocLength;
    const int32_t kMaxVocab;
    std::unique_ptr<mongocxx::pool> ClientToTrainingData_;
    std::unique_ptr<mongocxx::pool> ClientToVocab_;
};



#endif //LIGHTLDA_INITMONGODB_H
