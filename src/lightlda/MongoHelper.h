//
// Created by nw on 07.04.17.
//

#ifndef LIGHTLDA_MONGOHELPER_H
#define LIGHTLDA_MONGOHELPER_H

#include <map>
#include <vector>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/json.hpp>

//---------------------------------

#include <cstdint>
#include <iostream>
#include <vector>

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/types.hpp>


#include <unordered_map>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using namespace bsoncxx;


namespace multiverso { namespace lightlda
    {
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
            void SetMongoParameters(const std::string& uri,
                                    const std::string& dbName,
                                    const std::string& collection)
            {
                _mongo_uri=uri;
                _mongo_db=dbName;
                _mongo_collection=collection;
            }

            virtual ~MongoConfig() {}
        protected:

            std::string _mongo_uri;
            std::string _mongo_db;
            std::string _mongo_collection;

        };


    } // namespace lightlda
} // namespace multiverso

// concatenates arbitrary ranges into an array context
template <typename begin_t, typename end_t>
class range_array_appender
{
public:
    range_array_appender(begin_t begin, end_t end) : _begin(begin), _end(end)
    {
    }

    void operator()(bsoncxx::builder::stream::array_context<> ac) const
    {
        for (auto iter = _begin; iter != _end; ++iter)
        {
            ac << *iter;
        }
    }

private:
    begin_t _begin;
    end_t _end;
};

template <typename begin_t, typename end_t>
range_array_appender<begin_t, end_t> make_range_array_appender(begin_t&& begin, end_t&& end)
{
    return range_array_appender<begin_t, end_t>(std::forward<begin_t>(begin), std::forward<end_t>(end));
}



/* ----------------------------------------------------------------------------------- */
// concatenates arbitrary ranges into an key context
template <typename begin_t, typename end_t>
class range_kvp_appender {
public:
    range_kvp_appender(begin_t begin, end_t end) : _begin(begin), _end(end)
    {
    }

    void operator()(bsoncxx::builder::stream::key_context<> ac) const
    {
        for (auto iter = _begin; iter != _end; ++iter)
        {
            ac << iter->first << iter->second;
        }
    }

private:
    begin_t _begin;
    end_t _end;
};

template <typename begin_t, typename end_t>
range_kvp_appender<begin_t, end_t> make_range_kvp_appender(begin_t&& begin, end_t&& end)
{
    return range_kvp_appender<begin_t, end_t>(std::forward<begin_t>(begin), std::forward<end_t>(end));
}


/* ----------------------------------------------------------------------------------- */
// map values to bsoncxx array

template <typename begin_t, typename end_t>
class range_mapval_to_array_appender {
public:
    range_mapval_to_array_appender(begin_t begin, end_t end) : _begin(begin), _end(end)
    {
    }

    void operator()(bsoncxx::builder::stream::array_context<> ac) const
    {
        for (auto iter = _begin; iter != _end; ++iter)
        {
            ac << iter->second;
        }
    }

private:
    begin_t _begin;
    end_t _end;
};




template <typename begin_t, typename end_t>
range_mapval_to_array_appender<begin_t, end_t> make_range_mapval_to_array_appender(begin_t&& begin, end_t&& end)
{
    return range_mapval_to_array_appender<begin_t, end_t>(std::forward<begin_t>(begin), std::forward<end_t>(end));
}


template<typename T>
auto make_mapval_to_array(T&& map) -> decltype(make_range_mapval_to_array_appender(map.begin(),map.end()))
{
    return make_range_mapval_to_array_appender(map.begin(),map.end());
}



/* ----------------------------------------------------------------------------------- */
// map values to bsoncxx array
namespace mongoHelper
{
    template<typename T, typename U>
    using enable_if_match = typename std::enable_if<std::is_same<T,U>::value,int>::type;
    struct LocalMap{};
    struct Index{};
    struct GlobalMap{};
}


template <typename map_type, typename impl_type = mongoHelper::LocalMap>
class tf_map_convertor_impl {
public:
    tf_map_convertor_impl(map_type* local, map_type* global) : _localMap(local), _globalMap(global)
    {
    }

    template <typename T=impl_type, mongoHelper::enable_if_match<T , mongoHelper::LocalMap> = 0>
    void operator()(bsoncxx::builder::stream::array_context<> ac) const
    {
        std::cout << " size in map :      " << _localMap->size() << "\n";
        std::cout << " size comp map :      " << _globalMap->size() << "\n";

        for(const auto& p : *_globalMap)
        {
            if(_localMap->count(p.first))
                if(_localMap->at(p.second > 0))
                {
                    //std::cout << i << " :      " << _localMap->at(i) << "\n";
                    ac << _localMap->at(p.first);
                }
        }
    }

    template <typename T=impl_type, mongoHelper::enable_if_match<T , mongoHelper::GlobalMap> = 0>
    void operator()(bsoncxx::builder::stream::array_context<> ac) const
    {
        //std::cout << " size in map :      " << _localMap->size() << "\n";
        //std::cout << " size comp map :      " << _globalMap->size() << "\n";

        for(const auto& p : *_globalMap)
        {
            if(_localMap->count(p.first))
                if(_localMap->at(p.second > 0))
                {
                    //std::cout << i << " :      " << _localMap->at(i) << "\n";
                    ac << _globalMap->at(p.first);
                }
        }
    }

    template <typename T=impl_type, mongoHelper::enable_if_match<T , mongoHelper::Index> = 0>
    void operator()(bsoncxx::builder::stream::array_context<> ac) const
    {
        for(const auto& p : *_globalMap)
        {
            if(_localMap->count(p.first))
                if(_localMap->at(p.second > 0))
                {
                    //std::cout << i << " :      " << _localMap->at(i) << "\n";
                    ac << p.first;
                }
        }
    }

private:
    map_type* _localMap;
    map_type* _globalMap;
};



template<typename T, typename U = mongoHelper::LocalMap>
auto tf_map_convertor(T* mapin,T* mapcomp) -> decltype(tf_map_convertor_impl<T,U>(mapin,mapcomp))
{
    return tf_map_convertor_impl<T,U>(mapin,mapcomp);
}

typedef std::map<int32_t ,int32_t > UMap;

template<typename U = mongoHelper::LocalMap>
auto tf_map_convertor(UMap* mapin,UMap* mapcomp) -> decltype(tf_map_convertor_impl<UMap,U>(mapin,mapcomp))
{
    return tf_map_convertor_impl<UMap,U>(mapin,mapcomp);
}



/*
template <typename T, typename U>
auto DurationCast(U const& u) -> decltype(std::chrono::duration_cast<T>(u))
{
    return std::chrono::duration_cast<T>(u);
}

template<typename T>
using tf_map_convertor_t = typename tf_map_convertor<std::unordered_map<int32_t ,int32_t >,T>;
*/






template <typename doc_tokens>
class doctokens_convertor
{
public:
    doctokens_convertor(doc_tokens* tokens) : _tokens(tokens)
    {
    }

    void operator()(bsoncxx::builder::stream::array_context<> ac) const
    {
        for (const auto& token : *_tokens)
        {
            ac << bsoncxx::builder::stream::open_document
            << "wordId" << token.word_id
            << "topicId" << token.topic_id
            << bsoncxx::builder::stream::close_document;
        }
    }

private:
    doc_tokens* _tokens;
};

template <typename doc_tokens>
doctokens_convertor<doc_tokens> make_doctokens_convertor(doc_tokens* tokens)
{
    return doctokens_convertor<doc_tokens>(tokens);
}






#endif //LIGHTLDA_MONGOHELPER_H
