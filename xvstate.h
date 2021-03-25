// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xvproperty.h"

namespace top
{
    namespace base
    {
        //xvbstate_t(block state) present all properties of one block(specified block height)
        //note: it is not allow to modify anymore after xvbstate_t object is generated from serialize or new(initialize by subclass)
        class xvbstate_t : public xvexegroup_t
        {
            friend class xvblock_t;
            friend class xvblockstore_t;
            typedef xvexegroup_t base;

        public:
            static  const std::string   name(){ return std::string("xvbstat");}
            virtual std::string         get_obj_name() const override {return name();}
            static  const std::string   make_unit_name(const std::string & account, const uint64_t blockheight);
        private:
            static  void  register_object(xcontext_t & context);
            enum{enum_obj_type = enum_xobject_type_vbstate};//allow xbase create xvstate_t object from xdataobj_t::read_from()
            enum{enum_max_property_count = 256}; //only allow 256 properties for each account
            
        public:
            xvbstate_t(const std::string & account_addr,const uint64_t block_height,const std::vector<xvproperty_t*> & properties,enum_xdata_type type = (enum_xdata_type)enum_xobject_type_vbstate);
            
        protected:
            xvbstate_t(enum_xdata_type type = (enum_xdata_type)enum_xobject_type_vbstate);
            xvbstate_t(const xvbstate_t & obj);
            xvbstate_t(const uint64_t new_block_height,const xvbstate_t & source);
            virtual ~xvbstate_t();
            
        private://not implement those private construction
            xvbstate_t(xvbstate_t &&);
            xvbstate_t & operator = (const xvbstate_t & other);

        public:
            virtual xvexeunit_t*  clone() override; //cone a new object with same state //each property is readonly after clone
            xvbstate_t*           clone(const uint64_t clone_to_new_block_height);
            virtual std::string   dump() const override;  //just for debug purpose
            virtual void*         query_interface(const int32_t _enum_xobject_type_) override;//caller need to cast (void*) to related ptr
            
            //bin-log related functions
            enum_xerror_code      encode_change_to_binlog(std::string & output_bin);//
            enum_xerror_code      encode_change_to_binlog(xstream_t & output_bin);//
            bool                  apply_changes_of_binlog(const std::string & from_bin_log);//apply changes to current states,use carefully
            virtual bool          apply_changes_of_binlog(xstream_t & from_bin_log);//apply changes to current states,use carefully
            
            enum_xerror_code      decode_change_from_binlog(const std::string & from_bin_log,std::deque<top::base::xvmethod_t> & out_records);
            enum_xerror_code      decode_change_from_binlog(xstream_t & from_bin_log,std::deque<top::base::xvmethod_t> & out_records);
            
        public://read-only
            const std::string&          get_account_addr() const {return m_account_addr;}
            const std::string&          get_block_output_hash() const {return m_block_output_hash;}
            const uint64_t              get_block_height() const {return m_block_height;}
            
            bool                        find_property(const std::string & property_name); //check whether property already existing
            
        public://note: only allow access by our kernel module. it means private for application'contract
            xauto_ptr<xtokenvar_t>              load_token_var(const std::string & property_name);//for main token(e.g. TOP Token)
            xauto_ptr<xnoncevar_t>              load_nonce_var(const std::string & property_name);//for noance of account
            xauto_ptr<xcodevar_t>               load_code_var(const std::string & property_name); //for contract code of account
            xauto_ptr<xmtokens_t>               load_multiple_tokens_var(const std::string & property_name);//for native tokens
            xauto_ptr<xmkeys_t>                 load_multiple_keys_var(const std::string & property_name); //to manage pubkeys of account
            
        public: //general ones for both kernel and application
            xauto_ptr<xstringvar_t>             load_string_var(const std::string & property_name);
            xauto_ptr<xhashmapvar_t>            load_hashmap_var(const std::string & property_name);
            xauto_ptr<xvproperty_t>             load_property(const std::string & property_name);//general way
            
        public://load function of integer  for both kernel and application
            xauto_ptr<xvintvar_t<int64_t>>      load_int64_var(const std::string & property_name);
            xauto_ptr<xvintvar_t<uint64_t>>     load_uint64_var(const std::string & property_name);
            
        public://load function of deque  for both kernel and application
            xauto_ptr<xdequevar_t<int8_t>>      load_int8_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<int16_t>>     load_int16_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<int32_t>>     load_int32_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<int64_t>>     load_int64_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<uint64_t>>    load_uint64_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<std::string>> load_string_deque_var(const std::string & property_name);
            
        public://load function of map  for both kernel and application
            xauto_ptr<xmapvar_t<int8_t>>        load_int8_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<int16_t>>       load_int16_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<int32_t>>       load_int32_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<int64_t>>       load_int64_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<uint64_t>>      load_uint64_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<std::string>>   load_string_map_var(const std::string & property_name);
            
        public://note: only allow access by our kernel. it means private for application'contract
            xauto_ptr<xtokenvar_t>              new_token_var(const std::string & property_name);//for main token(e.g. TOP Token)
            xauto_ptr<xnoncevar_t>              new_nonce_var(const std::string & property_name);//for noance of account
            xauto_ptr<xcodevar_t>               new_code_var(const std::string & property_name); //for contract code of account
            xauto_ptr<xmtokens_t>               new_multiple_tokens_var(const std::string & property_name);//for native tokens
            xauto_ptr<xmkeys_t>                 new_multiple_keys_var(const std::string & property_name); //to manage pubkeys of account
            
        public://for general ones for both kernel and application
            xauto_ptr<xstringvar_t>             new_string_var(const std::string & property_name);
            xauto_ptr<xhashmapvar_t>            new_hashmap_var(const std::string & property_name);
           
        public://integer related  for both kernel and application
            xauto_ptr<xvintvar_t<int64_t>>      new_int64_var(const std::string & property_name);
            xauto_ptr<xvintvar_t<uint64_t>>     new_uint64_var(const std::string & property_name);
            
        public://new function of deque  for both kernel and application
            xauto_ptr<xdequevar_t<int8_t>>      new_int8_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<int16_t>>     new_int16_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<int32_t>>     new_int32_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<int64_t>>     new_int64_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<uint64_t>>    new_uint64_deque_var(const std::string & property_name);
            xauto_ptr<xdequevar_t<std::string>> new_string_deque_var(const std::string & property_name);
            
        public://new function of map  for both kernel and application
            xauto_ptr<xmapvar_t<int8_t>>        new_int8_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<int16_t>>       new_int16_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<int32_t>>       new_int32_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<int64_t>>       new_int64_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<uint64_t>>      new_uint64_map_var(const std::string & property_name);
            xauto_ptr<xmapvar_t<std::string>>   new_string_map_var(const std::string & property_name);
            
        protected:
            //subclass extend behavior and load more information instead of a raw one
            //return how many bytes readout /writed in, return < 0(enum_xerror_code_type) when have error
            virtual int32_t         do_write(xstream_t & stream) override;//allow subclass extend behavior
            virtual int32_t         do_read(xstream_t & stream)  override;//allow subclass extend behavior
            
            virtual std::string     get_property_value(const std::string & name);
            virtual xvproperty_t*   get_property_object(const std::string & name);
        
            void   set_block_output_hash(const std::string & block_output_hash) {m_block_output_hash = block_output_hash;}
        private:
            std::string     m_account_addr;
            std::string     m_block_output_hash;//output include the binlog generated by xvstate
            uint64_t        m_block_height;
        protected: //functions to modify value actually
            virtual const xvalue_t  do_new_property(const xvmethod_t & op);
            
        private://keep private as safety
            xauto_ptr<xvproperty_t>  new_property(const std::string & property_name,const int propertyType);
            const xvalue_t  new_property_internal(const std::string & property_name,const int propertyType);
            
        private://declare instruction methods supported by xvstate
            BEGIN_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_new_property,do_new_property)
            END_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
        };
        
    };//end of namespace of base

};//end of namespace top
