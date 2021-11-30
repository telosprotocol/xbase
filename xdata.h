// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include <string>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "xlock.h"
#include "xobject.h"
#include "xmem.h"

namespace top
{
    namespace base
    {        
        //total 8 flags with lowest 8 bit of uint16
        enum enum_xdata_flag
        {
            enum_xdata_flag_unsued     = 0x01,  //remove this flag to see whether anyone use it
            enum_xdata_flag_encrypted  = 0x02,  //raw data is encyprted by subclass,note: strong recommend to use xaes_t
            enum_xdata_flag_fragment   = 0x04,  //raw data is fragment,which means this object just present one fragment
            enum_xdata_flag_signature  = 0x08,  //raw data is signed by authentication key
            enum_xdata_flag_shared     = 0x10,  //raw data is shared,usally it requiret to readonly and write-on-copy
            enum_xdata_flag_synced     = 0x20,  //raw data is from sync channel
            enum_xdata_flag_acompress  = 0x40,  //raw data compressed by lz4,note: only valid compress method is xcompress_t
            enum_xdata_flag_max        = 0x80,  //can not over this
            
            enum_xdata_flags_mask      = 0xFF,  //Mask to keep them
        };
        
        //xdataunit_t is base calss for pdu,object etc they used for storage or communication, or use as property of some object
        class xdataunit_t : public xobject_t
        {
        public:
            enum enum_xdata_type
            {
                enum_xdata_type_min = enum_xobject_type_data_min,
                enum_xdata_type_undefine        = enum_xdata_type_min + 0,//anonymity type of dataunit
                enum_xdata_type_unknow          = enum_xdata_type_min + 0,//for any unknow type but still following same spec of xdata_header_t
                
                //most basic object
                enum_xdata_type_string          = enum_xdata_type_min + 1,
                enum_xdata_type_string_deque    = enum_xdata_type_min + 2,
                enum_xdata_type_objptr_deque    = enum_xdata_type_min + 3,
                enum_xdata_type_string_map      = enum_xdata_type_min + 4,
                enum_xdata_type_objptr_map      = enum_xdata_type_min + 5,
                //6,7 are avaiable
                enum_xdata_type_set             = enum_xdata_type_min + 8,
                enum_xdata_type_bitset          = enum_xdata_type_min + 9,
                
                enum_xdata_type_mutisigndata    = enum_xdata_type_min + 16, //muti-sig data
                
                enum_xdata_type_vaccountmeta    = enum_xdata_type_min + 19, //meta data of account
                
                //infrastructure & platform for pdu(packet unit)
                enum_xpdu_type_void             = enum_xdata_type_min + 20,  //for general xmsgpdu_t
                enum_xpdu_type_rpc              = enum_xdata_type_min + 21, //for general RPC call & Response
                enum_xpdu_type_proxy            = enum_xdata_type_min + 22, //for general proxy
                enum_xpdu_type_config           = enum_xdata_type_min + 23, //for general configuration
                enum_xpdu_type_update           = enum_xdata_type_min + 24, //for general installation & update
                enum_xpdu_type_event            = enum_xdata_type_min + 25, //for general event
                enum_xpdu_type_mail             = enum_xdata_type_min + 26, //for general mail
                enum_xpdu_type_message          = enum_xdata_type_min + 27, //for general message
                enum_xpdu_type_transaction      = enum_xdata_type_min + 28, //for general transaction
                enum_xpdu_type_notification     = enum_xdata_type_min + 29,//for general notification
                enum_xpdu_type_voice            = enum_xdata_type_min + 30, //for general voice
                enum_xpdu_type_video            = enum_xdata_type_min + 31, //for general video
                enum_xpdu_type_streaming        = enum_xdata_type_min + 32, //for general streaming
                enum_xpdu_type_file             = enum_xdata_type_min + 33, //for general file/documents
                enum_xpdu_type_replica          = enum_xdata_type_min + 34, //for data replicate and synchronize
                enum_xpdu_type_database         = enum_xdata_type_min + 35, //for general DB
                enum_xpdu_type_storage          = enum_xdata_type_min + 36, //for general persisting storage like disk/ssd
                enum_xpdu_type_network          = enum_xdata_type_min + 37, //for physical network layer
                
                //serivce layer for pdu(packet unit)
                enum_xpdu_type_consensus        = enum_xdata_type_min + 38,//general consensus pdu
                enum_xpdu_type_consensus_xbft   = enum_xdata_type_min + 39,//for consenus for TOP xBFT
                enum_xpdu_type_xclock           = enum_xdata_type_min + 40,//for consenus for TOP clock block
                
                enum_xpdu_type_contract         = enum_xdata_type_min + 41,//for smart-contract
                enum_xpdu_type_IoT              = enum_xdata_type_min + 42,//for IoT
                enum_xpdu_type_vpn              = enum_xdata_type_min + 43,//for VPN
                enum_xpdu_type_msgapp           = enum_xdata_type_min + 44,//for msg application
                enum_xpdu_type_servicing        = enum_xdata_type_min + 45,//for servicing layer
                
                //stop define here, dont over enum_xdata_core_type_max
                enum_xdata_core_type_max        = enum_xdata_type_min + 255, //reserved 255 core layer
                
                //after here, defined by application layer or customized for speical purpose
                enum_xdata_type_max = enum_xobject_type_data_max - 1,
            };
            
            typedef enum_xdata_type enum_xpdu_type;
        private:
            //unify layout for data serialization:
            enum { enum_xpdu_header_length = 8};
            struct xpdu_header_t
            {
                int16_t   obj_type;         //data object type
                uint16_t  obj_flags;        //data object flags
                uint32_t  obj_length;       //the length(max 24bit) of the whole data,note:high8bit is reserved as version of pdu
                uint8_t   body_raw[];       //raw data
            }_ALIGNPACKED_1;
        public:
            static xdataunit_t * read_from(const std::string & binary_data);//binary data is placed on string
            static xdataunit_t * read_from(xstream_t & stream);
            static bool          read_head(xstream_t & stream,int16_t & obj_type,uint16_t & obj_flags,uint32_t & obj_length,int & pdu_version);//parse stream directly and copy header information,return false if fail to parse stream(invalid data)
        protected:
            xdataunit_t(enum_xdata_type type);
            xdataunit_t(const xdataunit_t & obj);
            xdataunit_t & operator = (const xdataunit_t & obj);
            virtual ~xdataunit_t();
        private:
            xdataunit_t();
        public:
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override;
            
            inline   int16_t    get_obj_type()      const {return m_obj_type;}
            inline   uint32_t   get_obj_length()    const {return m_obj_length;}
            //inline   int        get_pdu_version()   const {return m_pdu_version;}
            //m_unknown_content keep the unreconized part when old client read packet/block from client of newer version
            std::string         get_unknown_content() const {return m_unknown_content;}
            
            inline  uint16_t    get_unit_flags()    const {return m_obj_flags;}
            inline  void        set_unit_flag(const uint16_t flag)//subclass need arrange those flag well
            {
                uint16_t copy_flags = m_obj_flags;
                copy_flags |= flag;
                m_obj_flags = copy_flags;
            }
            inline  void        reset_unit_flag(const uint16_t flag)//subclass need ensure flag just keep 1 bit
            {
                uint16_t copy_flags = m_obj_flags;
                copy_flags &= (~flag);
                m_obj_flags = copy_flags;
            }
            inline  bool        check_unit_flag(const uint16_t flag) const
            {
                const uint16_t copy_flags = m_obj_flags;
                return ((copy_flags & flag) != 0);
            }
            inline  void        reset_all_unit_flags(const uint16_t all_flags) { m_obj_flags = all_flags;}
        public://not safe for multiple threads,serialize_to/from write and read addtion head of dataobj
            virtual int32_t     serialize_to(xstream_t & stream);        //serialize header and object,return how many bytes is writed
            virtual int32_t     serialize_from(xstream_t & stream);      //serialize header and object,return how many bytes is readed
            
            //just wrap function for serialize_to(),but assign data to string and return
            virtual int32_t     serialize_to_string(std::string & bin_data);
            virtual int32_t     serialize_from_string(const std::string & bin_data);
            
        public://not safe for multiple threads, do_write & do_read write and read raw data of dataobj
            //return how many bytes readout /writed in, return < 0(enum_xerror_code_type) when have error
            virtual int32_t     do_write(xstream_t & stream) = 0;     //write whole object to binary
            virtual int32_t     do_read(xstream_t & stream) = 0;      //read from binary and regeneate content of xdataobj_t
            
        protected:
            int32_t             serialize_to_with_flags(xstream_t & stream,const uint16_t customized_flags); //serialize header and object,return how many bytes is writed
            
        protected:
            int16_t             m_obj_type;         //data object type
            uint16_t            m_obj_flags;        //data object flags
            uint32_t            m_obj_length;       //the length(max 24bit) of the whole data,note:high8bit is reserved as version of pdu
        private:
            uint8_t             m_pdu_version;      //version code of pdu
        protected:
            std::string         m_unknown_content;  //old client read a unit of newer version, keep unknown part here  
        };
        
        //xdatapdu_t obj for event & message & communications with better reliable & safety
        class xdatapdu_t  : public xdataunit_t
        {
        public:
            enum{enum_obj_type = enum_xpdu_type_void};
        public:
            xdatapdu_t();
            virtual ~xdatapdu_t();
        protected:
            xdatapdu_t(enum_xpdu_type xpdu_type);
            xdatapdu_t(const xdatapdu_t & obj);
            xdatapdu_t & operator = (const xdatapdu_t & obj);
        public:
            enum_xpdu_type  get_pdu_class(){return (enum_xpdu_type)get_obj_type();}
            const int32_t   get_msg_type()      const {return m_msg_type;}
            const int32_t   get_msg_TTL()       const {return m_msg_TTL;}
            const uint32_t  get_msg_nonce()     const {return m_msg_nonce;}
            const uint64_t  get_msg_to()        const {return m_msg_to;}
            const uint64_t  get_msg_from()      const {return m_msg_from;}
            const uint32_t  get_session_key()   const {return m_session_key;}
            const uint64_t  get_session_id()    const {return m_session_id;}
            
            //note: return reference as performance reason, please keeping xmsgpdu_t object while using get_msg_body
            const std::string & get_msg_body()  const {return m_msg_body;}
            const std::string & get_msg_auth()  const {return m_msg_auth;}
            
        public:
            virtual std::string  dump() const override; //for debug purpose
            virtual bool         verify_msg() {return false;}; //test whether msg is valid or not,it might involve checking for authentication
            
            void   reset_message(const uint8_t msg_type,const int8_t msg_TTL,const std::string & msg_content,const uint16_t msg_nonce,const uint64_t msg_from, const uint64_t msg_to);

            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override;
        protected://not safe for multiple threads, do_write & do_read write and read raw data of dataobj
            //return how many bytes readout /writed in, return < 0(enum_xerror_code_type) when have error
            virtual int32_t     do_write(xstream_t & stream) override;   //write whole object to binary
            virtual int32_t     do_read(xstream_t & stream)  override;    //read from binary and regeneate content of xdataobj_t
        protected:
            uint64_t    m_msg_to;       //usally is xip Addr,see definition at xbase.h, routing & verify receive very quickly as safety
            uint64_t    m_msg_from;     //usally is xip Addr,see definition at xbase.h, routing & verify  sender very quickly as safety
                
            uint8_t     m_msg_type;      //msg type under pdu object,defined by each enum_xpdu_type object. note: 0 means void/or undefined
            int8_t      m_msg_TTL;       //how many hops left
            uint16_t    m_msg_nonce;     //msg id or as random nonce depends msg 'class and type

            uint32_t    m_session_key;   //for consensus case: dynamic generated for each view,similar mechanisam as choosing leader
            uint64_t    m_session_id;    //for consensus case: it is logic view_id related leader 'election
            
            std::string m_msg_body;      //real msg content that is decode/encode by msg_class and msg_type at subclass
            std::string m_msg_auth;      //optional, authentication for message
        };
        
        //Key-Value data object for cache/storage system
        class xdataobj_t : public xdataunit_t
        {
        protected:
            //unify layout for data serialization:
            enum { enum_xdata_header_length = 32};
            struct xdata_header_t
            {
                int16_t   obj_type;         //data object type
                uint16_t  obj_flags;        //data object flags
                uint32_t  obj_length;       //the length(max 24bit) of the whole data,note:high8bit is reserved as version of pdu
                
                uint32_t  obj_hash;         //64bit xhash(body_raw)
                uint32_t  obj_last_hash;    //point to last m_obj_hash
                uint32_t  obj_version;      //version code of change for object
                uint32_t  obj_expire;       //expire time, seconds from one reference time base(default is 2010/01/01/00:00:00),never expire if 0
                uint64_t  obj_time;         //the time(UTC,ms) when generate obj_hash
                uint8_t   body_raw[];       //raw data
            }_ALIGNPACKED_1;
        public:
            static xdataobj_t * read_from(xstream_t & stream);
        protected:
            xdataobj_t(enum_xdata_type type);
            xdataobj_t(enum_xdata_type type,const std::string & key);
            virtual ~xdataobj_t();
        protected:
            xdataobj_t(const xdataobj_t & obj);//unsafe at multiple-threads'writing & reading
            xdataobj_t & operator = (const xdataobj_t & obj); //unsafe at multiple-threads'writing & reading
        private:
            xdataobj_t();
        public:
            inline   uint32_t            get_obj_hash()      const {return m_obj_hash;}
            inline   std::string         get_key()           const {return m_obj_key;}
            inline   uint32_t            get_hash_of_key()   const {return m_hash_of_objkey;}
            inline   uint32_t            get_obj_version()   const {return m_obj_version;}
            inline   uint64_t            get_obj_time()      const {return m_obj_time;}
            inline   uint32_t            get_obj_last_hash() const {return m_obj_last_hash;}
            inline   uint32_t            get_obj_expire()    const {return m_obj_expire;}       //0 means never expire
            
            inline   uint64_t            get_cache_expire_ms()  const {return m_cache_expire_ms;}     //0 means never expire
            inline   uint64_t            get_last_access_time() const {return m_last_access_time;} //0 means never accessed yet
            inline   uint32_t            get_modified_count()const {return m_modified_count;}
            
            //caller respond to cast (void*) to related  interface ptr
            virtual void*                query_interface(const int32_t _enum_xobject_type_) override;
            
            //default implementation(not best) : created a brand new object and serialzie from stream
            //note: might be issue executing at multiple-threads at same time(writing & reading)
            virtual xdataobj_t*          clone(); //subclass should have own clone implemmentation
            
        public://allow to change them directly
            inline  void                 add_obj_flag(enum_xdata_flag flag){ m_obj_flags|= flag;}
            inline  void                 remove_obj_flag(enum_xdata_flag flag){ m_obj_flags &= (~flag);}
            inline  void                 reset_obj_flags(const uint16_t new_flags) {m_obj_flags = new_flags;}
            inline  void                 set_obj_expire(const uint32_t time_to_expire){m_obj_expire = time_to_expire;}

            inline  void                 set_cache_expire_ms(const uint64_t time_to_expire){m_cache_expire_ms = time_to_expire;}
            inline  void                 set_last_access_time(const uint64_t access_time){m_last_access_time = access_time;}
            uint32_t                     add_modified_count(bool force_atomic_change = false);
            inline  void                 set_hash_of_key(const uint32_t hash_of_key){m_hash_of_objkey = hash_of_key;}
            
        public://not safe for multiple threads,serialize_to/from write and read addtion head of dataobj
            virtual int32_t             serialize_to(xstream_t & stream) override;        //serialize header and object,return how many bytes is writed
            virtual int32_t             serialize_from(xstream_t & stream) override;      //serialize header and object,return how many bytes is readed
            
            //note: full_serialize_to write the actual value of hash & expire time & version etc,but "serialize_to" just let hash & expire time & version etc be default value(0).
            virtual int32_t             full_serialize_to(xstream_t & stream);   //return how many bytes is writed
  
            //call carefully,open to reset flag of modified for subclass
            void                         reset_modified_count(); //use carefully
        protected:
            inline  void                 set_obj_version(const uint32_t version){m_obj_version = version;}
            
        protected: //serialized from/to db
            uint32_t            m_obj_hash;         //32bit of xhash64(body_raw)
            
            uint32_t            m_obj_last_hash;    //point to last m_obj_hash
            uint32_t            m_obj_version;      //version code of change for object
            uint32_t            m_obj_expire;       //expire time, seconds from one reference time base,never expire if 0
            uint64_t            m_obj_time;         //the time(UTC,seconds) when generate obj_hash
            std::string         m_obj_key;          //Object ID/Key,//must have and serialize in &out,  but allow it is empty
        protected:
            uint64_t            m_last_access_time;     //(not serialized),the last GMT time for write & read
            uint64_t            m_cache_expire_ms;      //(not serialized) remove from cache but not delete from db. compare by local time of xtime_utl::time_now_ms
            uint32_t            m_modified_count;       // (not serialized),count how many times modified since last save,it is 0 if nothing changed
            uint32_t            m_hash_of_objkey;   //32bit hash of m_obj_key
        private:
            //data watch list
        };
        
        //for any unknow type but still following same spec of xdata_header_t
        //check  (xdataobj_t->get_object_type() == enum_xdata_type_unknow) to determine whether it is a unknow object
        class xunknowobj_t : public xdataobj_t
        {
        public:
            enum{enum_obj_type = enum_xdata_type_unknow};
        public:
            xunknowobj_t();
            xunknowobj_t(const std::string & key);
        protected:
            virtual ~xunknowobj_t();
            xunknowobj_t(const xunknowobj_t & obj); //note: unsafe at multiple-thread
            xunknowobj_t & operator = (const xunknowobj_t & obj); //note: unsafe at multiple-thread
        public:
            const std::string   get() const {return m_unknow_content;}
            const std::string&  get_value() {return m_unknow_content;} //optimzation
            bool          get(std::string & value);
            virtual  void set(const std::string & value);
        protected: //not safe for multiple threads
            virtual int32_t      do_write(xstream_t & stream) override;       //serialize whole object to binary
            virtual int32_t      do_read(xstream_t & stream) override; //serialize from binary and regeneate content of xdataobj_t
            virtual bool         clear(); //relase resource
            virtual xdataobj_t*  clone() override;
        protected:
            std::string  m_unknow_content;
        };
        
        //stand string with serialize function
        class xstring_t : public xdataobj_t
        {
        public:
            enum{enum_obj_type = enum_xdata_type_string};
        public:
            xstring_t()
            :xdataobj_t(enum_xdata_type_string)
            {
            }
            xstring_t(const std::string & key)
            :xdataobj_t(enum_xdata_type_string,key)
            {
            }
        protected:
            xstring_t(enum_xdata_type override_type) //just open for subclass
                :xdataobj_t(override_type)
            {
            }
            virtual ~xstring_t()
            {
                clear();
            };
            xstring_t(const xstring_t & obj) //note: unsafe at multiple-thread
             :xdataobj_t(enum_xdata_type_string)
            {
                *this = obj;
            }
            xstring_t & operator = (const xstring_t & obj) //note: unsafe at multiple-thread
            {
                m_std_string = obj.m_std_string;
                xdataobj_t::operator = ((xdataobj_t&)obj);
                return *this;
            }
        public://not safe for multiple threads
            std::string get() const {return m_std_string;}
            bool    get(std::string & value)
            {
                value = m_std_string;
                return (m_std_string.empty() == false);
            }
            virtual  void    set(const std::string & value)
            {
                m_std_string = value;
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
            }
            bool    empty()
            {
                return m_std_string.empty();
            }
            int32_t size()
            {
                return (int32_t)m_std_string.size();
            }
            virtual bool   clear() //relase resource
            {
                if (!empty())
                {
                    m_std_string.clear();
                    add_modified_count();
                }
                return true;
            }
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override
            {
                if(enum_xdata_type_string == _enum_xobject_type_)
                    return this;
                return xdataobj_t::query_interface(_enum_xobject_type_);
            }
        protected: //not safe for multiple threads
            virtual int32_t    do_write(xstream_t & stream) override        //serialize whole object to binary
            {
                const int32_t begin_size = stream.size();
                
                stream << m_std_string;
                
                const int32_t end_size = stream.size();
                return (end_size - begin_size);
            }
            virtual int32_t    do_read(xstream_t & stream) override //serialize from binary and regeneate content of xdataobj_t
            {
                const int32_t begin_size = stream.size();
                
                stream >> m_std_string;
                
                const int32_t end_size = stream.size();
                return (begin_size - end_size);
            }
            virtual xdataobj_t*  clone() override //note: unsafe at multiple-thread
            {
                return new xstring_t(*this);
            }
        protected:
            std::string  m_std_string;
        };
        
        //stand deque with serialize function
        template<typename T>
        class xdeque_t : public xdataobj_t
        {
        };
        
        template<>
        class xdeque_t<std::string> : public xdataobj_t
        {
            typedef std::deque<std::string> std_deque;
        public:
            enum{enum_obj_type = enum_xdata_type_string_deque};
        public:
            xdeque_t()
            :xdataobj_t(enum_xdata_type_string_deque)
            {
            }
            xdeque_t(const std::string & key)
            :xdataobj_t(enum_xdata_type_string_deque,key)
            {
            }
        protected:
            xdeque_t(enum_xdata_type override_type) //just open for subclass
            :xdataobj_t(override_type)
            {
            }
            
            virtual ~xdeque_t()
            {
                clear();
            };
            xdeque_t(const xdeque_t & obj) //note: unsafe at multiple-thread
                :xdataobj_t(enum_xdata_type_string_deque)
            {
                *this = obj;
            }
            xdeque_t & operator = (const xdeque_t & obj)//note: unsafe at multiple-thread
            {
                m_std_queue = obj.m_std_queue;
                xdataobj_t::operator = ((xdataobj_t&)obj);
                return *this;
            }
        public: //not safe for multiple threads
            virtual  bool push_back(const std::string & value)
            {
                m_std_queue.emplace_back(value);
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                return true;
            }
            virtual  bool pop_back(std::string & value)
            {
                if(m_std_queue.empty())
                    return false;
                
                value = m_std_queue.back();
                m_std_queue.pop_back();
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                return true;
            }
            bool get_back(std::string & value)
            {
                if(m_std_queue.empty())
                    return false;
                
                value = m_std_queue.back();
                return true;
            }
            
            virtual   bool    push_front(const std::string & value)
            {
                m_std_queue.emplace_front(value);
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                return true;
            }
            virtual   bool    pop_front(std::string & value)
            {
                if(m_std_queue.empty())
                    return false;
                
                value = m_std_queue.front();
                m_std_queue.pop_front();
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                return true;
            }
            bool get_front(std::string & value)
            {
                if(m_std_queue.empty())
                    return false;
                
                value = m_std_queue.front();
                return true;
            }
            bool get(const uint32_t index,std::string & value)
            {
                if(m_std_queue.empty())
                    return false;
                
                if(index < m_std_queue.size())
                {
                    value = m_std_queue.at(index);
                    return true;
                }
                return false;
            }
            
            bool    empty()
            {
                return m_std_queue.empty();
            }
            int32_t size()
            {
                return (int32_t)m_std_queue.size();
            }
            const std::deque<std::string>& get_deque() const {
                return m_std_queue;
            }
            bool erase(const std::string & value) {
                for (auto it = m_std_queue.begin(); it != m_std_queue.end(); ++it) {
                    if (*it == value) {
                        m_std_queue.erase(it);
                        add_modified_count(); //atom is not need as it always increase
                        return true;
                    }
                }
                return false;
            }
            virtual bool clear() {
                if (!empty()) {
                    m_std_queue.clear();
                    add_modified_count();
                }
                return true;
            }
            
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override
            {
                if(enum_xdata_type_string_deque == _enum_xobject_type_)
                    return this;
                return xdataobj_t::query_interface(_enum_xobject_type_);
            }
        protected: //not safe for multiple threads
            virtual int32_t    do_write(xstream_t & stream) override        //serialize whole object to binary
            {
                const int32_t begin_size = stream.size();

                int32_t strings_count = (int32_t)m_std_queue.size();
                stream << strings_count;
                for(std_deque::iterator it = m_std_queue.begin(); it != m_std_queue.end(); ++it)
                {
                    stream << *it;
                }
                
                const int32_t end_size = stream.size();
                return (end_size - begin_size);
            }
            virtual int32_t    do_read(xstream_t & stream) override //serialize from binary and regeneate content of xdataobj_t
            {
                const int32_t begin_size = stream.size();

                int32_t strings_count = 0;
                stream >> strings_count;
                for(int32_t i = 0; i < strings_count; ++i)
                {
                    std::string value;
                    stream >> value;
                    m_std_queue.emplace_back(value);
                }
                
                const int32_t end_size = stream.size();
                return (begin_size - end_size);
            }
            virtual xdataobj_t*  clone() override //note: unsafe at multiple-thread
            {
                return new xdeque_t<std::string>(*this);
            }
        protected:
            std_deque  m_std_queue;
        };
                
        template<>
        class xdeque_t<xdataobj_t> : public xdataobj_t
        {
            typedef std::deque<xdataobj_t*> std_deque;
        public:
            enum{enum_obj_type = enum_xdata_type_objptr_deque};
        public:
            xdeque_t()
            :xdataobj_t(enum_xdata_type_objptr_deque)
            {
            }
            xdeque_t(const std::string & key)
            :xdataobj_t(enum_xdata_type_objptr_deque,key)
            {
            }
        protected:
            xdeque_t(enum_xdata_type override_type) //just open for subclass
            :xdataobj_t(override_type)
            {
            }
            virtual ~xdeque_t()
            {
                clear();
            };
            xdeque_t(const xdeque_t & obj)  //note: unsafe at multiple-thread
                :xdataobj_t(enum_xdata_type_objptr_deque)
            {
                *this = obj;
            }
            xdeque_t & operator = (const xdeque_t & obj) //note: unsafe at multiple-thread
            {
                m_std_queue = obj.m_std_queue;
                xdataobj_t::operator = ((xdataobj_t &)obj);
                return *this;
            }
        public: //not safe for multiple threads
            virtual   bool     push_back(xdataobj_t * object)
            {
                if(NULL == object)
                    return false;
                
                object->add_ref();
                m_std_queue.push_back(object);
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                return true;
            }
            //note:caller respond to release object;
            virtual   xdataobj_t*    pop_back()
            {
                if(m_std_queue.empty())
                    return NULL;
                
                xdataobj_t * object = m_std_queue.back();
                m_std_queue.pop_back();
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                return object;
            }
            xdataobj_t* get_back()  //caller should not release object after get
            {
                if(m_std_queue.empty())
                    return NULL;
                return m_std_queue.back();
            }
            
            virtual   bool     push_front(xdataobj_t * object)
            {
                if(NULL == object)
                    return false;
                
                object->add_ref();
                m_std_queue.push_front(object);
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                return true;
            }
            //note:caller respond to release object;
            virtual   xdataobj_t*    pop_front()
            {
                if(m_std_queue.empty())
                    return NULL;
                
                xdataobj_t * object = m_std_queue.front();
                m_std_queue.pop_front();
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                return object;
            }
            xdataobj_t* get_front() //caller should not release object after get
            {
                if(m_std_queue.empty())
                    return NULL;
                return m_std_queue.front();
            }
            
            xdataobj_t* get(const uint32_t index)//caller should not release object after get
            {
                if(m_std_queue.empty())
                    return NULL;
                
                if(index < m_std_queue.size())
                    return m_std_queue.at(index);
                
                return NULL;
            }
            
            bool    empty()
            {
                return m_std_queue.empty();
            }
            int32_t size()
            {
                return (int32_t)m_std_queue.size();
            }
            
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override
            {
                if(enum_xdata_type_objptr_deque == _enum_xobject_type_)
                    return this;
                return xdataobj_t::query_interface(_enum_xobject_type_);
            }
        protected: //not safe for multiple threads
            virtual int32_t    do_write(xstream_t & stream) override        //serialize whole object to binary
            {
                const int32_t begin_size = stream.size();
                
                int32_t objects_count = (int32_t)m_std_queue.size();
                stream << objects_count;
                for(std_deque::iterator it = m_std_queue.begin(); it != m_std_queue.end(); ++it)
                {
                    xdataobj_t* object = *it; //must be valid
                    xassert(object != NULL);
                    object->serialize_to(stream);
                }
                
                const int32_t end_size = stream.size();
                return (end_size - begin_size);
            }
            virtual int32_t    do_read(xstream_t & stream) override //serialize from binary and regeneate content of xdataobj_t
            {
                const int32_t begin_size = stream.size();

                int32_t objects_count = 0;
                stream >> objects_count;
                for(int32_t i = 0; i < objects_count; ++i)
                {
                    xdataobj_t * object = xdataobj_t::read_from(stream);
                    if(NULL == object)
                    {
                        xerror("xlist_t<xdataobj_t>,fail to read object(%d of %d) for key(%s)",i,objects_count,m_obj_key.c_str());
                        return enum_xerror_code_bad_data;
                    }
                    m_std_queue.push_back(object);
                }
                const int32_t end_size = stream.size();
                return (begin_size - end_size);
            }
            virtual bool   clear() //relase resource
            {
                if(m_std_queue.empty() == false)
                {
                    for(std_deque::iterator it = m_std_queue.begin(); it != m_std_queue.end(); ++it)
                    {
                        xdataobj_t* object = *it;
                        if(object != NULL)
                            object->release_ref();
                    }
                    m_std_queue.clear();
                }
                return true;
            }
            
            virtual xdataobj_t*  clone() override //note: unsafe at multiple-thread
            {
                return new xdeque_t<xdataobj_t>(*this);
            }
        protected:
            std_deque  m_std_queue;
        };
        typedef xdeque_t<std::string>   xstrdeque_t;
        typedef xdeque_t<xdataobj_t>    xobjdeque_t;
                
        //stand map with serialize function
        template<typename T>
        class xmap_t : public xdataobj_t
        {
        };
                
        template<>
        class xmap_t<std::string> : public xdataobj_t
        {
            //key must be string
        public:
            typedef std::map<std::string,std::string> std_map;
        public:
            enum{enum_obj_type = enum_xdata_type_string_map};
        public:
            xmap_t()
            :xdataobj_t(enum_xdata_type_string_map)
            {
            }
            xmap_t(const std::string & key)
            :xdataobj_t(enum_xdata_type_string_map,key)
            {
            }
        protected:
            xmap_t(enum_xdata_type override_type) //just open for subclass
            :xdataobj_t(override_type)
            {
            }
            virtual ~xmap_t()
            {
                clear();
            }
            xmap_t(const xmap_t & obj) //note: unsafe at multiple-thread
                :xdataobj_t(enum_xdata_type_string_map)
            {
                *this = obj;
            }
            
            xmap_t & operator = (const xmap_t & obj) //note: unsafe at multiple-thread
            {
                m_std_map = obj.m_std_map;
                xdataobj_t::operator = ((xdataobj_t &)obj);
                return *this;
            }
        public: //not safe for multiple threads
            bool find(const std::string & key)
            {
                std_map::iterator it = m_std_map.find(key);
                if(it != m_std_map.end())
                {
                    return true;
                }
                return false;
            }
            bool get(const std::string & key,std::string & value)
            {
                std_map::iterator it = m_std_map.find(key);
                if(it != m_std_map.end())
                {
                    value = it->second;
                    return true;
                }
                return false;
            }
            virtual   void  set(const std::string & key,const std::string & value)
            {
                m_std_map[key] = value;
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
            }
            virtual   void  merge(const xmap_t & obj)
            {
                for(auto it = obj.m_std_map.begin(); it != obj.m_std_map.end(); ++it)
                {
                    m_std_map[it->first] = it->second;
                }
            }
            virtual   bool  remove(const std::string & key)//return true if remove successful
            {
                std_map::iterator it = m_std_map.find(key);
                if(it != m_std_map.end())
                {
                    m_std_map.erase(it);
                    m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                    return true;
                }
                return false;
            }
            bool    empty()
            {
                return m_std_map.empty();
            }
            int32_t size()
            {
                return (int32_t)m_std_map.size();
            }
            virtual bool clear() //relase resource
            {
                if (!empty()) {
                    m_std_map.clear();
                    add_modified_count();
                }
                return true;
            }
            const std::map<std::string, std::string>& get_map() const {
                return m_std_map;
            }
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override
            {
                if(enum_xdata_type_string_map == _enum_xobject_type_)
                    return this;
                return xdataobj_t::query_interface(_enum_xobject_type_);
            }
        protected: //not safe for multiple threads
            virtual int32_t    do_write(xstream_t & stream) override        //serialize whole object to binary
            {
                const int32_t begin_size = stream.size();
                
                int32_t strings_count = (int32_t)m_std_map.size();
                stream << strings_count;
                for(std_map::iterator it = m_std_map.begin(); it != m_std_map.end(); ++it)
                {
                    stream << it->first;
                    stream << it->second;
                }
                const int32_t end_size = stream.size();
                return (end_size - begin_size);
            }
            virtual int32_t    do_read(xstream_t & stream) override //serialize from binary and regeneate content of xdataobj_t
            {
                const int32_t begin_size = stream.size();

                int32_t strings_count = 0;
                stream >> strings_count;
                for(int32_t i = 0; i < strings_count; ++i)
                {
                    std::string key;
                    std::string value;
                    stream >> key;
                    stream >> value;
                    m_std_map[key] = value;
                }
                const int32_t end_size = stream.size();
                return (begin_size - end_size);
            }
            virtual xdataobj_t*  clone() override //note: unsafe at multiple-thread
            {
                return new xmap_t<std::string>(*this);
            }
        protected:
            std_map  m_std_map;
        };
        
        template<>
        class xmap_t<xdataobj_t> : public xdataobj_t
        {
            //key must be string
            typedef std::map<std::string,xdataobj_t*> std_map;
        public:
            enum{enum_obj_type = enum_xdata_type_objptr_map};
        public:
            xmap_t()
            :xdataobj_t(enum_xdata_type_objptr_map)
            {
            }
            xmap_t(const std::string & key)
            :xdataobj_t(enum_xdata_type_objptr_map,key)
            {
            }
        protected:
            xmap_t(enum_xdata_type override_type) //just open for subclass
            :xdataobj_t(override_type)
            {
            }
            virtual ~xmap_t()
            {
                clear();
            }
            xmap_t(const xmap_t & obj) //note: unsafe at multiple-thread
                :xdataobj_t(enum_xdata_type_objptr_map)
            {
                *this = obj;
            }
            xmap_t & operator = (const xmap_t & obj) //note: unsafe at multiple-thread
            {
                m_std_map = obj.m_std_map;
                xdataobj_t::operator = ((xdataobj_t &)obj);
                return *this;
            }
        public: //not safe for multiple threads
            xdataobj_t* get(const std::string & key)//note:caller should not release object after get
            {
                std_map::iterator it = m_std_map.find(key);
                if(it != m_std_map.end())
                    return it->second;
                
                return NULL;
            }
            virtual   void  set(const std::string & key,xdataobj_t* value)
            {
                if(value != NULL)
                    value->add_ref();
                
                xdataobj_t * old = m_std_map[key];
                m_std_map[key] = value;
                if(old != NULL)
                    old->release_ref();
                
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
            }
            virtual   bool  remove(const std::string & key)//return true if remove successful
            {
                std_map::iterator it = m_std_map.find(key);
                if(it != m_std_map.end())
                {
                    if(it->second != NULL)
                        it->second->release_ref();
                    
                    m_std_map.erase(it);
                    m_modified_count = m_modified_count + 1; //atom is not need as it always increase
                    return true;
                }
                return false;
            }
            bool    empty()
            {
                return m_std_map.empty();
            }
            int32_t size()
            {
                return (int32_t)m_std_map.size();
            }
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override
            {
                if(enum_xdata_type_objptr_map == _enum_xobject_type_)
                    return this;
                return xdataobj_t::query_interface(_enum_xobject_type_);
            }
        protected: //not safe for multiple threads
            virtual int32_t    do_write(xstream_t & stream) override        //serialize whole object to binary
            {
                const int32_t begin_size = stream.size();
                
                int32_t objects_count = (int32_t)m_std_map.size();
                stream << objects_count;
                for(std_map::iterator it = m_std_map.begin(); it != m_std_map.end(); ++it)
                {
                    stream << it->first;
                    stream << it->second->serialize_to(stream);
                }
                const int32_t end_size = stream.size();
                return (end_size - begin_size);
            }
            virtual int32_t    do_read(xstream_t & stream) override //serialize from binary and regeneate content of xdataobj_t
            {
                const int32_t begin_size = stream.size();
                
                int32_t objects_count = 0;
                stream >> objects_count;
                for(int32_t i = 0; i < objects_count; ++i)
                {
                    std::string key;
                    stream >> key;
                    xdataobj_t * object = xdataobj_t::read_from(stream);
                    if(NULL == object)
                    {
                        xerror("xmap<xdataobj_t>,fail to read object(%d of %d) for key(%s)",i,objects_count,m_obj_key.c_str());
                        return enum_xerror_code_bad_data;
                    }
                    m_std_map[key] = object;//keep reference
                }
                const int32_t end_size = stream.size();
                return (begin_size - end_size);
            }
            virtual bool   clear() //release resource
            {
                if(m_std_map.empty() == false)
                {
                    for(std_map::iterator it = m_std_map.begin(); it != m_std_map.end(); ++it)
                    {
                        xdataobj_t* object = it->second;
                        if(object != NULL)
                            object->release_ref();
                    }
                    m_std_map.clear();
                }
                return true;
            }
            
            virtual xdataobj_t*  clone() override //note: unsafe at multiple-thread
            {
                return new xmap_t<xdataobj_t>(*this);
            }
        protected:
            std_map  m_std_map;
        };
        typedef xmap_t<std::string>  xstrmap_t;
        typedef xmap_t<xdataobj_t>   xobjmap_t;
        
        ///////////////////////consenus message/pdu //////////////////
        //general consensus pdu
        class xcspdu_t : public xdatapdu_t
        {
        public:
            xcspdu_t(enum_xpdu_type type);
            xcspdu_t(const xcspdu_t & obj);
            virtual ~xcspdu_t();
        protected:
            xcspdu_t & operator = (const xcspdu_t & obj);
        private:
            xcspdu_t();
        public:
            //note:return reference as performance consideration
            const uint64_t      get_block_viewid()   const  {return get_session_id();}
            const uint32_t      get_block_viewtoken()const  {return get_session_key();}
            
            const uint64_t      get_block_clock()   const  {return m_block_clock;}
            const uint64_t      get_block_height()  const  {return m_block_height;}
            const uint32_t      get_block_chainid() const  {return m_block_chainid;} //which chain(netid) m_block_account belong to
            const std::string & get_block_account()      const  {return m_block_account;}
            const uint32_t      get_block_account_hash() const  {return m_block_account_hash;}
            
            const std::string&  get_xclock_cert()        const  {return m_xclock_cert;}
            const std::string&  get_vblock_cert()        const  {return m_vblock_cert;}
        public:
            void                set_block_viewid(const uint64_t viewid)       { m_session_id  = viewid;}
            void                set_block_viewtoken(const uint32_t viewtoken) { m_session_key = viewtoken;}
            void                set_block_clock(const uint64_t clock)   { m_block_clock  = clock;}
            void                set_block_height(const uint64_t height) { m_block_height = height;}
            void                set_block_chainid(const uint32_t chainid){ m_block_chainid  = chainid;}
            void                set_block_account(const std::string & account);//may recalcuate m_consensus_account_hash
            void                set_xclock_cert(const std::string & clock_cert){m_xclock_cert = clock_cert;}
            void                set_vblock_cert(const std::string & block_cert){m_vblock_cert = block_cert;}
        
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override;
            virtual std::string dump() const override;
        protected:
            //return how many bytes readout /writed in, return < 0(enum_xerror_code_type) when have error
            virtual int32_t     do_write(base::xstream_t & stream) override;    //write whole object to binary
            virtual int32_t     do_read(base::xstream_t & stream) override;   //read from binary and regeneate content of xdataobj_t
        private: //private mode to avoid confict with consensus_event
            //note: here block is a virtual concept of container,could be chain-block or anything else
            uint64_t            m_block_clock;      //global time clock ' height
            uint64_t            m_block_height;     //like consensus block height if have
            uint32_t            m_block_chainid;    //which chain(24bit network-id) m_block_account belong to
            uint32_t            m_block_account_hash; //just for performance purpose to avoid mutilple time hash(m_consensus_account)
            std::string         m_block_account;    //like consenus object, order by order
        private:
            std::string         m_vblock_cert;      //highest qc certificate for this pdu,it might be nil
            std::string         m_xclock_cert;      //clock block'certification to proove that is leader
        };
 
        class xbftpdu_t : public xcspdu_t
        {
        public:
            enum{enum_obj_type = enum_xpdu_type_consensus_xbft};
        public:
            xbftpdu_t();//go default case,and object_type will be enum_xpdu_type_consensus_xbft
            xbftpdu_t(const xcspdu_t & obj);
            xbftpdu_t(const xbftpdu_t & obj);
            virtual ~xbftpdu_t();
        private:
            xbftpdu_t & operator = (const xbftpdu_t &);
        };
                
    }//end of namespace base
} //end of namespace top
