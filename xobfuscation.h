// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <vector>
#include "xmem.h"
#include "xpacket.h"

namespace top
{
    namespace base
    {
        //to avoid problem,obfuscation_utl object must be temp  that can not be persisten hold by new/malloc
        class xobfuscation_t
        {
        public://advance function that append random head with random init_iv
            //init_iv must be same value for encode and decode pair
            static   bool     encode(xstream_t & in_output,uint32_t init_iv); //directly append random head at front of in_output and obfuscate
            static   bool     encode(xmemh_t & in_output,uint32_t init_iv); //directly append random head at front of in_output and obfuscate
 
            //after decode, in_output.data() point the original raw data
            //return the size of original raw data, or  return < 0 when fail to decode
            static   int      decode(xstream_t & in_output,uint32_t init_iv);  //directly remove random head at front of in_output and unobfuscate
            static   int      decode(xmemh_t & in_output,uint32_t init_iv); //directly remove random head at front of in_output and unobfuscate
        };
    }
};
