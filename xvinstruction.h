// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "xvmethod.h"

namespace top
{
    namespace base
    {
        //xobjscript : function-oriented script
        enum enum_xvinstruct_class : int8_t //4bit only
        {
            //core instrution
            enum_xvinstruct_code_nil                = 0x00,  //nil op code,do nothing
            enum_xvinstruct_code_abort              = 0x01,  //stop & abort current execution,quit it
            enum_xvinstruct_code_push_value         = 0x02,  //push value to stack
            enum_xvinstruct_code_pop_value          = 0x03,  //pop top value from stack
            enum_xvinstruct_code_return             = 0x04,  //restore stack to begin of function execution
            enum_xvinstruct_code_if                 = 0x05,  //all condition control
 
            enum_xvinstruct_class_core_function     = 0x0a,
            enum_xvinstruct_class_state_function    = 0x0b,  //call to target with method id/
            //extend call instructions
            enum_xvinstruct_class_contract_function = 0x0c,  //VM contract ' function,allow defined max as 111 method id
            enum_xvinstruct_class_rpc_function      = 0x0d,  //RPC call as refection mapping,allow defined max as 111 method id
            enum_xvinstruct_code_max                = 0x0F,  //4bit,NEVER over this max value
        };
    
        //predefined method id for enum_xvinstruct_class_core_function
        enum enum_xvinstruct_core_method_id : int8_t
        {
            enum_xvinstruct_core_method_invalid            = 0x00,  //0 always be invalid
            
            enum_xvinstruct_core_method_add_value          = 0x01,  //logic add
            enum_xvinstruct_core_method_sub_value          = 0x02,  //logic sub
            enum_xvinstruct_core_method_mul_value          = 0x03,  //logic multiplcation
            enum_xvinstruct_core_method_div_value          = 0x04,  //logic dividion
            enum_xvinstruct_core_method_reset_value        = 0x05,  //logic reset/init to "logic zero"
            enum_xvinstruct_core_method_compare_value      = 0x06,  //all compare related functio
        };
    
        //predefined method id for enum_xvinstruct_class_state_function
        enum enum_xvinstruct_state_method_id : int8_t
        {
            enum_xvinstruct_state_method_invalid            = 0x00,  //0 always be invalid
            
            //vstage need implement those methods
            enum_xvinstruct_state_method_new_account        = 0x01,
            enum_xvinstruct_state_method_del_account        = 0x02,
            enum_xvinstruct_state_method_load_account       = 0x03,
            enum_xvinstruct_state_method_unload_account     = 0x04,
            enum_xvinstruct_state_method_save_account       = 0x05,
            enum_xvinstruct_state_method_link_account       = 0x06,  //link parent-child account,once linked not allow unlink anymore
 
            //vstate need implement those methods
            enum_xvinstruct_state_method_new_property       = 0x07,  //create application property,and execute as current target
            enum_xvinstruct_state_method_del_property       = 0x08,  //delete application property
            enum_xvinstruct_state_method_lock_property      = 0x09,  //lock property as  readonly
            enum_xvinstruct_state_method_unlock_property    = 0x0a,  //unlock property to normal status
            
            //core operation for property
            enum_xvinstruct_state_method_integer_reset      = 0x10,  //int8/int16/int32/int64/uint64_t
            enum_xvinstruct_state_method_string_reset       = 0x11,  //for std::string
            enum_xvinstruct_state_method_nonce_alloc        = 0x12,  //for nonce(atomic increase)
            enum_xvinstruct_state_method_code_deploy        = 0x13,  //deploy contract code
            
            enum_xvinstruct_state_method_query_token        = 0x16,  //check balance of token
            enum_xvinstruct_state_method_deposit_token      = 0x17,  //deposit balance
            enum_xvinstruct_state_method_withdraw_token     = 0x18,  //withdraw balance
            enum_xvinstruct_state_method_lock_token         = 0x19,  //amount of tokens are locked that not allow spend until unlocked
            enum_xvinstruct_state_method_unlock_token       = 0x1a,  //must match unlock 'condition(e.g. time)
            
            enum_xvinstruct_state_method_key_deploy         = 0x1b,  //deploy a key
            enum_xvinstruct_state_method_key_disable        = 0x1c,  //disable the key,once disable not allow reenable
            enum_xvinstruct_state_method_key_deactive       = 0x1d,  //deactive a key
            
            
            //map = std::deque<xxx>
            enum_xvinstruct_state_method_queue_push_front   = 0x20,
            enum_xvinstruct_state_method_queue_pop_front    = 0x21,
            enum_xvinstruct_state_method_queue_push_back    = 0x22,
            enum_xvinstruct_state_method_queue_pop_back     = 0x23,
            enum_xvinstruct_state_method_queue_update       = 0x24,
            enum_xvinstruct_state_method_queue_reset        = 0x25,
            
            //map = std::map<string,xxx>
            enum_xvinstruct_state_method_map_insert         = 0x26,
            enum_xvinstruct_state_method_map_erase          = 0x27,
            enum_xvinstruct_state_method_map_reset          = 0x28,
            //hashmap = map<string,map<string,string>> //key->field->value
            enum_xvinstruct_state_method_hashmap_insert     = 0x29,
            enum_xvinstruct_state_method_hashmap_erase      = 0x2a,
            enum_xvinstruct_state_method_hashmap_reset      = 0x2b,
            
            enum_xvinstruct_state_method_max                = INT8_MAX,//7bit,NEVER over this max value
        };
        
        //Macro & logic Instruction
        class xvoperate_t : public xvmethod_t
        {
        public:
            xvoperate_t(){}; //construct nil op
            
            //core instruction
            xvoperate_t(enum_xvinstruct_class code)
                :xvmethod_t(std::string(),code,0)
            {
            }
            xvoperate_t(enum_xvinstruct_class code,xvalue_t & param)
                :xvmethod_t(std::string(),code,0,param)
            {
            }
            xvoperate_t(enum_xvinstruct_class code,xvalue_t & param1,xvalue_t & param2)
                :xvmethod_t(std::string(),code,0,param1,param2)
            {
            }
            xvoperate_t(enum_xvinstruct_class code,xvalue_t & param1,xvalue_t & param2,xvalue_t & param3)
                :xvmethod_t(std::string(),code,0,param1,param2,param3)
            {
            }
            
            //base function related construction
            xvoperate_t(enum_xvinstruct_state_method_id api)
                :xvmethod_t(std::string(),enum_xvinstruct_class_state_function,api)
            {
            }
            xvoperate_t(enum_xvinstruct_state_method_id api,xvalue_t & param)
                :xvmethod_t(std::string(),enum_xvinstruct_class_state_function,api,param)
            {
            }
            xvoperate_t(enum_xvinstruct_state_method_id api,xvalue_t & param1,xvalue_t & param2)
                :xvmethod_t(std::string(),enum_xvinstruct_class_state_function,api,param1,param2)
            {
            }
            xvoperate_t(enum_xvinstruct_state_method_id api,xvalue_t & param1,xvalue_t & param2,xvalue_t & param3)
                :xvmethod_t(std::string(),enum_xvinstruct_class_state_function,api,param1,param2,param3)
            {
            }
            
            ~xvoperate_t(){};
            
        private: //dont implement those
            xvoperate_t(xvoperate_t && obj);
            xvoperate_t(const xvoperate_t & obj);
            xvoperate_t & operator = (const xvoperate_t & right);
        };
    };
};
