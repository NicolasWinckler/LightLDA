/*!
 * \file util.h
 * \brief Defines random number generator
 */

#ifndef LIGHTLDA_UTIL_H_
#define LIGHTLDA_UTIL_H_

#include <ctime>
#include <type_traits>
namespace multiverso { namespace lightlda
{

    /*! \brief seed for production */
    class RNG_PROD
    {
    public:
        RNG_PROD()
        {
            jxr_ = static_cast<unsigned int>(time(nullptr));
        }
        ~RNG_PROD(){}
    protected:
        /*! \brief seed */
        uint32_t jxr_;   
    };


    /*! \brief seed for debug mode */
    class RNG_DEBUG
    {
    public:
        RNG_DEBUG()
        {
            jxr_ = 1;
        }
        ~RNG_DEBUG(){}
    protected:
        /*! \brief seed */
        uint32_t jxr_;   
    };



    template<typename MODE=RNG_PROD>
    class xorshift_rng_base : public MODE
    {
        using MODE::jxr_;
    public:
        xorshift_rng_base() : MODE()
        {
            
        }
        ~xorshift_rng_base() {}

        /*! \brief get random (xorshift) 32-bit integer*/
        int32_t rand()
        {
            jxr_ ^= (jxr_ << 13); jxr_ ^= (jxr_ >> 17); jxr_ ^= (jxr_ << 5);
            return jxr_ & 0x7fffffff;
        }

        double rand_double()
        {
            return rand() * 4.6566125e-10;
        }
        int32_t rand_k(int K)
        {
            return static_cast<int>(rand() * 4.6566125e-10 * K);
        }
    private:
        // No copying allowed
        xorshift_rng_base(const xorshift_rng_base &other);
        void operator=(const xorshift_rng_base &other);
        
    };


    //typedef xorshift_rng_base<RNG_PROD> xorshift_rng;
    //typedef xorshift_rng_base<RNG_DEBUG> xorshift_rng;


    struct xorshift_rng : public xorshift_rng_base<RNG_DEBUG>
    {};




    /*! \brief xorshift_rng is a random number generator */
    // class xorshift_rng
    // {
    // public:
    //     xorshift_rng()
    //     {
    //         jxr_ = static_cast<unsigned int>(time(nullptr));
    //     }
    //     ~xorshift_rng() {}

    //     /*! \brief get random (xorshift) 32-bit integer*/
    //     int32_t rand()
    //     {
    //         jxr_ ^= (jxr_ << 13); jxr_ ^= (jxr_ >> 17); jxr_ ^= (jxr_ << 5);
    //         return jxr_ & 0x7fffffff;
    //     }

    //     double rand_double()
    //     {
    //         return rand() * 4.6566125e-10;
    //     }
    //     int32_t rand_k(int K)
    //     {
    //         return static_cast<int>(rand() * 4.6566125e-10 * K);
    //     }
    // private:
    //     // No copying allowed
    //     xorshift_rng(const xorshift_rng &other);
    //     void operator=(const xorshift_rng &other);
        
    // };
} // namespace lightlda
} // namespace multiverso

#endif // LIGHTLDA_UTIL_H_
