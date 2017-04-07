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
    struct MapValues{};
    struct MapKeys{};
}


template <typename map_type, typename impl_type = mongoHelper::MapValues>
class tf_map_convertor_impl {
public:
    tf_map_convertor_impl(map_type* inputMap, map_type* compareMap) : _inputMap(inputMap), _compareMap(compareMap)
    {
    }

    template <typename T=impl_type, mongoHelper::enable_if_match<T , mongoHelper::MapValues> = 0>
    void operator()(bsoncxx::builder::stream::array_context<> ac) const
    {
        std::cout << " size in map :      " << _inputMap->size() << "\n";
        std::cout << " size comp map :      " << _compareMap->size() << "\n";

        for (auto i(0); i<_compareMap->size(); i++)
        {
            if(_compareMap->at(i) > 0)
            {
                //std::cout << i << " :      " << _inputMap->at(i) << "\n";
                ac << _inputMap->at(i);
            }
        }
    }

    template <typename T=impl_type, mongoHelper::enable_if_match<T , mongoHelper::MapKeys> = 0>
    void operator()(bsoncxx::builder::stream::array_context<> ac) const
    {
        for (auto i(0); i<_compareMap->size(); i++)
        {
            if(_compareMap->at(i) > 0)
            {
                ac << i;
            }
        }
    }

private:
    map_type* _inputMap;
    map_type* _compareMap;
};



template<typename T, typename U = mongoHelper::MapValues>
auto tf_map_convertor(T* mapin,T* mapcomp) -> decltype(tf_map_convertor_impl<T,U>(mapin,mapcomp))
{
    return tf_map_convertor_impl<T,U>(mapin,mapcomp);
}

typedef std::unordered_map<int32_t ,int32_t > UMap;

template<typename U = mongoHelper::MapValues>
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
#endif //LIGHTLDA_MONGOHELPER_H
