// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
namespace top
{
    namespace base
    {
        /// compute CRC32 hash, based on Intel's Slicing-by-8 algorithm
        /** Usage:
         xcrc32_t obj;
         std::string myHash  = obj("Hello World");     // std::string
         std::string myHash2 = obj("How are you", 11); // arbitrary data, 11 bytes
         
         // or in a streaming fashion:
         
         xcrc32_t obj;
         while (more data available)
         obj.add(pointer to fresh data, number of new bytes);
         std::string myHash3 = obj.getHash();
         */
        class xcrc32_t
        {
        public:
            static uint32_t    crc32(const void* data, size_t numBytes);
            static uint32_t    crc32(const std::string& text);
            static std::string crc32_to_string(const uint32_t int_hash);
        public:
            /// same as reset()
            xcrc32_t();
            
            /// add arbitrary number of bytes
            void add(const void* data, size_t numBytes);
            void add(const std::string& text);
            
            /// return latest hash as hex characters
            std::string get_hash_string();
            uint32_t    get_hash_int();
            
            void        reset();  /// restart
        private:
            uint32_t    m_hash; /// hash
        };

    }
};
