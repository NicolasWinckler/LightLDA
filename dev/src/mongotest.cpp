//
// Created by nw on 07.04.17.
//


#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "InitMongoDB.h"
#include "MongoHelper.h"




struct Token
{
    int32_t word_id;
    int32_t topic_id;
};


int test1();
int test2();
int test3();




typedef std::unique_ptr<mongocxx::pool> MongoPool_ptr;

class mydbtest
{
    
std::string MongoDBName_;
            std::string MongoCollectionName_;
            std::string MongoUri_;
            int32_t doc_buf_idx_;
            bool has_read_;
            const int32_t kMaxDocLength = 10000000;
            std::unique_ptr<mongocxx::pool> ClientToTrainingData_;

public:
    mydbtest()
    {

    }
    virtual ~mydbtest()
    {}
    void CheckDBParameters()
            {
                // some checks...
                if(MongoUri_.empty())
                    std::cout<<"[DataBlock] Mongo uri is not defined, program will now exit \n";

                if(MongoDBName_.empty())
                    std::cout<<"[DataBlock] Mongo DataBase Name is not defined, program will now exit \n";

                if(MongoCollectionName_.empty())
                    std::cout<<"[DataBlock] Mongo Collection Name is not defined, program will now exit \n";

                if(!ClientToTrainingData_)
                    std::cout<<"[DataBlock] Mongo pool is not not valid, program will now exit \n";
            }
            void SetMongoParameters(const std::string& uri, const std::string& DBName, const std::string& collectionName)
            {
                MongoUri_ = uri;
                MongoDBName_ = DBName;
                MongoCollectionName_ = collectionName;

                ClientToTrainingData_ = std::move(MongoPool_ptr(new mongocxx::pool(mongocxx::uri{uri})));
            }
void ReadTrainingData(int32_t block_idx, int64_t docId)
            {
                CheckDBParameters();

                auto conn = ClientToTrainingData_->acquire();
                auto trainingDataCollection = (*conn)[MongoDBName_][MongoCollectionName_];


                // filter
                auto filter = bsoncxx::builder::stream::document{}
                        << "block_idx" << block_idx
                        << "docId" << docId
                        << bsoncxx::builder::stream::finalize;
                mongocxx::options::find opts{};
                opts.no_cursor_timeout(true);

                std::cout << "trainingDataCollection size = " << trainingDataCollection.count({filter.view()});

                auto trainingCursor = trainingDataCollection.find(filter.view(), opts);

                std::cout << "-----------------> OK 1" << std::endl;
                for (auto &&doc : trainingCursor)
                {
                    bsoncxx::document::element ele_blockid;
                    ele_blockid = doc["block_idx"];
                    std::cout << "-----------------> ele_blockid.get_int32(); " 
                    << ele_blockid.get_int32() << std::endl;
                    ele_blockid = doc["docId"];
                    std::cout << "-----------------> ele_blockid.get_int64(); " 
                    << ele_blockid.get_int64() << std::endl;


                    bsoncxx::document::element ele_id;
                    std::cout << "-----------------> OK 2" << std::endl;
                    
                    if ((ele_id = doc["tokenIds"]) && ele_id.type() == type::k_array)
                    {
                        bsoncxx::array::view observedWordArray{ele_id.get_array().value};
                        // loop over visited items (array)

                        int doc_token_count = 0;
                        std::vector<Token> doc_tokens;

                        std::cout << "-----------------> OK 3" << std::endl;
                        //bsoncxx::document::view tokenView;
                        for(const auto& observedWord : observedWordArray)
                        {
                            if(observedWord.type() == bsoncxx::type::k_document)
                            {
                            //if (doc_token_count >= kMaxDocLength) break;


                            bsoncxx::document::view tokenView = observedWord.get_document().view();
                            bsoncxx::document::view subdoc = observedWord.get_document().value;

                            bsoncxx::document::element word_ele = subdoc["wordId"];//{tokenView["wordId"]};
                            bsoncxx::document::element topic_ele = subdoc["topicId"];//{tokenView["topicId"]};

                            int32_t wordId = -1;
                            int32_t topicId = -1;

std::cout << "-----------------> OK 3-a" << std::endl;

                            if (word_ele)
std::cout << "-----------------> if (word_ele)" << std::endl;
std::cout << "-----------------> OK 3-b" << std::endl;


                            if ( word_ele.type() == bsoncxx::type::k_int32)
std::cout << "-----------------> if ( word_ele.type() == bsoncxx::type::k_int32)" << std::endl;
std::cout << "-----------------> OK 3-c" << std::endl;


std::cout << "-----------------> OK 4-a" << std::endl;
                            if (word_ele && word_ele.type() == bsoncxx::type::k_int32)
                            {

                                wordId = word_ele.get_int32();
                            }
std::cout << "-----------------> OK 4-b" << std::endl;

                            if (topic_ele && topic_ele.type() == bsoncxx::type::k_int32)
                                topicId = topic_ele.get_int32();
std::cout << "-----------------> OK 4-c" << std::endl;

                            if(wordId > 0 && topicId > 0)
                                doc_tokens.push_back({ wordId, topicId });
                            //else
                            //    Log::Fatal("[ERROR] word_id or/and topic_id not found in MongoDB, and is/are required for the LightLDA document buffer.");

std::cout << "-----------------> OK 5 wordid=" << wordId << std::endl;
                            }
                        }// end loop over words

                    }
                }// end loop over doc
}





};


#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/exception/exception.hpp>


int main(int argc, char* argv[])
{
    try{
    test3();
    }
    catch(mongocxx::exception& e)
    {
        std::cout << "[mongocxx::exception] exception caught: " << e.what();
    }
    catch(bsoncxx::exception& e)
    {
        std::cout << "[bsoncxx::exception] exception caught: " << e.what();
    }
    catch(std::exception& e) {
        std::cout << "[std::exception] exception caught: " << e.what();
    }
}



int test3()
{
    std::string uri("mongodb://localhost:27017");
    mydbtest db;
    db.SetMongoParameters(uri,"test2","trainingDataCollection");
    db.ReadTrainingData(1,0);
}

int test2()
{
    std::string uri("mongodb://localhost:27017");
    //int32_t block_idx=0;

    

    InitMongoDB database;
    database.SetVocabDBParameters(uri,"test","vocabCollection");

    database.SetTrainingDataDBParameters(uri,"test2","trainingDataCollection");
    //const int32_t kMaxDocLength = 8192;

    int offset = 0;
    for(int64_t docid = 0;docid<3;docid++)
    {
        std::vector<Token> doc_tokens;

        for(int i=5;i<8;i++)
        {
            doc_tokens.push_back({i+offset,1});

        }
        database.WriteTrainingData(1, docid, doc_tokens);
        offset+=100;
    }


    return 0;
}

int test1() {

    using builder::stream::document;
    using builder::stream::array;


    std::string mongoURI = "mongodb://127.0.0.1";
    mongocxx::instance instance{}; // This should be done only once.
    mongocxx::client client{mongocxx::uri{mongoURI}};
    mongocxx::database db = client["mydb"];
    mongocxx::collection coll = db["test"];


    // bsoncxx::builder::stream presents an iostream like interface for succinctly
    // constructing complex BSON objects.  It also allows for interesting
    // primitives by writing your own callables with interesting signatures.

    // stream::document builds a BSON document
    auto doc = document{};

    // stream::array builds a BSON array
    auto arr = array{};

    // Some key value pairs we'd like to append
    std::map<std::string, int> some_kvps = {{"a", 6},
                                            {"b", 6},
                                            {"c", 6}};

    // Some values we'd like to append;
    std::vector<Token> doc_tokens;

    std::vector<int> some_numbers = {1, 2, 3};


    std::map<int32_t ,int32_t > local_tf = {{0,1},{1,0},{2,1},{3,6},{4,5},{5,1}};
    std::map<int32_t ,int32_t > global_tf = {{0,1},{1,213},{2,1},{3,3},{4,5},{5,0}};


    // Adapt our kvps
    doc << make_range_kvp_appender(some_kvps.begin(), some_kvps.end());
    // Now doc = {
    //     "a" : 1,
    //     "b" : 2,
    //     "c" : 3
    // }

    // Adapt our values
    arr << make_mapval_to_array(some_kvps);
    //arr << make_range_array_appender(some_numbers.begin(), some_numbers.end());
    // Now arr = {
    //     "0" : 1,
    //     "1" : 2,
    //     "2" : 3
    // }

    auto localtf_arr = array{};
    auto globaltf_arr = array{};
    auto id_arr = array{};


    localtf_arr << tf_map_convertor<mongoHelper::LocalMap>(&local_tf,&local_tf);

    globaltf_arr << tf_map_convertor<mongoHelper::GlobalMap>(&global_tf,&local_tf);
    id_arr << tf_map_convertor<mongoHelper::Index>(&local_tf,&local_tf);
    int32_t block_idx=0;

    bsoncxx::builder::stream::document tempdoc{};

    auto finalDoc = bsoncxx::builder::stream::document{}  << "$set" << bsoncxx::builder::stream::open_document
                             << "block_idx" << block_idx
                            /* open vocab obj */
                             << "vocab" << bsoncxx::builder::stream::open_document
                             << bsoncxx::builder::concatenate(doc.view())
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
                             /*local_tf array */
                                << "local_tf"
                                << bsoncxx::builder::stream::open_array
                                << bsoncxx::builder::concatenate(arr.view())
                                << bsoncxx::builder::stream::close_array
                             /* close vocab obj */
                             << bsoncxx::builder::stream::close_document
                             /* close $set */
                             << bsoncxx::builder::stream::close_document
                             /* gives a view */
                             << bsoncxx::builder::stream::finalize;


    mongocxx::options::update updateOpts;
    updateOpts.upsert(true);
    auto filter = bsoncxx::builder::stream::document{};
    coll.update_one(filter.view(), std::move(finalDoc), updateOpts);
    //*/


    return 0;
}


