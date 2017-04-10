//
// Created by nw on 07.04.17.
//


#include "MongoHelper.h"

int main(int, char**) {
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
