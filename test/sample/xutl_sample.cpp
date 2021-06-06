// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>

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

const int xckey_max_srand_seed_count_ = 16;
static uint64_t g_xckey_srand_seed[xckey_max_srand_seed_count_] = {0}; //64 random seed to improve the random performance
static uint32_t xrandom32()
{
    uint64_t rand_value;
    const uint32_t stack_address_of_rand_value = (uint32_t)((uint64_t)&rand_value);
    
    uint64_t & rand_seed = g_xckey_srand_seed[stack_address_of_rand_value % xckey_max_srand_seed_count_];
    if(rand_seed == 0) //random reset the seed
    {
        srand((unsigned)time(NULL)); //add abosulte time
        rand_seed += rand();         //add random offset
        rand_seed += clock();        //add relative time as CPU tick
        rand_seed += stack_address_of_rand_value; //add random address
        
        auto now = std::chrono::system_clock::now();
        auto now_nano = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
        rand_seed += now_nano.time_since_epoch().count();
    }
    const uint64_t  init_seed = rand_seed;
    rand_value = init_seed * 1664525 + 1013904223 + rand_value;
    rand_seed = (rand_value >> 8);//select more random part
    return (uint32_t)rand_seed;
}

//static void xrandom_buffer(uint8_t *buf, size_t len)
//{
//    uint32_t r = 0;
//    for (size_t i = 0; i < len; i++) {
//        if (i % 4 == 0) {
//            r = xrandom32();
//        }
//        buf[i] = (r >> ((i % 4) * 8)) & 0xFF;
//    }
//}

/*
static uint32_t get_in_slots(const std::set<uint32_t> &subset_slots,
                             const std::set<uint32_t> &out_slots,
                             std::set<uint32_t> &in_slots,
                             uint32_t rand,
                             uint32_t in_num,
                             const std::vector<char> &elect_nodes,
                             uint64_t clock) {
    xassert(elect_nodes.size() >= in_num);
    xassert(elect_nodes.size() >= subset_slots.size());
    if (in_num == 0)
    {
        return 0;
    }
    else if(in_num > elect_nodes.size())
    {
        in_num = elect_nodes.size();
    }
    
    bool out_slot_can_in_again = (in_num < elect_nodes.size() - subset_slots.size());
    
    
    std::vector<uint32_t> difference_subset_and_elect_nodes;
    for(int index = 0; index < elect_nodes.size(); ++index)
    {
        uint32_t slot = elect_nodes.at(index).get_slot_id();
        auto res = subset_slots.find(slot);
        if(res == subset_slots.end()) //not found
        {
            difference_subset_and_elect_nodes.push_back(slot);
        }
    }
    
    if(difference_subset_and_elect_nodes.size() >= in_num) //must have enough candidates to join subset_slots
    {
        while(in_slots.size() < in_num)
        {
            //random pickup one from difference_subset_and_elect_nodes
            const uint32_t random_pick_index = rand % difference_subset_and_elect_nodes.size();
            const uint32_t slot = difference_subset_and_elect_nodes[random_pick_index];
            
            //add to in_slots
            in_slots.insert(slot);
            rand = (uint32_t)(slot * clock); // next rand set slot
            
            //remove the selected item
            difference_subset_and_elect_nodes.erase(difference_subset_and_elect_nodes.begin() + random_pick_index);
        }
    }
    else //if(difference_subset_and_elect_nodes.size() > out_slots.size())
    {
        for(std::vector<uint32_t>::iterator it = difference_subset_and_elect_nodes.begin(); it != difference_subset_and_elect_nodes.end(); ++it)
        {
            in_slots.insert(*it);
        }
        //
    }

    
    
    
    do {
        bool find = false;
        uint32_t index = rand % elect_nodes.size();
        do {
            auto slot = elect_nodes.at(index).get_slot_id();
            auto b = subset_slots.find(slot);
            if ( b == subset_slots.end() ||
                (out_can_in && b != subset_slots.end() && out_slots.find(slot) != out_slots.end())) { // ok, can in
                auto ret = in_outs.insert(slot);
                if (ret.second) { // insert success
                    rand = slot * clock; // next rand set slot
                    find = true;
                }
            }
            if (++index == elect_nodes.size()) { // find next
                index = 0;
            }
        } while (!find);
        
    } while (in_outs.size() < in_num);
    assert(in_outs.size() == in_num);
    return 0;
}
*/

// The following code is not supposed to be used in a production environment.
// It's included only to make the library testable.
// The message above tries to prevent any accidental use outside of the test
// environment.
//
// You are supposed to replace the random8() and random32() function with your
// own secure code. There is also a possibility to replace the random_buffer()
// function as it is defined as a weak symbol.

static uint32_t seed = 0;

void random_reseed(const uint32_t value) { seed += value; }

uint64_t sys_random() //generate a random number by system ' kernel,it read /dev/urandom for Linux and MacOS
{
    uint64_t random_seed;
    
#if defined(LINUX) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__APPLE__)
    int sys_fd = open("/dev/urandom", O_RDONLY); //O_RDONLY is 0
    if(sys_fd >= 0)
    {
        ssize_t result = read(sys_fd,&random_seed,sizeof(random_seed));
        close(sys_fd); //close first
        if(result > 0)
        {
            return random_seed;
        }
    }
#endif
    random_seed = (uint64_t)&random_seed;
    random_seed += rand();
    random_seed += rand();
    return random_seed;
}

uint32_t random32(void) {
    // Linear congruential generator from Numerical Recipes
    // https://en.wikipedia.org/wiki/Linear_congruential_generator
    if(seed < 1048576) //too small
    {
        uint64_t init_seed = sys_random();
        init_seed += rand(); //already init srand() when module start
        seed = (uint32_t)init_seed;
    }
    else //remix
    {
        seed += rand();
    }
    seed = 1664525 * seed + 1013904223;
    return seed;
}

int test_utility(bool is_stress_test)
{
    printf("------------------------[test_utility] start -----------------------------  \n");

    int max_test_round = 100;
    if(is_stress_test)
        max_test_round = 10000;
    
    uint64_t max_64bit = (uint64_t)-1;
    max_64bit += 1;
    max_64bit += 1;
    max_64bit *= 256;
    
    printf("c random init:%d \n",random32());
    printf("c random fresh:%d \n",random32());
    printf("sys random init:%llu \n",xsys_utl::get_sys_random_number());
    
    std::string random_string = xsys_utl::get_sys_random_string(255);
    printf("sys random string:%s \n",random_string.c_str());

    std::string  test_substr("mWr");
    const uint64_t value = xstring_utl::hex2uint64(test_substr);
    //test clock/timing function
    if(value != 0)
    {

        const int64_t time_from_gettimeofday = top::base::xtime_utl::gettimeofday() * 1000;
        const int64_t time_from_timenow_ms   = top::base::xtime_utl::time_now_ms();
    
        printf("sys timing:time_from_gettimeofday=%lld vs time_from_timenow_ms=%lld \n",time_from_gettimeofday,time_from_timenow_ms);
    }
    
  
    //test nullptr and NULL;
    int* int_c_ptr = NULL;
    if(nullptr == int_c_ptr)
    {
        int_c_ptr = 0;
    }
    if(nullptr == int_c_ptr)
    {
        int_c_ptr = NULL;
    }
    if(int_c_ptr == nullptr)
    {
        int_c_ptr = new int;
    }
    delete int_c_ptr;
    
    
    int* int_cpp_ptr = nullptr;
    if(0 == int_cpp_ptr)
    {
        int_cpp_ptr = nullptr;
    }
    if(NULL == int_cpp_ptr)
    {
        int_cpp_ptr = nullptr;
    }
    if(int_cpp_ptr == 0)
    {
        int_cpp_ptr = nullptr;
    }
    if(int_cpp_ptr == NULL)
    {
        int_cpp_ptr = new int;
    }
    delete int_cpp_ptr;
    
    //uint8_t _buffer[256];
    //xrandom_buffer(_buffer,sizeof(_buffer));
    for(int i = 0; i < 256; ++i)
    {
        uint32_t random_value = xrandom32();
        printf("xrandom32()-> %u \n",random_value);
    }
    
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
    
    //test xmalloc & xfree
    {
        int target_size = 2048;
        void * mem_ptr = top::base::xmalloc(top::base::xcontext_t::instance(), target_size);
        memset(mem_ptr,0,target_size);
        top::base::xfree(top::base::xcontext_t::instance(), mem_ptr);
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
    
    //compress and decompress for small piece data
    printf("-----------[compress_to_stream/decompresss] start----------------------\n");
    {
        for (uint32_t i = 0; i < 100000; i++)
        {
            std::string org_bin;
            std::string output_bin;
            std::string final_bin;
            {
                top::base::xautostream_t<1024> _raw_stream(top::base::xcontext_t::instance());
                _raw_stream << "11111";
                _raw_stream << std::to_string(i);
                _raw_stream << top::base::xtime_utl::get_fast_random();
                _raw_stream << top::base::xtime_utl::get_fast_random();
                _raw_stream << top::base::xtime_utl::get_fast_random();
                
                org_bin.assign((const char*)_raw_stream.data(),_raw_stream.size());
                top::base::xstream_t::compress_to_string(org_bin,output_bin);
            }
            {
                const int decompress_result = top::base::xstream_t::decompress_from_string(output_bin,final_bin);
                xassert(decompress_result > 0);
                xassert(final_bin == org_bin);
                if(final_bin != org_bin)
                    return -9;
            }
        }
    }
    printf("-----------[compress_to_stream/decompresss] finish----------------------\n");
    
    //compress and decompress for random piece data
    printf("-----------[compress_to_string/decompresss] start----------------------\n");
    {
        for (uint32_t i = 0; i < 100000; i++)
        {
            std::string org_bin;
            std::string output_bin;
            std::string final_bin;
            std::string rand_str;
            int32_t rand_i1;
            uint32_t rand_i2;
            uint64_t rand_i3;
            {
                //stream and compress
                {
                    top::base::xautostream_t<1024> _raw_stream(top::base::xcontext_t::instance());
                    
                    rand_i1 = top::base::xtime_utl::get_fast_random();
                    if( (rand_i1 > 0) && ((i % 2) == 0) )
                        rand_i1 = -rand_i1;
                    
                    _raw_stream.write_compact_var(rand_i1);
                    rand_i2 = top::base::xtime_utl::get_fast_randomu();
                    _raw_stream.write_compact_var(rand_i2);
                    rand_i3 = top::base::xtime_utl::get_fast_random64();
                    _raw_stream.write_compact_var(rand_i3);
                    
                    const uint16_t rand_size = (uint16_t)top::base::xtime_utl::get_fast_random();
                    rand_str.resize(rand_size);
                    for(int i = 0; i < rand_size; ++i)
                    {
                        rand_str[i] = (char)(i*rand_i2);
                    }
                    _raw_stream.write_compact_var(rand_str);
                    org_bin.assign((const char*)_raw_stream.data(),_raw_stream.size());
                    top::base::xstream_t::compress_to_string(org_bin,output_bin);
                }
                {
                    const int decompress_result = top::base::xstream_t::decompress_from_string(output_bin,final_bin);
                    xassert(decompress_result > 0);
                    xassert(final_bin == org_bin);
                    if(final_bin != org_bin)
                        return -10;
                }
                
                {
                    int32_t  verify_rand_i1;
                    uint32_t verify_rand_i2;
                    uint64_t verify_rand_i3;
                    std::string verify_rand_str;
                    top::base::xstream_t _stream(top::base::xcontext_t::instance(),(uint8_t*)final_bin.data(),(uint32_t)final_bin.size());
                    _stream.read_compact_var(verify_rand_i1);
                    _stream.read_compact_var(verify_rand_i2);
                    _stream.read_compact_var(verify_rand_i3);
                    _stream.read_compact_var(verify_rand_str);

                    xassert(verify_rand_i1 == rand_i1);
                    if(verify_rand_i1 != rand_i1)
                        return -11;
                    
                    xassert(verify_rand_i2 == rand_i2);
                    if(verify_rand_i2 != rand_i2)
                        return -11;
                    
                    xassert(verify_rand_i3 == rand_i3);
                    if(verify_rand_i3 != rand_i3)
                        return -11;
                    
                    xassert(verify_rand_str == rand_str);
                    if(verify_rand_str != rand_str)
                        return -11;
                }
            }
        }
    }
    printf("-----------[compress_to_string/decompresss] finish----------------------\n");
    
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
