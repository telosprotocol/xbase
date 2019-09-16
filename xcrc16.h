// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
namespace top
{
    namespace base
    {
        class xcrc16_t
        {
        public:
            static uint16_t crc16(const void* data, size_t numBytes);
            static uint16_t crc16(const std::string & text);
            static std::string crc16_to_string(const uint16_t hash_value);
        public:
            /// same as reset()
            xcrc16_t();
            
            /// add arbitrary number of bytes
            void add(const void* data, size_t numBytes);
            void add(const std::string& text);
            
            /// return latest hash as hex characters
            std::string get_hash_string();
            uint16_t    get_hash_int();
            void        reset(); /// restart
        private:
            uint16_t m_hash;  /// hash
        };
    }
};
