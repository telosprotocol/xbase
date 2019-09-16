// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "xlog.h"
#include "xdata.h"
#include "xpacket.h"
#include "xutl.h"
#include "xaes.h"
#include "xcompress.h"
#include "xobfuscation.h"
#include "xcontext.h"
#include "xdfcurve.h"

using namespace top::base;

int test_utility(bool is_stress_test)
{
    printf("------------------------[test_utility] start -----------------------------  \n");

    int max_test_round = 100;
    if(is_stress_test)
        max_test_round = 10000;
    
    std::string test_raw_data = "welcome aes data";
    const uint32_t random_seed = xtime_utl::get_fast_randomu() % 128;
    for(int i = 0; i < 512; ++i) //avg 512 bytes per packet
    {
        test_raw_data.push_back((uint8_t)(i * random_seed));
    }
    const int  raw_data_size = (int)test_raw_data.size();
    
    top::base::xcontext_t::instance();
    const int  test_round = 10;
   // if(test_round == 1)
    {
        const int  test_xpacket_round = 100000;
        for(int i = 0; i < test_round; ++i)
        {
            const int64_t start_time = xtime_utl::time_now_ms();
            for(int j = 0; j < test_xpacket_round; ++j)
            {
                std::string local_string(test_raw_data.data(),test_raw_data.size());
                if(local_string.empty())
                    return 1;
            }
            const int64_t end_time = xtime_utl::time_now_ms();
            const int duration = (int)(end_time - start_time) + 1;
            printf("----[std::string] successsful finish %d round with duration(%d ms),speed=%d----  \n",test_xpacket_round,duration,test_xpacket_round/duration);
        }
    }
    
    {
        uint8_t local_buf[4096];
        const int  test_xpacket_round = 100000;
        for(int i = 0; i < test_round; ++i)
        {
            const int64_t start_time = xtime_utl::time_now_ms();
            for(int j = 0; j < test_xpacket_round; ++j)
            {
                //use case to improve performance for memory alloc by resuing outside memory
                xpacket_t  localpacket(xcontext_t::instance(),local_buf, sizeof(local_buf),0,false);
                //xpacket_t  localpacket(xcontext_t::instance());
                localpacket.get_body().push_back((uint8_t*)test_raw_data.data(),(int)test_raw_data.size());
                //localpacket.get_body().pop_back((int)test_raw_data.size());
            }
            const int64_t end_time = xtime_utl::time_now_ms();
            const int duration = (int)(end_time - start_time) + 1;
            printf("----[xpacket_t] successsful finish %d round with duration(%d ms),speed=%d----  \n",test_xpacket_round,duration,test_xpacket_round/duration);
        }
    }
    //test serialize
    {
        xautostream_t<1024> test_stream(top::base::xcontext_t::instance());
        
        xuint_t<128>    random_128bit_input;
        xuint_t<256>    random_256bit_input;
        std::string                         random_string_input = test_raw_data;
        std::list<std::string>              random_list_input;
        std::vector<std::string>            random_vector_input;
        std::set<std::string>               random_set_input;
        std::map<int,std::string>           random_map_input;
        std::unordered_map<int,std::string>  random_unordered_map_input;
        
        
        for(int i = 0; i < (random_seed % 256); ++i) //random init
        {
            random_list_input.push_back(random_string_input);
            random_vector_input.push_back(random_string_input);
            random_set_input.insert(random_string_input);
            random_map_input[i] = random_string_input;
            random_unordered_map_input[i] = random_string_input;
        }
        //string
        {
            test_stream << random_string_input;
            std::string output;
            test_stream >> output;
            if(output != random_string_input)
            {
                printf("------------------------[xstream::serialize string fail -----------------------------  \n");
                return -1;
            }
        }
        //vector
        {
            test_stream << random_vector_input;
            std::vector<std::string> output;
            test_stream >> output;
            if(output != random_vector_input)
            {
                printf("------------------------[xstream::serialize vector fail -----------------------------  \n");
                return -1;
            }
        }
        //list
        {
            test_stream << random_list_input;
            std::list<std::string> output;
            test_stream >> output;
            if(output != random_list_input)
            {
                printf("------------------------[xstream::serialize list fail -----------------------------  \n");
                return -1;
            }
        }
        //set
        {
            test_stream << random_set_input;
            std::set<std::string> output;
            test_stream >> output;
            if(output != random_set_input)
            {
                printf("------------------------[xstream::serialize set fail -----------------------------  \n");
                return -1;
            }
        }
        //map
        {
            test_stream << random_map_input;
            std::map<int,std::string> output;
            test_stream >> output;
            if(output != random_map_input)
            {
                printf("------------------------[xstream::serialize map fail -----------------------------  \n");
                return -1;
            }
        }
        //unordered_map
        {
            test_stream << random_unordered_map_input;
            std::unordered_map<int,std::string> output;
            test_stream >> output;
            if(output != random_unordered_map_input)
            {
                printf("------------------------[xstream::serialize unordered_map fail -----------------------------  \n");
                return -1;
            }
        }
    }
    
    //test obfucation
    {
        const int64_t start_time = xtime_utl::time_now_ms();
        printf("-----------[obfucation] start,time:%lld,raw_data_size=%d-----------------------------  \n",start_time,raw_data_size);
        
        for(int i = 0; i < max_test_round; ++i)
        {
            for(int tags = 0; tags < 1; ++tags)
            {
                xpacket_t test_packet;
                const int random_address_offset = xtime_utl::get_fast_randomu() % 128;
                test_packet.get_body().push_back(NULL,random_address_offset);
                test_packet.get_body().push_back((uint8_t*)test_raw_data.data(), raw_data_size);
                test_packet.get_body().pop_front(random_address_offset);
                
                const uint32_t random_seed = xtime_utl::get_fast_randomu();
                if(false == xobfuscation_t::encode(test_packet.get_body(),random_seed))
                {
                    printf("------------------------[xobfuscation_utl::encode] fail -----------------------------  \n");
                    return -1;
                }
                
                const int unobfuscation_size = xobfuscation_t::decode(test_packet.get_body(), random_seed);
                if( unobfuscation_size != raw_data_size)
                {
                    printf("------------------------[xobfuscation_utl::decode] fail unobfuscation_size=%d-----------------------------  \n",unobfuscation_size);
                    return -2;
                }
                
                if(memcmp(test_packet.get_body().data(), test_raw_data.data(), raw_data_size) != 0)
                {
                    printf("------------------------[xobfuscation_utl::decode] fail as content is bad -----------------------------  \n");
                    return -3;
                }
            }
        }
        
        const int64_t end_time = xtime_utl::time_now_ms();
        const int duration = (int)(end_time - start_time) + 1;
        const int speed = max_test_round * 1000 / duration;
        printf("----[obfucation] successsful finish %d round with duration(%d ms),speed=%d----  \n",max_test_round,duration,speed);
        
    }
    //test AES function
    {
        uint8_t aes_key[32];
        for(int i = 0; i < 32; ++i)
        {
            aes_key[i] = xtime_utl::get_fast_randomu();
        }
        
        uint8_t aes_iv[16];
        for(int i = 0; i < 16; ++i)
        {
            aes_iv[i] = xtime_utl::get_fast_randomu();
        }
    
        //128bit CBC encrypt/decrypt
        {
            const int64_t start_time = xtime_utl::time_now_ms();
            printf("-----------[aes_cbc_128bit] start,time:%lld -----------------------------  \n",start_time);
            
            uint8_t copy_aes_iv[16];
            for(int i = 0; i < max_test_round; ++i)
            {
                xpacket_t aes_packet;
                
                const int random_address_offset = xtime_utl::get_fast_randomu() % 128;
                aes_packet.get_body().push_back(NULL,random_address_offset);
                aes_packet.get_body().push_back((uint8_t*)test_raw_data.data(), raw_data_size);
                aes_packet.get_body().pop_front(random_address_offset);
                
                memcpy(copy_aes_iv, aes_iv, sizeof(copy_aes_iv));//since encrypt may updated iv dynamicly
                const int encrypt_size = xaes_t::aes_cbc_encrypt_128bit(aes_packet.get_body(), aes_key, copy_aes_iv);
                xassert(encrypt_size >= test_raw_data.size());
                
                memcpy(copy_aes_iv, aes_iv, sizeof(copy_aes_iv));//since encrypt may updated iv dynamicly
                const int decrypt_size = xaes_t::aes_cbc_decrypt_128bit(aes_packet.get_body(),aes_key, copy_aes_iv);
                xassert(decrypt_size == test_raw_data.size());
                
                if(decrypt_size != test_raw_data.size())
                {
                    printf("------------------------[aes_cbc_128bit] fail as res=%d -----------------------------  \n",decrypt_size);
                    return -1;
                }
                if(memcmp(aes_packet.get_body().data(), test_raw_data.data(), decrypt_size) != 0)
                {
                    printf("------------------------[aes_cbc_128bit] fail as content is bad -----------------------------  \n");
                    return -2;
                }
            }
            const int64_t end_time = xtime_utl::time_now_ms();
            const int duration = (int)(end_time - start_time) + 1;
            const int speed = max_test_round * 1000 / duration;
            printf("----[aes_cbc_128bit] successsful finish %d round with duration(%d ms),speed=%d----  \n",max_test_round,duration,speed);
        }
        //256bit CBC encrypt/decrypt
        {
            const int64_t start_time = xtime_utl::time_now_ms();
            printf("-----------[aes_cbc_256bit] start,time:%lld -----------------------------  \n",start_time);
            
            uint8_t copy_aes_iv[16];
            for(int i = 0; i < max_test_round; ++i)
            {
                xpacket_t aes_packet;
                
                const int random_address_offset = xtime_utl::get_fast_randomu() % 128;
                aes_packet.get_body().push_back(NULL,random_address_offset);
                aes_packet.get_body().push_back((uint8_t*)test_raw_data.data(), (uint32_t)test_raw_data.size());
                aes_packet.get_body().pop_front(random_address_offset);
                
                memcpy(copy_aes_iv, aes_iv, sizeof(copy_aes_iv));//since encrypt may updated iv dynamicly
                const int encrypt_size = xaes_t::aes_cbc_encrypt_256bit(aes_packet.get_body(), aes_key, copy_aes_iv);
                xassert(encrypt_size >= test_raw_data.size());
                
                memcpy(copy_aes_iv, aes_iv, sizeof(copy_aes_iv));//since encrypt may updated iv dynamicly
                const int decrypt_size = xaes_t::aes_cbc_decrypt_256bit(aes_packet.get_body(),aes_key, copy_aes_iv);
                xassert(decrypt_size == test_raw_data.size());
                
                if(decrypt_size != test_raw_data.size())
                {
                    printf("------------------------[aes_cbc_256bit] fail as res=%d -----------------------------  \n",decrypt_size);
                    return -1;
                }
                if(memcmp(aes_packet.get_body().data(), test_raw_data.data(), decrypt_size) != 0)
                {
                    printf("------------------------[aes_cbc_256bit] fail as content is bad -----------------------------  \n");
                    return -2;
                }
            }
            const int64_t end_time = xtime_utl::time_now_ms();
            const int duration = (int)(end_time - start_time) + 1;
            const int speed = max_test_round * 1000 / duration;
            printf("----[aes_cbc_256bit] successsful finish %d round with duration(%d ms),speed=%d----  \n",max_test_round,duration,speed);
        }
        
        //128bit CTR encrypt/decrypt
        {
            const int64_t start_time = xtime_utl::time_now_ms();
            printf("-----------[aes_ctr_128bit] start,time:%lld -----------------------------  \n",start_time);
            
            uint8_t copy_aes_iv[16];
            for(int i = 0; i < max_test_round; ++i)
            {
                xpacket_t aes_packet;
                
                const int random_address_offset = xtime_utl::get_fast_randomu() % 128;
                aes_packet.get_body().push_back(NULL,random_address_offset);
                aes_packet.get_body().push_back((uint8_t*)test_raw_data.data(), (uint32_t)test_raw_data.size());
                aes_packet.get_body().pop_front(random_address_offset);
                
                memcpy(copy_aes_iv, aes_iv, sizeof(copy_aes_iv));//since encrypt may updated iv dynamicly
                const int encrypt_size = xaes_t::aes_ctr_encrypt_128bit(aes_packet.get_body(), aes_key, copy_aes_iv);
                xassert(encrypt_size == test_raw_data.size());
                
                memcpy(copy_aes_iv, aes_iv, sizeof(copy_aes_iv));//since encrypt may updated iv dynamicly
                const int decrypt_size = xaes_t::aes_ctr_decrypt_128bit(aes_packet.get_body(),aes_key, copy_aes_iv);
                xassert(decrypt_size == test_raw_data.size());
                
                if(decrypt_size != test_raw_data.size())
                {
                    printf("------------------------[aes_ctr_128bit] fail as res=%d -----------------------------  \n",decrypt_size);
                    return -1;
                }
                if(memcmp(aes_packet.get_body().data(), test_raw_data.data(), decrypt_size) != 0)
                {
                    printf("------------------------[aes_ctr_128bit] fail as content is bad -----------------------------  \n");
                    return -2;
                }
            }
            const int64_t end_time = xtime_utl::time_now_ms();
            const int duration = (int)(end_time - start_time) + 1;
            const int speed = max_test_round * 1000 / duration;
            printf("----[aes_ctr_128bit] successsful finish %d round with duration(%d ms),speed=%d----  \n",max_test_round,duration,speed);
        }
        
        //256bit CTR encrypt/decrypt
        {
            const int64_t start_time = xtime_utl::time_now_ms();
            printf("-----------[aes_ctr_256bit] start,time:%lld -----------------------------  \n",start_time);
            
            uint8_t copy_aes_iv[16];
            for(int i = 0; i < max_test_round; ++i)
            {
                xpacket_t aes_packet;
                
                const int random_address_offset = xtime_utl::get_fast_randomu() % 128;
                aes_packet.get_body().push_back(NULL,random_address_offset);
                aes_packet.get_body().push_back((uint8_t*)test_raw_data.data(), (uint32_t)test_raw_data.size());
                aes_packet.get_body().pop_front(random_address_offset);
                
                memcpy(copy_aes_iv, aes_iv, sizeof(copy_aes_iv));//since encrypt may updated iv dynamicly
                const int encrypt_size = xaes_t::aes_ctr_encrypt_256bit(aes_packet.get_body(), aes_key, copy_aes_iv);
                xassert(encrypt_size == test_raw_data.size());
                
                memcpy(copy_aes_iv, aes_iv, sizeof(copy_aes_iv));//since encrypt may updated iv dynamicly
                const int decrypt_size = xaes_t::aes_ctr_decrypt_256bit(aes_packet.get_body(),aes_key, copy_aes_iv);
                xassert(decrypt_size == test_raw_data.size());
                
                if(decrypt_size != test_raw_data.size())
                {
                    printf("------------------------[aes_ctr_256bit] fail as res=%d -----------------------------  \n",decrypt_size);
                    return -1;
                }
                if(memcmp(aes_packet.get_body().data(), test_raw_data.data(), decrypt_size) != 0)
                {
                    printf("------------------------[aes_ctr_256bit] fail as content is bad -----------------------------  \n");
                    return -2;
                }
            }
            const int64_t end_time = xtime_utl::time_now_ms();
            const int duration = (int)(end_time - start_time) + 1;
            const int speed = max_test_round * 1000 / duration;
            printf("----[aes_ctr_256bit] successsful finish %d round with duration(%d ms),speed=%d----  \n",max_test_round,duration,speed);
        }
        
        //test compress
        {
            const int64_t start_time = xtime_utl::time_now_ms();
            printf("-----------[aes_compress] start,time:%lld -----------------------------  \n",start_time);
            for(int i = 0; i < max_test_round; ++i)
            {
                char compressed_data[1024];
                const int compressed_size = xcompress_t::lz4_compress((const char*)test_raw_data.data(), compressed_data, (int)test_raw_data.size(), sizeof(compressed_data));
                xassert(compressed_size <= test_raw_data.size());
                
                char decompressed_data[1024];
                const int decompressed_size = xcompress_t::lz4_decompress((const char*)compressed_data, decompressed_data, compressed_size, 1024);
                xassert(decompressed_size == test_raw_data.size());
                if(decompressed_size != test_raw_data.size())
                {
                    printf("------------------------[aes_compress] fail as size is bad -----------------------------  \n");
                    return -1;
                }
                
                if(memcmp(decompressed_data, test_raw_data.data(), decompressed_size) != 0)
                {
                    printf("------------------------[aes_compress] fail as content is bad -----------------------------  \n");
                    return -2;
                }
            }
            
            const int64_t end_time = xtime_utl::time_now_ms();
            const int duration = (int)(end_time - start_time) + 1;
            const int speed = max_test_round * 1000 / duration;
            printf("----[aes_compress] successsful finish %d round with duration(%d ms),speed=%d----  \n",max_test_round,duration,speed);
        }
        
        //test x25519 ECDH
        {
            uint8_t   _ecc_public_key_1[32];              //XECDH Public key
            uint8_t   _ecc_private_key_1[32];             //XECDH private key
            uint8_t   _ecc_shared_secret_1[32];           //XECDH shared secret as AES Key
            x25519ECDH_t::create_xdf_key_pair(_ecc_public_key_1, _ecc_private_key_1);
            
            uint8_t   _ecc_public_key_2[32];              //XECDH Public key
            uint8_t   _ecc_private_key_2[32];             //XECDH private key
            uint8_t   _ecc_shared_secret_2[32];           //XECDH shared secret as AES Key
            x25519ECDH_t::create_xdf_key_pair(_ecc_public_key_2, _ecc_private_key_2);
            
            x25519ECDH_t::create_xdf_shared_secret(_ecc_private_key_1,_ecc_public_key_2,_ecc_shared_secret_1);
            x25519ECDH_t::create_xdf_shared_secret(_ecc_private_key_2,_ecc_public_key_1,_ecc_shared_secret_2);
            
            if(memcmp(_ecc_shared_secret_1, _ecc_shared_secret_2, 32) != 0)
            {
                assert(0);
                printf("------------------------[x25519 ECDH] fail as shard secret is not same -----------------------------  \n");
                return -1;
            }
            printf("------------------------[x25519 ECDH] successsful finish -----------------------------  \n");
        }
    }

    
    printf("/////////////////////////////// [test_utility] finish ///////////////////////////////  \n");
    return 0;
    
}
