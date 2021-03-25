// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <deque>
#include <map>
#include "xdata.h"

namespace top
{
    namespace base
    {
        //strong type for int
        enum vint8_t  : int8_t;
        enum vint16_t : int16_t;
        enum vint32_t : int32_t;
        enum vtoken_t : int64_t;
        enum vnonce_t : uint64_t;
    
        class xvmethod_t;
        typedef uint256_t*      xuint256_ptr;
        typedef xdataunit_t*    xdataunit_ptr;
        typedef xvmethod_t*     xvmethod_ptr;
    
        //general rule : keep xvalue be readonly once construct
        class xvalue_t final
        {
        public:
            enum enum_xvalue_type
            {
                enum_xvalue_type_null       = 0x00, //empty
                enum_xvalue_type_error      = 0x01, //error#,refer enum_xerror_code
                enum_xvalue_type_bool       = 0x02, //bool
                enum_xvalue_type_int8       = 0x03, //int8
                enum_xvalue_type_int16      = 0x04, //int16
                enum_xvalue_type_int32      = 0x05, //int32
                enum_xvalue_type_int64      = 0x06, //int64...
                enum_xvalue_type_uint64     = 0x07, //uint64...
                enum_xvalue_type_token      = 0x08, //token balance of int64
                enum_xvalue_type_nonce      = 0x09, //nonce of uint64
                //reservered for other small value type
                enum_xvalue_type_uint256    = 0x0c, //uint256..
                enum_xvalue_type_vmethod    = 0x0d, //carry xvoperate object->indirectly to evalaute value
                enum_xvalue_type_small_value= 0x0d, //small data or fixed size
                enum_xvalue_type_xobject    = 0x0e, //xdataunit_t*
                enum_xvalue_type_string     = 0x0f, //std::string
                //above are basic type
                
                //compount(container) value
                enum_xvalue_type_vector     = 0x10, //std::vector<std::string>
                enum_xvalue_type_deque      = 0x20, //std::deque<std::string>
                enum_xvalue_type_map        = 0x30, //std::map<std::string,int64>
                enum_xvalue_type_hashmap    = 0x40, //std::map<std::string,std::map<std::string,std::string> >
                
                enum_xvalue_type_max        = 0x7F, //7bit,NEVER over this max value
            };
            
        public:
            inline const uint8_t      get_type()     const {return (value_type & 0x7F);}//highest bit is encode type
            
            inline enum_xerror_code   get_error()    const {return (enum_xvalue_type_error == get_type())   ? enum_xerror_code(int64_val) : enum_xerror_code_fail;}
            inline const bool         get_bool()     const {return (enum_xvalue_type_bool  == get_type())   ? (int64_val != 0) : false;}
            inline const int8_t       get_int8()     const {return (enum_xvalue_type_int8  == get_type())   ? (int8_t)int64_val  : 0;}
            inline const int16_t      get_int16()    const {return (enum_xvalue_type_int16 == get_type())   ? (int16_t)int64_val : 0;}
            inline const int32_t      get_int32()    const {return (enum_xvalue_type_int32 == get_type())   ? (int32_t)int64_val : 0;}
            inline const int64_t      get_int64()    const {return (enum_xvalue_type_int64 == get_type())   ? int64_val  : 0;}
            inline const uint64_t     get_uint64()   const {return (enum_xvalue_type_uint64 == get_type())  ? uint64_val : 0;}
            inline const vtoken_t     get_token()    const {return (enum_xvalue_type_token == get_type())   ? vtoken_t(int64_val) : vtoken_t(0) ;}
            inline const vnonce_t     get_nonce()    const {return (enum_xvalue_type_nonce == get_type())   ? vnonce_t(uint64_val) : vnonce_t(0);}
            inline const std::string& get_string()   const {return string_val;} //string_val has dedicated space
            
            inline const xuint256_ptr  get_uint256()  const {return (enum_xvalue_type_uint256 == get_type()) ? uint256_ptr : nullptr;}
            inline const xvmethod_ptr  get_vmethod()  const {return (enum_xvalue_type_vmethod == get_type()) ? vcall_ptr :  nullptr;}
            inline const xdataunit_ptr get_object()   const {return (enum_xvalue_type_xobject == get_type()) ? object_ptr :  nullptr;}
 
            inline const std::map<std::string,std::map<std::string,std::string> >* const  get_hashmap() const  {return (enum_xvalue_type_hashmap == get_type()) ? hashmap_val : nullptr;}
            
        public: //read data value of basic types
            template<typename T>
            const T               get_value() const;

        public: //get vector
            template<typename T>
            const std::vector<T>* get_vector() const;
            
        public: //read deque
            template<typename T>
            const std::deque<T>*  get_deque() const;
            
        public: //read map
            template<typename T>
            const std::map<std::string,T>* get_map() const;
            
        public: //query basic type by template specialize
            template<typename T>
            static const int     query_type();

        public:
            xvalue_t();
            xvalue_t(const bool val);
            xvalue_t(const enum_xerror_code val);
            xvalue_t(const vint8_t val);
            xvalue_t(const vint16_t val);
            xvalue_t(const vint32_t val);
            xvalue_t(const int8_t val);
            xvalue_t(const int16_t val);
            xvalue_t(const int32_t val);
            xvalue_t(const int64_t val);
            xvalue_t(const uint64_t val);
            xvalue_t(const vtoken_t val);
            xvalue_t(const vnonce_t val);
            xvalue_t(const std::string & _strval);
            
            xvalue_t(const uint256_t & val);
            xvalue_t(const xvmethod_t & val);
            xvalue_t(const xdataunit_t * _objval);
            xvalue_t(const std::map<std::string,std::map<std::string,std::string> > & val);
            
        public: //vector contruction
            xvalue_t(const std::vector<int8_t>& val);
            xvalue_t(const std::vector<int16_t>& val);
            xvalue_t(const std::vector<int32_t>& val);
            xvalue_t(const std::vector<int64_t>& val);
            xvalue_t(const std::vector<uint64_t>& val);
            xvalue_t(const std::vector<std::string>& val);
            
        public: //deque contruction
            xvalue_t(const std::deque<int8_t>& val);
            xvalue_t(const std::deque<int16_t>& val);
            xvalue_t(const std::deque<int32_t>& val);
            xvalue_t(const std::deque<int64_t>& val);
            xvalue_t(const std::deque<uint64_t>& val);
            xvalue_t(const std::deque<std::string>& val);
            
        public: //map contruction
            xvalue_t(const std::map<std::string,int8_t>& val);
            xvalue_t(const std::map<std::string,int16_t>& val);
            xvalue_t(const std::map<std::string,int32_t>& val);
            xvalue_t(const std::map<std::string,int64_t>& val);
            xvalue_t(const std::map<std::string,uint64_t>& val);
            xvalue_t(const std::map<std::string,std::string>& val);
            
        public:
            xvalue_t(const xvalue_t & obj);
            //note:use carefully for move operation, which may break rule: never change xvalue_t once construction
            xvalue_t(xvalue_t && obj);
            xvalue_t & operator = (const xvalue_t & right);
            
            ~xvalue_t();
        public:
            int32_t     serialize_to(xstream_t & stream);        //serialize header and object,return how many bytes is writed
            int32_t     serialize_from(xstream_t & stream);      //serialize header and object,return how many bytes is readed
            
        private:
            void         close();
            void         copy_from(const xvalue_t & right);
            void         move_from(xvalue_t & right);
            //internal write & read operation
            int32_t      do_write(xstream_t & stream);
            int32_t      do_read(xstream_t & stream);
            
            inline void* get_anyptr()   const {return any_ptr;}
        private:
            union
            {
                int64_t         int64_val;
                uint64_t        uint64_val;
                void*           any_ptr;
                xdataunit_t*    object_ptr;
                uint256_t*      uint256_ptr;
                xvmethod_t*     vcall_ptr;
                std::map<std::string,std::map<std::string,std::string> > *  hashmap_val;
            };
            std::string       string_val; //string is very common,give dedicated space to increase performance
            uint8_t           value_type; //[1bit:encode type][3bit:container type][4bit:basic data type]
            
        private: //help functions
            template<typename T>
            class vector_utl
            {
            public:
                static std::vector<T>* copy_from(const xvalue_t & var)//assume xvalue_t is vector type
                {
                    xassert( (var.get_type() & 0xF0) == enum_xvalue_type_vector);
                    const std::vector<T> * vector_ptr = var.get_vector<T>();
                    xassert(vector_ptr != nullptr);
                    if(vector_ptr != nullptr)
                        return  new std::vector<T>(*var.get_vector<T>());
         
                    return  nullptr;
                }
                
                static bool write(const xvalue_t & var,xstream_t & stream)
                {
                    xassert( (var.get_type() & 0xF0) == enum_xvalue_type_vector);
                    const std::vector<T> * vector_ptr = var.get_vector<T>();
                    xassert(vector_ptr != nullptr);
                    if(vector_ptr != nullptr)
                    {
                        if(stream.write_compact_vector(*vector_ptr))
                            return true;
                    }
                    return false;
                }
                
                static std::vector<T>* read(xstream_t & stream)
                {
                    std::vector<T> * vectorval = new std::vector<T>();
                    stream.read_compact_vector(*vectorval);
                    return vectorval;
                }
                
                static bool delete_ptr(const xvalue_t & var)
                {
                    const std::vector<T> * vector_ptr = var.get_vector<T>();
                    xassert(vector_ptr != nullptr);
                    if(vector_ptr != nullptr)
                    {
                        delete vector_ptr;
                        return true;
                    }
                    return false;
                }
            };
            
            template<typename T>
            class deque_utl
            {
            public:
                static std::deque<T>* copy_from(const xvalue_t & var)//assume xvalue_t is vector type
                {
                    xassert( (var.get_type() & 0xF0) == enum_xvalue_type_deque);
                    const std::deque<T> * deque_ptr = var.get_deque<T>();
                    xassert(deque_ptr != nullptr);
                    if(deque_ptr != nullptr)
                        return  new std::deque<T>(*deque_ptr);
                    else
                        return  nullptr;
                }
                
                static bool write(const xvalue_t & var,xstream_t & stream)
                {
                    xassert( (var.get_type() & 0xF0) == enum_xvalue_type_deque);
                    const std::deque<T> * deque_ptr = var.get_deque<T>();
                    xassert(deque_ptr != nullptr);
                    if(deque_ptr != nullptr)
                    {
                        if(stream.write_compact_deque(*deque_ptr))
                            return true;
                    }
                    return false;
                }
                
                static std::deque<T>* read(xstream_t & stream)
                {
                    std::deque<T> * dequeval = new std::deque<T>();
                    stream.read_compact_deque(*dequeval);
                    return dequeval;
                }
                
                static bool delete_ptr(xvalue_t & var)
                {
                    const std::deque<T> * deque_ptr = var.get_deque<T>();
                    xassert(deque_ptr != nullptr);
                    if(deque_ptr != nullptr)
                    {
                        delete deque_ptr;
                        return true;
                    }
                    return false;
                }
            };
            
            template<typename T>
            class map_utl
            {
            public:
                static std::map<std::string,T>* copy_from(const xvalue_t & var)//assume xvalue_t is map type
                {
                    xassert( (var.get_type() & 0xF0) == enum_xvalue_type_map);
                    const std::map<std::string,T>* map_ptr = var.get_map<T>();
                    xassert(map_ptr != nullptr);
                    if(map_ptr != nullptr)
                        return  new std::map<std::string,T>(*map_ptr);
                        
                    return nullptr;
                }
                
                static bool write(const xvalue_t & var,xstream_t & stream)
                {
                    xassert( (var.get_type() & 0xF0) == enum_xvalue_type_map);
                    const std::map<std::string,T>* map_ptr = var.get_map<T>();
                    xassert(map_ptr != nullptr);
                    if(map_ptr != nullptr)
                    {
                        if(stream.write_compact_map(*map_ptr) > 0)
                            return true;
                    }
                    return false;
                }
                
                static std::map<std::string,T>* read(xstream_t & stream)
                {
                    std::map<std::string,T> * mapval = new std::map<std::string,T>();
                    stream.read_compact_map(*mapval);
                    return mapval;
                }
                
                static bool delete_ptr(xvalue_t & var)
                {
                    const std::map<std::string,T>* map_ptr = var.get_map<T>();
                    xassert(map_ptr != nullptr);
                    if(map_ptr != nullptr)
                    {
                        delete map_ptr;
                        return true;
                    }
                    return false;
                }
            };
        };
        
        //xvalueobj_t is the wrap of xvalue_t but with reference control
        class xvalueobj_t final : public xobject_t
        {
            friend class xvexeunit_t;
            friend class xvproperty_t;
            typedef xobject_t  base;
            enum
            {
                enum_xvalue_flag_readonly    = 0x10, //value is readonly
                enum_xvalue_flag_shared      = 0x20, //value are shared by mutiple owner
            };
        public:
            static  const std::string   name(){ return std::string("xvalueobj");}
            virtual std::string         get_obj_name() const override {return name();}
            enum{enum_obj_type = enum_xobject_type_valueobj};//allow xbase create xvproperty_t object from xdataobj_t::read_from()
            
        public:
            xvalueobj_t(const std::string & name,const xvalue_t & value);
            xvalueobj_t(const xvalueobj_t & obj);
            
        protected: //just open for friend class or subclass
            xvalueobj_t();
            ~xvalueobj_t();
            
        private://dont implement those construction
            xvalueobj_t(xvalueobj_t &&);
            xvalueobj_t & operator = (const xvalueobj_t & other);
            
        public:
            const std::string &        get_name() const {return m_name;} //property name
            const xvalue_t&            get_value()const {return m_value;}//property value
            
            //just tracking at runtime(at memory)
            inline  bool   is_readonly() const {return check_obj_flag(enum_xvalue_flag_readonly);}
            inline  bool   is_shared()   const {return check_obj_flag(enum_xvalue_flag_shared);}
      
            virtual void*  query_interface(const int32_t _enum_xobject_type_) override;
        public:
            int32_t     serialize_to(xstream_t & stream);        //serialize header and object,return how many bytes is writed
            int32_t     serialize_from(xstream_t & stream);      //serialize header and object,return how many bytes is readed
            
        private:
            //update value,not safe for multiple_thrad
            bool         copy_from_value(const xvalue_t & new_val);
            bool         move_from_value(xvalue_t & new_val);
            
            void         set_name(const std::string & new_name){m_name = new_name;}
            void         set_readonly_flag() {set_obj_flag(enum_xvalue_flag_readonly);}//not allow modify anymore after it
            void         set_shared_flag() {set_obj_flag(enum_xvalue_flag_shared);}//not allow modify anymore after it
        private:
            std::string  m_name;
            xvalue_t     m_value;
        };
    
        //generally present a virtual method at data level
        class xvmethod_t
        {
            enum : uint8_t
            {
                enum_op_params_max_count             = 3,
                
                enum_op_call_method_by_name          = 1 << 6,    //call by method_name or method_id
                enum_op_call_method_by_target        = 1 << 7,    //carry target as long call
            };
        public:
            xvmethod_t(); //construct nil op
            
            //member'method_id related construction
            xvmethod_t(const std::string & target_uri,const uint8_t code,const int8_t method_id);
            xvmethod_t(const std::string & target_uri,const uint8_t code,const int8_t method_id,xvalue_t & param);
            xvmethod_t(const std::string & target_uri,const uint8_t code,const int8_t method_id,xvalue_t & param1,xvalue_t & param2);
            xvmethod_t(const std::string & target_uri,const uint8_t code,const int8_t method_id,xvalue_t & param1,xvalue_t & param2,xvalue_t & param3);
            
            //member'method_name related construction
            xvmethod_t(const std::string & target_uri,const uint8_t code,const std::string & method_name);
            xvmethod_t(const std::string & target_uri,const uint8_t code,const std::string & method_name,xvalue_t & param);
            xvmethod_t(const std::string & target_uri,const uint8_t code,const std::string & method_name,xvalue_t & param1,xvalue_t & param2);
            xvmethod_t(const std::string & target_uri,const uint8_t code,const std::string & method_name,xvalue_t & param1,xvalue_t & param2,xvalue_t & param3);
            
            //copy & move & assign construction
            xvmethod_t(xvmethod_t && obj);
            xvmethod_t(const xvmethod_t & obj);
            xvmethod_t(const xvmethod_t & obj,const std::string & new_target_uri);//clone and modify to new target uri
            xvmethod_t & operator = (const xvmethod_t & right);
            
            ~xvmethod_t();
        public:
            inline const std::string&           get_method_uri()   const {return m_method_uri;}
            inline const int                    get_method_type()  const {return (m_op_code & 0x0F);}
            inline const std::string&           get_method_name()  const {return m_method_name;}
            inline const int8_t                 get_method_id()    const {return m_method_id;}//gurantee less than INT8_MAX
            
            inline const  int                   get_params_count() const {return ((m_op_code & 0x30) >> 4);}
            inline const std::deque<xvalue_t>&  get_method_params()const {return m_method_params;}
            
            //indicate whether carry target inside
            inline bool                         has_method_uri()   const {return ((m_op_code & enum_op_call_method_by_target)  != 0);}
            //indicate wheter to use method_name to find meethod
            inline bool                         is_name_method()   const {return ((m_op_code & enum_op_call_method_by_name) != 0);}
            inline bool                         is_id_method()     const {return !is_name_method();}
            
        public:
            int32_t         serialize_to(xstream_t & stream);        //serialize header and object,return how many bytes is writed
            int32_t         serialize_from(xstream_t & stream);      //serialize header and object,return how many bytes is readed
            
        private:
            void            close();
            int32_t         do_write(xstream_t & stream);     //serialize header and object,return how many bytes is writed
            int32_t         do_read(xstream_t & stream);      //serialize header and object,return how many bytes is readed
        private:
            //m_method_uri might be like [account_address].[blockheight].[property_name]
            std::string           m_method_uri;   //uniform execution resource identify
            std::string           m_method_name;  //method name of target
            std::deque<xvalue_t>  m_method_params;//method'parameters
            int8_t                m_method_id;    //predefined method id
            uint8_t               m_op_code;      //[1bit:long-call with target][1bit:call_by_name][highest 2bit:count of pararms][low 4bit:type]
        };
    
    };
};
