// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <vector>
namespace top
{
    namespace base
    {
        /// compute MD5 hash
        /** Usage:
         xmd5_t md5;
         std::string myHash  = md5("Hello World");     // std::string
         std::string myHash2 = md5("How are you", 11); // arbitrary data, 11 bytes
         
         // or in a streaming fashion:
         
         xmd5_t md5;
         while (more data available)
         md5.add(pointer to fresh data, number of new bytes);
         std::string myHash3 = md5.getHash();
         */
        //md5 is 128bit' hash
        class xmd5_t
        {
        public:
            /// compute MD5 of a memory block
            static std::string digest(const void* data, size_t numBytes);
            /// compute MD5 of a string, excluding final zero
            static std::string digest(const std::string& text);
        public:
            /// same as reset()
            xmd5_t();

            int                     get_digest_length() const {return 16;} //return how many bytes of digest
            bool                    reset();
            int                     update(const std::string & text) ;
            int                     update(const void* data, size_t numBytes) ;
            bool                    get_hash(std::vector<uint8_t> & raw_bytes) ; //get raw hash data
            std::string             get_hex_hash(); // return latest hash as 16 hex characters
        private:
            /// process 64 bytes
            void processBlock(const void* data);
            /// process everything left in the internal buffer
            void processBuffer();
            
            /// split into 64 byte blocks (=> 512 bits)
            enum { BlockSize = 512 / 8 };
            
            /// size of processed data in bytes
            uint64_t m_numBytes;
            /// valid bytes in m_buffer
            size_t   m_bufferSize;
            /// bytes not processed yet
            uint8_t  m_buffer[BlockSize];
            /// hash, stored as integers
            uint32_t m_hash[4];
        };

    }
};
