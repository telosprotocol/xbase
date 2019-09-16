// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <memory.h>
#include <functional>
#include "xbase.h"

namespace top
{
    namespace base
    {
        //_predefine_bits_ must be >=64
        template<int _predefine_bits_>
        class xuint_t
        {
        public:
            enum
            {
                enum_xint_size_bits     = _predefine_bits_,
                enum_xint_size_bytes    = _predefine_bits_ / 8,
                enum_xint_size_2bytes   = _predefine_bits_ / 16,
                enum_xint_size_4bytes   = _predefine_bits_ / 32,
                enum_xint_size_8bytes   = _predefine_bits_ / 64,
            };
        public:
            xuint_t()
            {
                memset(raw_uint8, 0, sizeof(raw_uint8));
            }
            xuint_t(const uint64_t init_value)
            {
                raw_uint64[0] = init_value;
            }
            xuint_t(uint8_t int_bytes[enum_xint_size_bytes])
            {
                memcpy(raw_uint8, int_bytes, enum_xint_size_bytes);
            }
            xuint_t(uint64_t int_8bytes[enum_xint_size_8bytes])
            {
                memcpy(raw_uint8, int_8bytes, (enum_xint_size_8bytes << 3));
            }
            xuint_t(const xuint_t & obj)
            {
                memcpy(raw_uint8, obj.raw_uint8, sizeof(raw_uint8));
            }
            xuint_t & operator = (const xuint_t & right)
            {
                memcpy(raw_uint8, right.raw_uint8, sizeof(raw_uint8));
                return *this;
            }
            ~xuint_t(){};
        public:
            inline uint8_t* data() const {return (uint8_t*)raw_uint8;}
            inline int      size() const {return enum_xint_size_bytes;} //how many bytes for length
            
            bool operator < (const xuint_t & right) const
            {
                return (memcmp(raw_uint8, right.raw_uint8, sizeof(raw_uint8)) < 0);
            }
            bool operator <= (const xuint_t & right) const
            {
                return (memcmp(raw_uint8, right.raw_uint8, sizeof(raw_uint8)) <= 0);
            }
            bool operator > (const xuint_t & right) const
            {
                return (memcmp(raw_uint8, right.raw_uint8, sizeof(raw_uint8)) > 0);
            }
            bool operator >= (const xuint_t & right) const
            {
                return (memcmp(raw_uint8, right.raw_uint8, sizeof(raw_uint8)) >= 0);
            }
            bool operator == (const xuint_t & right) const
            {
                return (memcmp(raw_uint8, right.raw_uint8, sizeof(raw_uint8)) == 0);
            }
			bool operator != (const xuint_t & right) const
			{
				return (memcmp(raw_uint8, right.raw_uint8, sizeof(raw_uint8)) != 0);
			}
 
            void bit_set(uint32_t i) {
                if(i < enum_xint_size_bits)
                    raw_uint32[i >> 5] |= (1 << (i & 31));
            }
            void bit_clear(uint32_t i) {
                if(i < enum_xint_size_bits)
                    raw_uint32[i >> 5] &= ~(1 << (i & 31));
            }
            
            bool bit_is_set(uint32_t i) const {
                if(i >= enum_xint_size_bits)
                    return false;
                
                uint32_t value = (raw_uint32[i >> 5] & (1 << (i & 31)));
                return value != 0;
            }
            
            void bit_clear_all() {
                memset(raw_uint8, 0, sizeof(raw_uint8));
            }
            
            void bit_set_all() {
                memset(raw_uint8, 1, sizeof(raw_uint8));
            }
            
            void reset()
            {
                memset(raw_uint8, 0, sizeof(raw_uint8));
            }
            
            bool empty() const
            {
                for(int i = 0; i < enum_xint_size_8bytes; ++i)
                {
                    if(raw_uint64[i] != 0)
                        return false;
                }
                return true;
            }
        public:
            union
            {
                uint8_t   raw_uint8[enum_xint_size_bytes];
                uint16_t  raw_uint16[enum_xint_size_2bytes];
                uint32_t  raw_uint32[enum_xint_size_4bytes];
                uint64_t  raw_uint64[enum_xint_size_8bytes];
            };
            #ifdef DEBUG
            uint8_t  assert_test[enum_xint_size_8bytes - 1];
            #endif
        };
    }

    typedef  top::base::xuint_t<128>   uint128_t;
    typedef  top::base::xuint_t<160>   uint160_t;
    typedef  top::base::xuint_t<256>   uint256_t;
    typedef  top::base::xuint_t<512>   uint512_t;
    
    //GCC 4.7.1+ or Clang support extend 128bit type,so define xuint128_t as special optimization
    //note:128bit has lower performance than 64bit
    //_MSC_VER - Windows Visual Studio Compiler flag
#if defined(__LINUX_PLATFORM__) || defined(__MAC_PLATFORM__)
    typedef __uint128_t                xuint128_t;
#else
    typedef  uint128_t                 xuint128_t;
#endif
}; //end of namespace top



