// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include <string>
#include <list>
#include <deque>
#include <map>
#include <set>
#include "xobject.h"
#include "xmem.h"

namespace top
{
    namespace base
    {        
        //total 16 flags as 16bit
        enum enum_xdata_flag
        {
            enum_xdata_flag_compressed = 0x10,  //raw data compressed by lz4,note: only valid compress method is xcompress_t
            enum_xdata_flag_encrypted  = 0x20,  //raw data is encyprted by subclass,note: strong recommend to use xaes_t
            enum_xdata_flag_fragment   = 0x40,  //raw data are fragment,which means this object just present one fragment
        };
        
        //xdatapdu_t used for packet communication between nodes or present property of object
        class xdataunit_t : public xobject_t
        {
        public:
            enum enum_xdata_type
            {
                enum_xdata_type_min = enum_xobject_type_data_min,
                enum_xdata_type_unknow          = enum_xdata_type_min + 0,//for any unknow type but still following same spec of xdata_header_t
                
                enum_xdata_type_string          = enum_xdata_type_min + 1,
                enum_xdata_type_string_deque    = enum_xdata_type_min + 2,
                enum_xdata_type_objptr_deque    = enum_xdata_type_min + 3,
                
                enum_xdata_type_string_map      = enum_xdata_type_min + 4,
                enum_xdata_type_objptr_map      = enum_xdata_type_min + 5,
                
                enum_xdata_type_set             = enum_xdata_type_min + 8,
                
                enum_xdata_type_max = enum_xobject_type_data_max - 1,
            };
        private:
            //unify layout for data serialization:
            enum { enum_xpdu_header_length = 6};
            struct xpdu_header_t
            {
                int32_t   obj_length;       //the length(max 24bit) of the whole data,note:high8bit is reserved as 0
                int16_t   obj_type;         //data object type
                uint8_t   body_raw[];       //raw data
            }_ALIGNPACKED_1;
        public:
            static xdataunit_t * read_from(xstream_t & stream);
        protected:
            xdataunit_t(enum_xdata_type type);
            virtual ~xdataunit_t();
        private:
            xdataunit_t();
            xdataunit_t(const xdataunit_t &);
            xdataunit_t & operator = (const xdataunit_t &);
        public:
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override;
            
        public://not safe for multiple threads,serialize_to/from write and read addtion head of dataobj
            virtual int32_t     serialize_to(xstream_t & stream);        //serialize header and object,return how many bytes is writed
            virtual int32_t     serialize_from(xstream_t & stream);      //serialize header and object,return how many bytes is readed
            
        public://not safe for multiple threads, do_write & do_read write and read raw data of dataobj
            //return how many bytes readout /writed in, return < 0(enum_xerror_code_type) when have error
            virtual int32_t     do_write(xstream_t & stream) = 0;     //write whole object to binary
            virtual int32_t     do_read(xstream_t & stream) = 0;      //read from binary and regeneate content of xdataobj_t
        protected:
            int32_t             m_obj_length;       //the length(max 24bit) of the whole data,note:high8bit is reserved as 0
            int16_t             m_ob_type;          //just used for alignment of memory layout
        };
        
        //Key-Value data object for cache/db system
        class xdataobj_t : public xdataunit_t
        {
        protected:
            //unify layout for data serialization:
            enum { enum_xdata_header_length = 32};
            struct xdata_header_t
            {
                int16_t   obj_type;         //data object type
                uint16_t  obj_flags;        //data object flags
                int32_t   obj_length;       //the length(max 24bit) of the whole data,note:high8bit is reserved as 0
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
        private:
            xdataobj_t();
            xdataobj_t(const xdataobj_t &);
            xdataobj_t & operator = (const xdataobj_t &);
        public:
            inline   uint16_t            get_obj_flags()     const {return m_obj_flags;}
            inline   uint32_t            get_obj_hash()      const {return m_obj_hash;}
            inline   std::string         get_key()           const {return m_obj_key;}
            inline   uint32_t            get_obj_version()   const {return m_obj_version;}
            inline   uint64_t            get_obj_time()      const {return m_obj_time;}
            inline   uint32_t            get_obj_last_hash() const {return m_obj_last_hash;}
            inline   uint32_t            get_obj_expire()    const {return m_obj_expire;}       //0 means never expire
            
            inline   uint32_t            get_cache_expire()  const {return m_cache_expire;}   //0 means never expire
            inline   uint64_t            get_last_access_time() const {return m_last_access_time;} //0 means never accessed yet
            inline   uint32_t            get_modified_count()const {return m_modified_count;}
            
            //caller respond to cast (void*) to related  interface ptr
            virtual void*                query_interface(const int32_t _enum_xobject_type_) override;
            
        public://allow to change them directly
            inline  void                 add_obj_flag(enum_xdata_flag flag){ m_obj_flags|= flag;}
            inline  void                 remove_obj_flag(enum_xdata_flag flag){ m_obj_flags &= (~flag);}
            inline  void                 reset_obj_flags(const uint16_t new_flags) {m_obj_flags = new_flags;}
            inline  void                 set_obj_expire(const uint32_t time_to_expire){m_obj_expire = time_to_expire;}

            inline  void                 set_cache_expire(const uint32_t time_to_expire){m_cache_expire = time_to_expire;}
            inline  void                 set_last_access_time(const uint64_t access_time){m_last_access_time = access_time;}
            uint32_t                     add_modified_count(bool force_atomic_change = false);
            
        public://not safe for multiple threads,serialize_to/from write and read addtion head of dataobj
            virtual int32_t              serialize_to(xstream_t & stream) override;        //serialize header and object,return how many bytes is writed
            virtual int32_t              serialize_from(xstream_t & stream) override;      //serialize header and object,return how many bytes is readed
            
        protected: //serialized from/to db
            uint16_t            m_obj_flags;        //data object flags
            uint32_t            m_obj_hash;         //32bit of xhash64(body_raw)
            
            uint32_t            m_obj_last_hash;    //point to last m_obj_hash
            uint32_t            m_obj_version;      //version code of change for object
            uint32_t            m_obj_expire;       //expire time, seconds from one reference time base,never expire if 0
            uint64_t            m_obj_time;         //the time(UTC,ms) when generate obj_hash
            std::string         m_obj_key;          //Object ID/Key,//must have and serialize in &out,  but allow it is empty
        protected:
            uint64_t            m_last_access_time;     //(not serialized),the last GMT time for write & read
            uint32_t            m_cache_expire;         //(not serialized) remove from cache but not delete from db.seconds from one reference time base(2010/01/01/00:00:00),0 means never expire
            uint32_t            m_modified_count;       // (not serialized),count how many times modified since last save,it is 0 if nothing changed
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
            xunknowobj_t()
                :xdataobj_t(enum_xdata_type_unknow)
            {
            }
            xunknowobj_t(const std::string & key)
                :xdataobj_t(enum_xdata_type_unknow,key)
            {
            }
        protected:
            virtual ~xunknowobj_t()
            {
            };
        private:
            xunknowobj_t(const xunknowobj_t &);
            xunknowobj_t & operator = (const xunknowobj_t &);
        public:
            std::string   get() const {return m_unknow_content;}
            bool          get(std::string & value)
            {
                value = m_unknow_content;
                return (m_unknow_content.empty() == false);
            }
            virtual  void set(const std::string & value)
            {
                m_unknow_content = value;
                m_modified_count = m_modified_count + 1; //atom is not need as it always increase
            }
        protected: //not safe for multiple threads
            virtual int32_t    do_write(xstream_t & stream) override        //serialize whole object to binary
            {
                const int32_t begin_size = stream.size();
                
                stream.push_back((uint8_t*)m_unknow_content.data(),(int)m_unknow_content.size());
                const int32_t end_size = stream.size();
                return (end_size - begin_size);
            }
                
            virtual int32_t    do_read(xstream_t & stream) override //serialize from binary and regeneate content of xdataobj_t
            {
                const int32_t begin_size = stream.size();
                
                m_unknow_content.clear();
                m_unknow_content.assign((const char*)stream.data(),begin_size);
                stream.pop_front(begin_size);
                
                const int32_t end_size = stream.size();
                return (begin_size - end_size);
            }
            virtual bool   clear() //relase resource
            {
                m_unknow_content.clear();
                return true;
            }
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
        private:
            xstring_t(const xstring_t &);
            xstring_t & operator = (const xstring_t &);
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
            virtual bool   clear() //relase resource
            {
                m_std_string.clear();
                return true;
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
        private:
            xdeque_t(const xdeque_t &);
            xdeque_t & operator = (const xdeque_t &);
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
            virtual bool   clear() //relase resource
            {
                m_std_queue.clear();
                return true;
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
        private:
            xdeque_t(const xdeque_t &);
            xdeque_t & operator = (const xdeque_t &);
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
        private:
            xmap_t(const xmap_t &);
            xmap_t & operator = (const xmap_t &);
        public: //not safe for multiple threads
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
            virtual bool   clear() //relase resource
            {
                m_std_map.clear();
                return true;
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
        private:
            xmap_t(const xmap_t &);
            xmap_t & operator = (const xmap_t &);
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
            virtual bool   clear() //relase resource
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
        protected:
            std_map  m_std_map;
        };
        typedef xmap_t<std::string>  xstrmap_t;
        typedef xmap_t<xdataobj_t>   xobjmap_t;
                
    };//end of namespace base
}; //end of namespace top
