// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <vector>
#include "xobject.h"

namespace top
{
    namespace base
    {
        //extern/3rd part register specific xhash_t into xcontext to provide different hash function
        class xhashplugin_t : public xobject_t
        {
        protected:
            xhashplugin_t(const uint32_t types); //combine every enum_xhash_type
            virtual ~xhashplugin_t();
        private:
            xhashplugin_t();
            xhashplugin_t(const xhashplugin_t &);
            xhashplugin_t & operator = (const xhashplugin_t &);
        public:
            uint32_t      get_types() const {return m_hash_types;} //return all types suppored by this
            virtual const std::string hash(const std::string & input,enum_xhash_type type) = 0;

            virtual void* query_interface(const int32_t type) override;
        private:
            uint32_t   m_hash_types;
        };
        
        //xxh32_t and xxh64_t that is the most fast hash algorithm
        //note: xxh32_t and xxh64_t  get 32/64bit' digest instead of 128/256bits(e.g. sha256)
        /* benchmark
         Comparison (single thread, Windows Seven 32 bits, using SMHasher on a Core 2 Duo @3GHz)
         Name            Speed       Q.Score   Author
         xxHash          5.4 GB/s     10
         CrapWow         3.2 GB/s      2       Andrew
         MumurHash 3a    2.7 GB/s     10       Austin Appleby
         SpookyHash      2.0 GB/s     10       Bob Jenkins
         SBox            1.4 GB/s      9       Bret Mulvey
         Lookup3         1.2 GB/s      9       Bob Jenkins
         SuperFastHash   1.2 GB/s      1       Paul Hsieh
         CityHash64      1.05 GB/s    10       Pike & Alakuijala
         FNV             0.55 GB/s     5       Fowler, Noll, Vo
         CRC32           0.43 GB/s     9
         MD5-32          0.33 GB/s    10       Ronald L. Rivest
         SHA1-32         0.28 GB/s    10
         
         Q.Score is a measure of quality of the hash function.
         It depends on successfully passing SMHasher test set.
         10 is a perfect score.
         
         A 64-bit version, named XXH64, is available since r35.
         It offers much better speed, but for 64-bit applications only.
         Name     Speed on 64 bits    Speed on 32 bits
         XXH64       13.8 GB/s            1.9 GB/s
         XXH32        6.8 GB/s            6.0 GB/s
         */
        class xhash32_t
        {
        public:
            static uint32_t  digest(const void* data, size_t numBytes);
            static uint32_t  digest(const std::string & text);
            
            //note:it is a block operation which take long time ,and  difficulty indicate how many bits need to be 0(from high to low)
            static uint32_t  pow_digest(const uint8_t difficulty,const std::string & _content_data,uint32_t & random_nonce);//return hash and updated  random_nonce
            static uint32_t  pow_digest(const uint8_t difficulty,const uint32_t _content_hash,uint32_t & random_nonce);//return hash and updated  random_nonce
        public:
            xhash32_t();
            ~xhash32_t();
        public:
            int                     get_digest_length() const {return 4;} //return how many bytes of digest
            bool                    reset();/// restart
            int                     update(const std::string & text) ;
            int                     update(const void* data, size_t numBytes);
            bool                    get_hash(std::vector<uint8_t> & raw_bytes); //get raw hash data
            uint32_t                get_hash(); //raw hash as 32bit
        private:
            void* xxhash_state;
        };
        
        class xhash64_t
        {
        public:
            static uint64_t  digest(const void* data, size_t numBytes);
            static uint64_t  digest(const std::string & text);

            //note:it is a block operation which take long time ,and  difficulty indicate how many bits need to be 0(from high to low)
            static uint64_t  pow_digest(uint8_t difficulty,const std::string & _content_data,uint64_t & in_out_random_nonce);//return hash and updated  random_nonce
            static uint64_t  pow_digest(uint8_t difficulty,const uint64_t _content_hash,const uint64_t _random_time,uint64_t & in_out_random_nonce);//return hash and updated  random_nonce
        public:
            xhash64_t();
            ~xhash64_t();
        public:
            int                     get_digest_length() const {return 8;} //return how many bytes of digest
            bool                    reset();/// restart
            int                     update(const std::string & text) ;
            int                     update(const void* data, size_t numBytes) ;
            bool                    get_hash(std::vector<uint8_t> & raw_bytes); //get raw hash data
            uint64_t                get_hash(); //raw data as 64bit
        private:
            void* xxhash_state;
        };
    }
}
