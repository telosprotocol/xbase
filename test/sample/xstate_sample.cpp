// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "xvstate.h"

template<typename T>
class xvar_t
{
public:
    static T get(top::base::xvalue_t & var){
        return var.get_value<T>();
    }
};

int test_xstate(bool is_stress_test)
{
    //test varint and varstring first
    {
        //short string
        {
            top::base::xautostream_t<1024> _test_stream(top::base::xcontext_t::instance());
            
            const std::string test_string("1234567");
            const int32_t test_i32  = -32;
            const uint32_t test_ui32 = 32;
            const int64_t test_i64  = -64;
            const uint64_t test_ui64 = 64;
            _test_stream.write_compact_var(test_i32);
            _test_stream.write_compact_var(test_ui32);
            _test_stream.write_compact_var(test_i64);
            _test_stream.write_compact_var(test_ui64);
            _test_stream.write_compact_var(test_string);
            
            std::string bin_string;
            top::base::xstream_t::compress_to_string(_test_stream, bin_string);
            _test_stream.reset();
            top::base::xstream_t::decompress_from_string(bin_string, _test_stream);
            
            std::string verify_string;
            int32_t verify_i32  = 0;
            uint32_t verify_ui32 = 0;
            int64_t verify_i64  = 0;
            uint64_t verify_ui64 = 0;
            _test_stream.read_compact_var(verify_i32);
            _test_stream.read_compact_var(verify_ui32);
            _test_stream.read_compact_var(verify_i64);
            _test_stream.read_compact_var(verify_ui64);
            _test_stream.read_compact_var(verify_string);
            
            xassert(test_i32 == verify_i32);
            xassert(test_ui32 == verify_ui32);
            xassert(test_i64 == verify_i64);
            xassert(test_ui64 == verify_ui64);
            xassert(test_string == verify_string);
            
            xassert(_test_stream.size() == 0);
        }
        //medium string(> 64K)
        {
            top::base::xautostream_t<1024> _test_stream(top::base::xcontext_t::instance());
            std::string test_string;
            std::string test_seed_string("0123456789");
            for(int i = 0; i < 6666; ++i) //over 65536 bytes
            {
                test_string += test_seed_string;
            }
            _test_stream.write_compact_var(test_string);
            
            std::string bin_string;
            top::base::xstream_t::compress_to_string(_test_stream, bin_string);
            _test_stream.reset();
            top::base::xstream_t::decompress_from_string(bin_string, _test_stream);
            
            std::string verify_string;
            _test_stream.read_compact_var(verify_string);
            xassert(test_string == verify_string);
            xassert(_test_stream.size() == 0);
        }
        
        //large string(> 16M)
        if(0)
        {
            top::base::xautostream_t<1024> _test_stream(top::base::xcontext_t::instance());
            std::string test_string;
            std::string test_seed_string("0123456789");
            for(int i = 0; i < 1677722; ++i) //over 16M bytes
            {
                test_string += test_seed_string;
            }
            _test_stream.write_compact_var(test_string);
            std::string verify_string;
            _test_stream.read_compact_var(verify_string);
            xassert(test_string == verify_string);
            xassert(_test_stream.size() == 0);
        }
 
    }
    
    const std::string account_addr("abcdefg");
    //vm-rumtime modify state of block & record
    {

        top::base::xvalue_t var(std::string("test string"));
        const int32_t intv = xvar_t<int32_t>::get(var);
        const std::string strv = xvar_t<std::string>::get(var);
        
        top::base::xauto_ptr<top::base::xvbstate_t> hq_block_state(new top::base::xvbstate_t(account_addr, 1,std::vector<top::base::xvproperty_t*>()));
        if(hq_block_state) //build state of a hq block
        {
            top::base::xauto_ptr<top::base::xtokenvar_t> token_property(hq_block_state->new_token_var(std::string("@token")));
            if(token_property)
            {
                auto add_100 = token_property->deposit(top::base::vtoken_t(100));
                auto sub_10 = token_property->withdraw(top::base::vtoken_t(10));
                auto sub_100 = token_property->withdraw(top::base::vtoken_t(100));
            }

            top::base::xauto_ptr<top::base::xmtokens_t> native_tokens(hq_block_state->new_multiple_tokens_var (std::string("@nativetokens")));
            if(native_tokens)
            {
                xassert(native_tokens->deposit("@vpntoken", 100));
                xassert(native_tokens->withdraw("@vpntoken", 10));
                xassert(native_tokens->withdraw("@vpntoken", 100));
                
                xassert(native_tokens->withdraw("@messagingtoken", 10));
                xassert(native_tokens->withdraw("@messagingtoken", 100));
                xassert(native_tokens->deposit("@messagingtoken", 100));
            }
            
            top::base::xauto_ptr<top::base::xnoncevar_t> nonce_property(hq_block_state->new_nonce_var(std::string("@nonce")));
            if(nonce_property){
                auto res = nonce_property->alloc_nonce();
            }
            
            top::base::xauto_ptr<top::base::xcodevar_t> code_property(hq_block_state->new_code_var(std::string("@code")));
            if(code_property){
                code_property->deploy_code("@code");
                auto result = code_property->deploy_code("try overwrite to value");
                xassert(result == false);
            }
            
            top::base::xauto_ptr<top::base::xstringvar_t> string_property(hq_block_state->new_string_var(std::string("@string")));
            if(string_property){
                string_property->reset("test");
                string_property->reset("@string");
            }
            
            top::base::xauto_ptr<top::base::xmapvar_t<std::string>> map_property(hq_block_state->new_string_map_var(std::string("@stringmap")));
            if(map_property){
                auto res = map_property->insert("name", "@stringmap");
            }
            
            top::base::xauto_ptr<top::base::xdequevar_t<std::string>> queue_property(hq_block_state->new_string_deque_var(std::string("@stringdeque")));
            if(map_property){
                auto res = queue_property->push_back("@stringdeque");
            }
            
            top::base::xauto_ptr<top::base::xvintvar_t<int64_t>> int64_property(hq_block_state->new_int64_var(std::string("@5")));
            if(int64_property){
                auto res = int64_property->set(-5);
            }
            
            top::base::xauto_ptr<top::base::xvintvar_t<uint64_t>> uint64_property(hq_block_state->new_uint64_var(std::string("@6")));
            if(uint64_property){
                auto res = uint64_property->set(6);
            }
            
            {
                top::base::xauto_ptr<top::base::xdequevar_t<int8_t>> int8_deque_property(hq_block_state->new_int8_deque_var(std::string("@deque_int8")));
                if(int8_deque_property){
                    auto res = int8_deque_property->push_back(6);
                    int8_deque_property->update(0,-8);
                    xassert(int8_deque_property->query(0) == -8);
                }
                
                top::base::xauto_ptr<top::base::xdequevar_t<int16_t>> int16_deque_property(hq_block_state->new_int16_deque_var(std::string("@deque_int16")));
                if(int16_deque_property){
                    auto res = int16_deque_property->push_front(1);
                    int16_deque_property->pop_front();
                    int16_deque_property->push_back(2);
                    int16_deque_property->update(0,-16);
                    xassert(int16_deque_property->query(0) == -16);
                }
                
                top::base::xauto_ptr<top::base::xdequevar_t<int32_t>> int32_deque_property(hq_block_state->new_int32_deque_var(std::string("@deque_int32")));
                if(int32_deque_property){
                    int32_deque_property->push_back(1);
                    int32_deque_property->pop_back();
                    int32_deque_property->push_front(2);
                    int32_deque_property->update(0,-32);
                    xassert(int32_deque_property->query(0) == -32);
                }
                
                top::base::xauto_ptr<top::base::xdequevar_t<int64_t>> int64_deque_property(hq_block_state->new_int64_deque_var(std::string("@deque_int64")));
                if(int64_deque_property){
                    auto res = int64_deque_property->push_back(-64);
                    xassert(int64_deque_property->query(0) == -64);
                }
                
                top::base::xauto_ptr<top::base::xdequevar_t<uint64_t>> uint64_deque_property(hq_block_state->new_uint64_deque_var(std::string("@deque_uint64")));
                if(uint64_deque_property){
                    auto res = uint64_deque_property->push_back(64);
                    xassert(uint64_deque_property->query(0) == 64);
                }
            }
            
            {
                top::base::xauto_ptr<top::base::xmapvar_t<int8_t>> int8_map_property(hq_block_state->new_int8_map_var(std::string("@map_int8")));
                if(int8_map_property){
                    auto res = int8_map_property->insert("name", 1);
                    int8_map_property->erase("name");
                    int8_map_property->insert("name", -8);
                    xassert(int8_map_property->query("name")  == -8);
                }
                
                top::base::xauto_ptr<top::base::xmapvar_t<int16_t>> int16_map_property(hq_block_state->new_int16_map_var(std::string("@map_int16")));
                if(int16_map_property){
                    auto res = int16_map_property->insert("name", 1);
                    int8_map_property->reset();
                    int8_map_property->insert("name", -8);
                    int16_map_property->insert("name", -16);
                    xassert(int16_map_property->query("name")  == -16);
                }
                
                top::base::xauto_ptr<top::base::xmapvar_t<int32_t>> int32map_property(hq_block_state->new_int32_map_var(std::string("@map_int32")));
                if(int32map_property){
                    auto res = int32map_property->insert("name", 1);
                    std::map<std::string,int32_t> new_map;
                    new_map["name"] = -32;
                    int32map_property->reset(new_map);
                    xassert(int32map_property->query("name")  == -32);
                }
                
                top::base::xauto_ptr<top::base::xmapvar_t<int64_t>> int64map_property(hq_block_state->new_int64_map_var(std::string("@map_int64")));
                if(int64map_property){
                    auto res = int64map_property->insert("name", -64);
                    xassert(int64map_property->query("name")  == -64);
                }
                
                top::base::xauto_ptr<top::base::xmapvar_t<uint64_t>> uint64map_property(hq_block_state->new_uint64_map_var(std::string("@map_uint64")));
                if(uint64map_property){
                    auto res = uint64map_property->insert("name", 64);
                    xassert(uint64map_property->query("name")  == 64);
                }
            }
        }
        
        //test rebuild state from bin-log

        //rebuild bin-log
        std::string recorded_bin_log;
        hq_block_state->encode_change_to_binlog(recorded_bin_log);
        top::base::xauto_ptr<top::base::xvbstate_t> confirm_block_state(new top::base::xvbstate_t(account_addr, 1,std::vector<top::base::xvproperty_t*>()));
        confirm_block_state->apply_changes_of_binlog(recorded_bin_log);
        
        //reserialize block/state
        std::string state_log;
        confirm_block_state->serialize_to_string(state_log);
        top::base::xauto_ptr<top::base::xvbstate_t> reload_state(new top::base::xvbstate_t(account_addr, 1,std::vector<top::base::xvproperty_t*>()));
        reload_state->serialize_from_string(state_log);
        
        //test clone from existing state
        top::base::xauto_ptr<top::base::xvbstate_t> clone_block_state((top::base::xvbstate_t*)reload_state->clone(2));
        if(clone_block_state) //do final verify
        {
            //verify result
            auto token = clone_block_state->load_token_var(std::string("@token"));
            xassert(-10 == token->get_balance());
            
            auto native_tokens = clone_block_state->load_multiple_tokens_var (std::string("@nativetokens"));
            if(native_tokens)
            {
                xassert(-10 == native_tokens->get_balance("@vpntoken"));
                xassert(-10 == native_tokens->get_balance("@messagingtoken"));
            }
            
            auto nonce = clone_block_state->load_nonce_var(std::string("@nonce"));
            xassert(1 == nonce->get_nonce());
            
            auto code = clone_block_state->load_code_var(std::string("@code"));
            xassert(code->get_code() == "@code");
            
            auto str = clone_block_state->load_string_var(std::string("@string"));
            xassert(str->query() == "@string");
            
            auto stringmap = clone_block_state->load_string_map_var(std::string("@stringmap"));
            xassert(stringmap->query("name") == "@stringmap");
            
            auto stringqueue = clone_block_state->load_string_deque_var(std::string("@stringdeque"));
            xassert(stringqueue->query(0) == "@stringdeque");
            
            auto int64_val = clone_block_state->load_int64_var(std::string("@5"));
            xassert(int64_val->get() == -5);
            
            auto uint64_val = clone_block_state->load_uint64_var(std::string("@6"));
            xassert(uint64_val->get() == 6);
            
            //verify deque
            {
                auto deque_int8 = clone_block_state->load_int8_deque_var(std::string("@deque_int8"));
                xassert(deque_int8->query(0) == -8);
                
                auto deque_int16 = clone_block_state->load_int16_deque_var(std::string("@deque_int16"));
                xassert(deque_int16->query(0) == -16);
                
                auto deque_int32 = clone_block_state->load_int32_deque_var(std::string("@deque_int32"));
                xassert(deque_int32->query(0) == -32);
                
                auto deque_int64 = clone_block_state->load_int64_deque_var(std::string("@deque_int64"));
                xassert(deque_int64->query(0) == -64);
                
                auto deque_uint64 = clone_block_state->load_uint64_deque_var(std::string("@deque_uint64"));
                xassert(deque_uint64->query(0) == 64);
            }
            
            //verify map
            {
                auto map_int8 = clone_block_state->load_int8_map_var(std::string("@map_int8"));
                xassert(map_int8->query("name") == -8);
                
                auto map_int16 = clone_block_state->load_int16_map_var(std::string("@map_int16"));
                xassert(map_int16->query("name") == -16);
                
                auto map_int32 = clone_block_state->load_int32_map_var(std::string("@map_int32"));
                xassert(map_int32->query("name") == -32);
                
                auto map_int64 = clone_block_state->load_int64_map_var(std::string("@map_int64"));
                xassert(map_int64->query("name") == -64);
                
                auto map_uint64 = clone_block_state->load_uint64_map_var(std::string("@map_uint64"));
                xassert(map_uint64->query("name") == 64);
            }
        
            xassert(true);
        }
        
    
    }
 
    printf("/////////////////////////////// [test_xstate] finish ///////////////////////////////  \n");
    return 0;
}
