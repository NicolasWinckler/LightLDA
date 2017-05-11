/*!
 * \file meta.h
 * \brief This file defines meta information for training dataset
 */

#ifndef LIGHTLDA_META_H_
#define LIGHTLDA_META_H_

#include <string>
#include <vector>
#include <cstdint>
#include "Meta_file.h"
#include "MetaMongoDB.h"
#include "Vocab.h"
namespace multiverso { namespace lightlda
{
    //typedef class dev::Meta_file Meta;
    typedef class dev::MetaMongoDB Meta;
    

} // namespace lightlda
} // namespace multiverso

#endif // LIGHTLDA_META_H_
