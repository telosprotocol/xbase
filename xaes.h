// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <vector>
#include "xmem.h"

namespace top
{
    namespace base
    {
        class xaes_t
        {
        public: //AES-CBC Mode,return < 0 if fail,otherwise return the data size of encrypted or decrypted
            static  int  aes_cbc_encrypt_128bit(xmemh_t & in_output,uint8_t aes_key[16],uint8_t aes_iv[16]);
            static  int  aes_cbc_encrypt_128bit(xbuffer_t & in_output,uint8_t aes_key[16],uint8_t aes_iv[16]);
            //data_size must be alignment as 16bytes,otherwise it may return fail
            static  int  aes_cbc_encrypt_128bit(const uint8_t* in_output_data,const int32_t data_size,uint8_t aes_key[16],uint8_t aes_iv[16]);
            
            static  int  aes_cbc_decrypt_128bit(xmemh_t & in_output,uint8_t aes_key[16],uint8_t aes_iv[16]);
            static  int  aes_cbc_decrypt_128bit(xbuffer_t & in_output,uint8_t aes_key[16],uint8_t aes_iv[16]);
            //data_size must be alignment as 16bytes,otherwise it may return fail
            static  int  aes_cbc_decrypt_128bit(const uint8_t* in_output_data,const int32_t data_size,uint8_t aes_key[16],uint8_t aes_iv[16]);
            
            //note:256bit aes still using 128bit block to hold data,so the init iv must be 16 bytes length
            static  int  aes_cbc_encrypt_256bit(xmemh_t & in_output,uint8_t aes_key[32],uint8_t aes_iv[16]);
            static  int  aes_cbc_encrypt_256bit(xbuffer_t & in_output,uint8_t aes_key[32],uint8_t aes_iv[16]);
            //data_size must be alignment as 16bytes,otherwise it may return fail
            static  int  aes_cbc_encrypt_256bit(const uint8_t* in_output_data,const int32_t data_size,uint8_t aes_key[32],uint8_t aes_iv[16]);
            
            static  int  aes_cbc_decrypt_256bit(xmemh_t & in_output,uint8_t aes_key[32],uint8_t aes_iv[16]);
            static  int  aes_cbc_decrypt_256bit(xbuffer_t & in_output,uint8_t aes_key[32],uint8_t aes_iv[16]);
            //data_size must be alignment as 16bytes,otherwise it may return fail
            static  int  aes_cbc_decrypt_256bit(const uint8_t* in_output_data,const int32_t data_size,uint8_t aes_key[32],uint8_t aes_iv[16]);
            
        public://AES-CTR Mode,return < 0 if fail,otherwise return the size of encrypted or decrypted
            //CTR mode has better performance,and very good for streaming data because it dose not need padding
            static  int  aes_ctr_encrypt_128bit(xmemh_t & in_output,uint8_t aes_key[16],uint8_t aes_nounce[16]);
            static  int  aes_ctr_encrypt_128bit(xbuffer_t & in_output,uint8_t aes_key[16],uint8_t aes_nounce[16]);
            static  int  aes_ctr_encrypt_128bit(const uint8_t* in_output_data,const int32_t data_size,uint8_t aes_key[16],uint8_t aes_nounce[16]);
            static  int  aes_ctr_decrypt_128bit(xmemh_t & in_output,uint8_t aes_key[16],uint8_t aes_nounce[16]);
            static  int  aes_ctr_decrypt_128bit(xbuffer_t & in_output,uint8_t aes_key[16],uint8_t aes_nounce[16]);
            static  int  aes_ctr_decrypt_128bit(const uint8_t* in_output_data,const int32_t data_size,uint8_t aes_key[16],uint8_t aes_nounce[16]);

            
            //note:256bit aes still using 128bit block to hold data,so the init iv must be 16 bytes length
            static  int  aes_ctr_encrypt_256bit(xmemh_t & in_output,uint8_t aes_key[32],uint8_t aes_nounce[16]);
            static  int  aes_ctr_encrypt_256bit(xbuffer_t & in_output,uint8_t aes_key[32],uint8_t aes_nounce[16]);
            static  int  aes_ctr_encrypt_256bit(const uint8_t* in_output_data,const int32_t data_size,uint8_t aes_key[32],uint8_t aes_nounce[16]);
            static  int  aes_ctr_decrypt_256bit(xmemh_t & in_output,uint8_t aes_key[32],uint8_t aes_nounce[16]);
            static  int  aes_ctr_decrypt_256bit(xbuffer_t & in_output,uint8_t aes_key[32],uint8_t aes_nounce[16]);
            static  int  aes_ctr_decrypt_256bit(const uint8_t* in_output_data,const int32_t data_size,uint8_t aes_key[32],uint8_t aes_nounce[16]);
        };
    }
};
