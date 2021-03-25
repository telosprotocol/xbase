// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xvexeunit.h"
#include "xvinstruction.h"
#include "xcontext.h"

namespace top
{
    namespace base
    {
        //xvproperty_t
        //note: dont guarentee safe at multiple thread
        class xvproperty_t : public xvexeunit_t
        {
            friend class xvbstate_t;
        public:
            static  const std::string   name(){ return std::string("xvproperty");}
            virtual std::string         get_obj_name() const override {return name();}
        private:
            static  void register_object(xcontext_t & context);
            enum{enum_obj_type = enum_xobject_type_vproperty};//allow xbase create xvproperty_t object from xdataobj_t::read_from()

        protected:
            xvproperty_t(enum_xdata_type type = (enum_xdata_type)enum_xobject_type_vproperty);
            xvproperty_t(const std::string & name,const xvalue_t & value,enum_xdata_type type = (enum_xdata_type)enum_xobject_type_vproperty);
            xvproperty_t(const xvproperty_t & obj);
            virtual ~xvproperty_t();
            
        private://dont implement those construction
            xvproperty_t(xvproperty_t &&);
            xvproperty_t & operator = (const xvproperty_t & other);
            
        public:
            virtual void*       query_interface(const int32_t _enum_xobject_type_) override;//caller need to cast (void*) to related ptr
            virtual void        set_unit_name(const std::string & name) override;
            const std::string & get_name() const {return get_unit_name();} //property name
            
        protected:
            const xvalue_t&     get_value() const; //readonly access
            const xvalue_t&     get_writable_value();//writeable access
            
            //update value,not safe for multiple_thread
            bool         copy_from_value(const xvalue_t & new_val);
            bool         move_from_value(xvalue_t & new_val);

        protected:
            //return how many bytes readout /writed in, return < 0(enum_xerror_code_type) when have error
            virtual int32_t    do_write(xstream_t & stream) override; //allow subclass extend behavior
            virtual int32_t    do_read(xstream_t & stream)  override; //allow subclass extend behavior
            
        private:
            virtual xvalueobj_t*  load_value_obj() const; //trigger on-demand load if need
            bool          set_value_obj(xvalueobj_t * new_val_obj);
        private:
            xvalueobj_t*  m_value_ptr;//where state stored
        };
    
        //main token(TOP Token) of chain
        class xtokenvar_t : public xvproperty_t
        {
            friend class xvbstate_t;
        public:
            static  const std::string   name(){ return std::string("xtokenvar");}
            virtual std::string         get_obj_name() const override {return name();}
        private:
            static  void register_object(xcontext_t & context);
            enum{enum_obj_type = enum_xobject_type_vprop_token};//allow xbase create xvstate_t object from xdataobj_t::read_from()

        protected:
            xtokenvar_t();
            xtokenvar_t(const std::string & property_name,const vtoken_t property_value);
            xtokenvar_t(const xtokenvar_t & obj);
            virtual ~xtokenvar_t();
            
        private://dont implement those construction
            xtokenvar_t(xtokenvar_t &&);
            xtokenvar_t & operator = (const xtokenvar_t & other);
            
        public:
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override;//caller need to cast (void*) to related ptr
            virtual xvexeunit_t*  clone() override; //cone a new object with same state

        public:  //read interface
             const vtoken_t get_balance();
            
        public: //write interface
            const vtoken_t  deposit(const vtoken_t add_token);//return the updated balance
            const vtoken_t  withdraw(const vtoken_t sub_token);//return the updated balance
    
        private: //internal instruction functions
            const xvalue_t  do_deposit(const xvmethod_t & op);
            const xvalue_t  do_withdraw(const xvmethod_t & op);
            const xvalue_t  do_query(const xvmethod_t & op);
            
        private:
            BEGIN_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_query_token,do_query)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_deposit_token,do_deposit)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_withdraw_token,do_withdraw)
            END_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
        };
        
        //nonce property of account
        class xnoncevar_t : public xvproperty_t
        {
            friend class xvbstate_t;
        public:
            static  const std::string   name(){ return std::string("xnoncevar");}
            virtual std::string         get_obj_name() const override {return name();}
        private:
            static  void register_object(xcontext_t & context);
            enum{enum_obj_type = enum_xobject_type_vprop_nonce};//allow xbase create xvstate_t object from xdataobj_t::read_from()
            
        protected:
            xnoncevar_t();
            xnoncevar_t(const std::string & property_name,const vnonce_t property_value);
            xnoncevar_t(const xnoncevar_t & obj);
            virtual ~xnoncevar_t();
            
        private://dont implement those construction
            xnoncevar_t(xnoncevar_t &&);
            xnoncevar_t & operator = (const xnoncevar_t & other);
            
        public:
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override;//caller need to cast (void*) to related ptr
            virtual xvexeunit_t*  clone() override; //cone a new object with same state
            
        public:  //read interface
            const vnonce_t  get_nonce();
            
        public: //write interface
            const vnonce_t  alloc_nonce();//return the new available nonce
     
        private: //internal instruction functions
            const xvalue_t  do_alloc(const xvmethod_t & op);
            
        private:
            BEGIN_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_nonce_alloc,do_alloc)
            END_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
        };
    
        //standard std::string
        //note:not safe at multiple thread,ensure use at same thread of state or lock it before use
        class xstringvar_t : public xvproperty_t
        {
            friend class xvbstate_t;
        public:
            static  const std::string   name(){ return std::string("xstringvar");}
            virtual std::string         get_obj_name() const override {return name();}
        private:
            static  void register_object(xcontext_t & context);
            enum{enum_obj_type = enum_xobject_type_vprop_string};//allow xbase create xvstate_t object from xdataobj_t::read_from()
            
        protected:
            xstringvar_t(enum_xdata_type type = (enum_xdata_type)enum_obj_type);
            xstringvar_t(const std::string & property_name, const std::string & property_value,enum_xdata_type type = (enum_xdata_type)enum_obj_type);
            xstringvar_t(const xstringvar_t & obj);
            virtual ~xstringvar_t();
            
        private://dont implement those construction
            xstringvar_t(xstringvar_t &&);
            xstringvar_t & operator = (const xstringvar_t & other);
            
        public:
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override;//caller need to cast (void*) to related ptr
            virtual xvexeunit_t*  clone() override; //cone a new object with same state
                        
        public: //read interface
            const std::string &   query() const;//return whole string
            
        public: //write interface
            bool  reset(); //erase to empty string
            bool  reset(const std::string & values);//erase_all and set all
            
        protected://internal instruction functions
            virtual const xvalue_t  do_reset(const xvmethod_t & op);
            
        private://mapping instruction code/name and member functions
            BEGIN_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_string_reset,do_reset)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_code_deploy,do_reset)
            END_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
        };
    
        class xcodevar_t : public xstringvar_t
        {
            friend class xvbstate_t;
        public:
            static  const std::string   name(){ return std::string("xcodevar");}
            virtual std::string         get_obj_name() const override {return name();}
        private:
            static  void register_object(xcontext_t & context);
            enum{enum_obj_type = enum_xobject_type_vprop_code};//allow xbase create xvstate_t object from xdataobj_t::read_from()

        protected:
            xcodevar_t();
            xcodevar_t(const std::string & property_name, const std::string & property_value);
            xcodevar_t(const xcodevar_t & obj);
            virtual ~xcodevar_t();
        private://dont implement those construction
            xcodevar_t(xcodevar_t &&);
            xcodevar_t & operator = (const xcodevar_t & other);
            
        public:
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override;//caller need to cast (void*) to related ptr
            virtual xvexeunit_t*    clone() override; //cone a new object with same state
            
        public://read interface
            const std::string &     get_code() const {return xstringvar_t::query();}//return whole code string
            
        public://write interface
            bool  deploy_code(const std::string & values);//erase_all and set all
            
        private://internal instruction functions
            virtual const xvalue_t  do_reset(const xvmethod_t & op) override;
            
        private://disable write api exposed by xstringvar_t
            //bool  reset() {return false;}
            //bool  reset(const std::string & values){return false;}
            using xstringvar_t::reset;
        };
            
        //standard std::map<std::string,std::map<std::string,std::string> > : key : field:value
        //note:not safe at multiple thread,ensure use at same thread of state or lock it before use
        class xhashmapvar_t : public xvproperty_t
        {
            friend class xvbstate_t;
        public:
            static  const std::string   name(){ return std::string("xhashmapvar");}
            virtual std::string         get_obj_name() const override {return name();}
        private:
            static  void register_object(xcontext_t & context);
            enum{enum_obj_type = enum_xobject_type_vprop_hashmap};//allow xbase create xvstate_t object from xdataobj_t::read_from()
            
        protected:
            xhashmapvar_t(enum_xdata_type type = (enum_xdata_type)enum_obj_type);
            xhashmapvar_t(const std::string & property_name, const std::map<std::string,std::map<std::string,std::string> > & property_value,enum_xdata_type type = (enum_xdata_type)enum_obj_type);
            xhashmapvar_t(const xhashmapvar_t & obj);
            virtual ~xhashmapvar_t();
            
        private://dont implement those construction
            xhashmapvar_t(xhashmapvar_t &&);
            xhashmapvar_t & operator = (const xhashmapvar_t & other);
            
        public:
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override;//caller need to cast (void*) to related ptr
            virtual xvexeunit_t*  clone() override; //cone a new object with same state
                        
        public: //read interface
            bool                                    find(const std::string & key); //test whether key is existing or not
            const std::map<std::string,std::string> query(const std::string & key);
            const std::string                       query(const std::string & key,const std::string & field); //key&field must be exist,otherwise return empty string
            
        public: //write interface
            bool  insert(const std::string & key,const std::string & field,const std::string & value);//create key if not found key
            
            bool  erase(const std::string & key);//erase every filed assocated with this key
            bool  erase(const std::string & key,const std::string & field);//only successful when key already existing
            
            bool  reset(); //erase all key and values
            bool  reset(const std::map<std::string,std::map<std::string,std::string> > & key_values);//erase_all and set all
            
        protected://internal instruction functions
            virtual const xvalue_t  do_insert(const xvmethod_t & op);  //create key if not found key
            virtual const xvalue_t  do_erase(const xvmethod_t & op);   //only successful when key already existing
            virtual const xvalue_t  do_reset(const xvmethod_t & op);   //erase_all ,then set all
            
        private://mapping instruction code/name and member functions
            BEGIN_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_hashmap_insert,do_insert)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_hashmap_erase,do_erase)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_hashmap_reset,do_reset)
            END_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
        };
        
        //standard int(int8,int16,int32,int64,uint64_t)
        //note:not safe at multiple thread,ensure use at same thread of state or lock it before use
        template<typename T>
        class xvintvar_t : public xvproperty_t
        {
            friend class xvbstate_t;
        public:
            static  const std::string   name(){ return std::string("xvintvar");}
            virtual std::string         get_obj_name() const override {return name();}
        private:
            static  void register_object(xcontext_t & context)
            {
                auto lambda_new_func = [](const int type)->xobject_t*{
                    return new xvintvar_t<T>();
                };
                xcontext_t::register_xobject2(context,(enum_xobject_type)xvintvar_t<T>::query_obj_type(),lambda_new_func);
            }
            
        public:
            xvintvar_t(enum_xdata_type type = (enum_xdata_type)query_obj_type())
            :xvproperty_t(std::string(),T(0),type)
            {
                REGISTER_XVIFUNC_ID_API(enum_xvinstruct_class_state_function);
            }
            xvintvar_t(const std::string & name,const T value,enum_xdata_type type = (enum_xdata_type)query_obj_type())
            :xvproperty_t(name,value,type)
            {
                REGISTER_XVIFUNC_ID_API(enum_xvinstruct_class_state_function);
            }
            xvintvar_t(const xvintvar_t & obj)
            :xvproperty_t(obj)
            {
                REGISTER_XVIFUNC_ID_API(enum_xvinstruct_class_state_function);
            }
            virtual ~xvintvar_t(){};
            
        private://dont implement those construction
            xvintvar_t(xvintvar_t &&);
            xvintvar_t & operator = (const xvintvar_t & other);
            
        public:
            virtual xvexeunit_t*  clone() override //cone a new object with same state
            {
                return new xvintvar_t(*this);
            }
            
            //caller need to cast (void*) to related ptr
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override
            {
                if(_enum_xobject_type_ == query_obj_type())
                    return this;
                
                return xvproperty_t::query_interface(_enum_xobject_type_);
            }
            
        public://read interface
            T     get() const { return get_value().template get_value<T>();}
            
        public: //write interface
            bool  set(const T new_value)
            {
                xvalue_t param(new_value);
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_integer_reset,param);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == enum_xcode_successful);
            }
            bool  reset()
            {
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_integer_reset);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == enum_xcode_successful);
            }
        protected://internal instruction functions
            virtual const xvalue_t  do_reset(const xvmethod_t & op)
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_integer_reset)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() > 1) //carry too many pararms
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                if(op.get_method_params().size() == 1)
                {
                    const xvalue_t & new_value = op.get_method_params().at(0);
                    if(new_value.get_type() != get_value().get_type())
                    {
                        xassert(0);
                        return xvalue_t(enum_xerror_code_invalid_param_type);
                    }
                    copy_from_value(new_value);
                }
                else
                {
                    xvalue_t new_value((T)0);
                    copy_from_value(new_value);
                }
                return xvalue_t(enum_xcode_successful);
            }
        private:
            static const int  query_obj_type();
            
        private://mapping instruction code/name and member functions
            BEGIN_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_integer_reset,do_reset)
            END_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
        };
    
        //standard std::deque<xxx>
        //note:not safe at multiple thread,ensure use at same thread of state or lock it before use
        template<typename T>
        class xdequevar_t : public xvproperty_t
        {
            friend class xvbstate_t;
        public:
            static  const std::string   name(){ return std::string("xdequevar");}
            virtual std::string         get_obj_name() const override {return name();}
            static  const int           query_obj_type();
        private:
            static  void register_object(xcontext_t & context)
            {
                auto lambda_new_func = [](const int type)->xobject_t*{
                    return new xdequevar_t<T>();
                };
                xcontext_t::register_xobject2(context,(enum_xobject_type)xdequevar_t<T>::query_obj_type(),lambda_new_func);
            }

        protected:
            xdequevar_t(enum_xdata_type type = (enum_xdata_type)query_obj_type())
                :xvproperty_t(std::string(),std::deque<T>(),type)
            {
                REGISTER_XVIFUNC_ID_API(enum_xvinstruct_class_state_function);
            }
            xdequevar_t(const std::string & property_name, const std::deque<T> & property_value,enum_xdata_type type = (enum_xdata_type)query_obj_type())
                :xvproperty_t(property_name,property_value,type)
            {
                REGISTER_XVIFUNC_ID_API(enum_xvinstruct_class_state_function);
            }
            xdequevar_t(const xdequevar_t & obj)
                :xvproperty_t(obj)
            {
            }
            virtual ~xdequevar_t(){};
            
        private://dont implement those construction
            xdequevar_t(xdequevar_t &&);
            xdequevar_t & operator = (const xdequevar_t & other);
            
        public:
            virtual xvexeunit_t*  clone() override //clone a new object with same state
            {
                return new xdequevar_t(*this);
            }
            
            //caller need to cast (void*) to related ptr
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override
            {
                if(_enum_xobject_type_ == query_obj_type())
                    return this;
                
                return xvproperty_t::query_interface(_enum_xobject_type_);
            }
            
        public: //read interface
            const T  query(const uint32_t pos) //position must be valid
            {
                const std::deque<T>* queue_obj = get_value().template get_deque<T>();
                xassert(queue_obj != nullptr);
                if(queue_obj != nullptr)
                {
                    if(pos < queue_obj->size())
                        return queue_obj->at(pos);
                    
                    xassert(0);
                }
                return T();
            }
            
            const T  query_front()
            {
                const std::deque<T>* queue_obj = get_value().template get_deque<T>();
                xassert(queue_obj != nullptr);
                if(queue_obj != nullptr)
                    return queue_obj->front();

                return T();
            }
            
            const T  query_back()
            {
                const std::deque<T>* queue_obj = get_value().template get_deque<T>();
                xassert(queue_obj != nullptr);
                if(queue_obj != nullptr)
                    return queue_obj->back();
                
                return T();
            }
            
            const std::deque<T> query() //return whole queue
            {
                const std::deque<T>* queue_obj = get_value().template get_deque<T>();
                xassert(queue_obj != nullptr);
                if(queue_obj != nullptr)
                    return *queue_obj;
                
                return std::deque<T>();
            }
            
        public: //write interface
            bool  push_front(const T & value)
            {
                xvalue_t new_value(value);
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_queue_push_front,new_value);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == 0);
            }
            
            bool  pop_front()
            {
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_queue_pop_front);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == 0);
            }
            
            bool  push_back(const T & value)
            {
                xvalue_t new_value(value);
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_queue_push_back,new_value);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == 0);
            }
            
            bool  pop_back()
            {
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_queue_pop_back);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == 0);
            }
            
            bool  update(const int32_t pos,const T & value) //position must be valid
            {
                if(pos < 0)
                {
                    xassert(pos >= 0);
                    return false;
                }
                
                xvalue_t target_pos((const vint32_t)pos);
                xvalue_t target_value(value);
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_queue_update,target_pos,target_value);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == 0);
            }
            
            bool  reset() //erase all values
            {
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_queue_reset);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == 0);
            }
            
            bool  reset(const std::deque<T> & values) //erase_all and set all
            {
                xvalue_t new_queue(values);
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_queue_reset,new_queue);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == 0);
            }
            
        private://internal instruction functions
            const xvalue_t  do_push_front(const xvmethod_t & op)
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_queue_push_front)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() != 1) //must carry value
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                const xvalue_t & push_value_param = op.get_method_params().at(0);
                if(push_value_param.get_type() != xvalue_t::query_type<T>())
                    return xvalue_t(enum_xerror_code_invalid_param_type);
 
                std::deque<T>* queue_obj = (std::deque<T>*)get_writable_value().template get_deque<T>();
                xassert(queue_obj != nullptr);
                if(queue_obj != nullptr)
                {
                    queue_obj->push_front(push_value_param.template get_value<T>());
                    return xvalue_t(enum_xcode_successful);
                }
                return xvalue_t(enum_xerror_code_bad_vfunction);
            }
            
            const xvalue_t do_pop_front(const xvmethod_t & op)
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_queue_pop_front)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() != 0) //should not carry any parameters
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                std::deque<T>* queue_obj = (std::deque<T>*)get_writable_value().template get_deque<T>();
                xassert(queue_obj != nullptr);
                if(queue_obj != nullptr)
                {
                    queue_obj->pop_front();
                    return xvalue_t(enum_xcode_successful);
                }
                return xvalue_t(enum_xerror_code_bad_vfunction);
            }
            
            const xvalue_t  do_push_back(const xvmethod_t & op)
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_queue_push_back)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() != 1) //must carry value
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                const xvalue_t & push_value_param = op.get_method_params().at(0);
                if(push_value_param.get_type() != xvalue_t::query_type<T>())
                    return xvalue_t(enum_xerror_code_invalid_param_type);
                 
                std::deque<T>* queue_obj = (std::deque<T>*)get_writable_value().template get_deque<T>();
                xassert(queue_obj != nullptr);
                if(queue_obj != nullptr)
                {
                    queue_obj->push_back(push_value_param.template get_value<T>());
                    return xvalue_t(enum_xcode_successful);
                }
                return xvalue_t(enum_xerror_code_bad_vfunction);
            }
            
            const xvalue_t do_pop_back(const xvmethod_t & op)
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_queue_pop_back)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() != 0) //should not carry any parameters
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                std::deque<T>* queue_obj = (std::deque<T>*)get_writable_value().template get_deque<T>();
                xassert(queue_obj != nullptr);
                if(queue_obj != nullptr)
                {
                    queue_obj->pop_back();
                    return xvalue_t(enum_xcode_successful);
                }
                return xvalue_t(enum_xerror_code_bad_vfunction);
            }
            
            const xvalue_t  do_update(const xvmethod_t & op)
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_queue_update)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() != 2) //must carry position and content
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                const xvalue_t & target_pos_param = op.get_method_params().at(0);
                if(target_pos_param.get_type() != xvalue_t::enum_xvalue_type_int32)
                    return xvalue_t(enum_xerror_code_invalid_param_type);
                const int32_t target_pos = target_pos_param.get_int32();
                if(target_pos < 0)
                    return xvalue_t(enum_xerror_code_bad_param);
                
                const xvalue_t & target_value_param = op.get_method_params().at(1);
                if(target_value_param.get_type() != xvalue_t::query_type<T>())
                    return xvalue_t(enum_xerror_code_invalid_param_type);
                
                std::deque<T>* queue_obj = (std::deque<T>*)get_writable_value().template get_deque<T>();
                xassert(queue_obj != nullptr);
                if(queue_obj != nullptr)
                {
                    if(target_pos >= queue_obj->size())
                        return xvalue_t(enum_xerror_code_bad_param);
                    
                    queue_obj->at(target_pos) = target_value_param.template get_value<T>();
                    return xvalue_t(enum_xcode_successful);
                }
                return xvalue_t(enum_xerror_code_bad_vfunction);
            }
            
            const xvalue_t do_reset(const xvmethod_t & op)  //erase all key and values
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_queue_reset)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() > 1) //carry too many pararms
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                if(op.get_method_params().size() == 1)
                {
                    const xvalue_t & new_map_param = op.get_method_params().at(0);
                    if(new_map_param.get_deque<T>() == nullptr)
                        return xvalue_t(enum_xerror_code_bad_param);
                    
                    copy_from_value(new_map_param);
                }
                else
                {
                    std::deque<T> empty;
                    copy_from_value(empty);
                }
                return xvalue_t(enum_xcode_successful);
            }
            
        private://mapping instruction code/name and member functions
            BEGIN_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_queue_push_front,do_push_front)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_queue_pop_front,do_pop_front)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_queue_push_back,do_push_back)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_queue_pop_back,do_pop_back)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_queue_update,do_update)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_queue_reset,do_reset)
            END_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
        };
    
        //standard std::map<std::string,xxx>
        //note:not safe at multiple thread,ensure use at same thread of state or lock it before use
        template<typename T>
        class xmapvar_t : public xvproperty_t
        {
            friend class xvbstate_t;
        public:
            static  const std::string   name(){ return std::string("xmapvar");}
            virtual std::string         get_obj_name() const override {return name();}
            static  const int           query_obj_type();
        private:
            static  void register_object(xcontext_t & context)
            {
                auto lambda_new_func = [](const int type)->xobject_t*{
                    return new xmapvar_t<T>();
                };
                xcontext_t::register_xobject2(context,(enum_xobject_type)xmapvar_t<T>::query_obj_type(),lambda_new_func);
            }
        protected:
            xmapvar_t(enum_xdata_type type = (enum_xdata_type)query_obj_type())
                :xvproperty_t(std::string(),std::map<std::string,T>(),type)
            {
                REGISTER_XVIFUNC_ID_API(enum_xvinstruct_class_state_function);
            }
            xmapvar_t(const std::string & property_name, const std::map<std::string,T> & property_value,enum_xdata_type type = (enum_xdata_type)query_obj_type())
                :xvproperty_t(property_name,property_value,type)
            {
                REGISTER_XVIFUNC_ID_API(enum_xvinstruct_class_state_function);
            }
            xmapvar_t(const xmapvar_t & obj)
                :xvproperty_t(obj)
            {
            }
            virtual ~xmapvar_t(){};
            
        private://dont implement those construction
            xmapvar_t(xmapvar_t &&);
            xmapvar_t & operator = (const xmapvar_t & other);
            
        public:
            virtual xvexeunit_t*  clone() override //clone a new object with same state
            {
                return new xmapvar_t(*this);
            }
            
            //caller need to cast (void*) to related ptr
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override
            {
                if(_enum_xobject_type_ == query_obj_type())
                    return this;
                
                return xvproperty_t::query_interface(_enum_xobject_type_);
            }
            
        public: //read interface
            bool     find(const std::string & key) //test whether key is existing or not
            {
                const std::map<std::string,T>* map_obj = get_value().template get_map<T>();
                xassert(map_obj != nullptr);
                if(map_obj != nullptr)
                {
                    auto it = map_obj->find(key);
                    if(it != map_obj->end())
                        return true;
                }
                return false;
            }
            
            const T  query(const std::string & key) //key must be exist,otherwise return empty string
            {
                const std::map<std::string,T>* map_obj = get_value().template get_map<T>();
                xassert(map_obj != nullptr);
                if(map_obj != nullptr)
                {
                    auto it = map_obj->find(key);
                    if(it != map_obj->end())
                        return it->second;
                }
                return T();
            }
            
            const std::map<std::string,T> query() //return whole maps
            {
                const std::map<std::string,T>* map_obj = get_value().template get_map<T>();
                xassert(map_obj != nullptr);
                if(map_obj != nullptr)
                    return *map_obj;

                return std::map<std::string,T>();
            }
            
        public: //write interface
            bool  insert(const std::string & key,const T value)//create key if not found key
            {
                xvalue_t new_key(key);
                xvalue_t new_value(value);
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_map_insert,new_key,new_value);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == enum_xcode_successful);
            }
            
            bool  erase(const std::string & key)//only successful when key already existing
            {
                xvalue_t target(key);
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_map_erase,target);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == enum_xcode_successful);
            }
            
            bool  reset() //erase all key and values
            {
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_map_reset);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == enum_xcode_successful);
            }
            
            bool reset(const std::map<std::string,T> & maps)//erase_all and set all
            {
                xvalue_t new_map(maps);
                xvmethod_t instruction(get_execute_uri(),enum_xvinstruct_class_state_function, enum_xvinstruct_state_method_map_reset,new_map);
                return (execute(instruction,(xvcanvas_t*)get_canvas()).get_error() == enum_xcode_successful);
            }
            
        protected://internal instruction functions
            virtual const xvalue_t  do_insert(const xvmethod_t & op)     //create key if not found key
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_map_insert)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() != 2) //must carry key,value
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                const xvalue_t & insert_key_param = op.get_method_params().at(0);
                if(insert_key_param.get_type() != xvalue_t::enum_xvalue_type_string)
                    return xvalue_t(enum_xerror_code_invalid_param_type);
                
                const xvalue_t & insert_value_param = op.get_method_params().at(1);
                if(insert_value_param.get_type() != xvalue_t::query_type<T>())
                    return xvalue_t(enum_xerror_code_invalid_param_type);
                
                const std::string target_key_string = insert_key_param.get_string();
                std::map<std::string,T>* map_obj = (std::map<std::string,T>*)get_writable_value().template get_map<T>();
                xassert(map_obj != nullptr);
                if(map_obj != nullptr)
                {
                    (*map_obj)[target_key_string] = insert_value_param.template get_value<T>();
                    return xvalue_t(enum_xcode_successful);
                }
                return xvalue_t(enum_xerror_code_bad_vproperty);
            }
            
            virtual  const xvalue_t do_erase(const xvmethod_t & op)   //only successful when key already existing
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_map_erase)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() != 1) //must carry newkey
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                const xvalue_t & erase_key_param = op.get_method_params().at(0);
                if(erase_key_param.get_type() != xvalue_t::enum_xvalue_type_string)
                    return xvalue_t(enum_xerror_code_invalid_param_type);
                
                const std::string target_key_string = erase_key_param.get_string();
                std::map<std::string,T>* map_obj = (std::map<std::string,T>*)get_writable_value().template get_map<T>();
                xassert(map_obj != nullptr);
                if(map_obj != nullptr)
                {
                    auto it = map_obj->find(target_key_string);
                    if(it != map_obj->end())
                        map_obj->erase(it);
                    
                    return xvalue_t(enum_xcode_successful);
                }
                return xvalue_t(enum_xerror_code_bad_vproperty);
            }
            
            virtual  const xvalue_t do_reset(const xvmethod_t & op)  //erase all key and values
            {
                if(op.get_method_id() != enum_xvinstruct_state_method_map_reset)
                    return xvalue_t(enum_xerror_code_bad_method);
                
                if(op.get_method_params().size() > 1) //carry too many pararms
                    return xvalue_t(enum_xerror_code_invalid_param_count);
                
                if(op.get_method_params().size() == 1)
                {
                    const xvalue_t & new_map_param = op.get_method_params().at(0);
                    if(new_map_param.get_map<T>() == nullptr)
                        return xvalue_t(enum_xerror_code_bad_param);
                    
                    copy_from_value(new_map_param);
                }
                else
                {
                    std::map<std::string,T> empty;
                    copy_from_value(empty);
                }
                return xvalue_t(enum_xcode_successful);
            }
            
        private://mapping instruction code/name and member functions
            BEGIN_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_map_insert,do_insert)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_map_erase,do_erase)
                IMPL_XVIFUNCE_ID_API(enum_xvinstruct_state_method_map_reset,do_reset)
            END_DECLARE_XVIFUNC_ID_API(enum_xvinstruct_class_state_function)
        };
    
        //manage all native tokens by std::map<std::string,vtoken_t>
        //note:not safe at multiple thread,ensure use at same thread of state or lock it before use
        class xmtokens_t : public xmapvar_t<int64_t>
        {
            friend class xvbstate_t;
            typedef xmapvar_t<int64_t> base;
        public:
            static  const std::string   name(){ return std::string("xmtokens");}
            virtual std::string         get_obj_name() const override {return name();}
     
        private:
            static  void register_object(xcontext_t & context);
            enum{enum_obj_type = enum_xobject_type_vprop_mtokens};//allow xbase create xvstate_t object from xdataobj_t::read_from()

        protected:
            xmtokens_t();
            xmtokens_t(const std::string & property_name, const std::map<std::string,int64_t> & property_value);
            xmtokens_t(const xmtokens_t & obj);
            virtual ~xmtokens_t();
            
        private://dont implement those construction
            xmtokens_t(xmtokens_t &&);
            xmtokens_t & operator = (const xmtokens_t & other);
            
        public:
            //caller need to cast (void*) to related ptr
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override;
            
            virtual xvexeunit_t*  clone() override; //clone a new object with same state

        public:  //read interface
            const int64_t  get_balance(const std::string & token_name);
            
        public: //write interface
            bool  deposit(const std::string & token_name ,const int64_t add_token);
            bool  withdraw(const std::string & token_name,const int64_t sub_token);
            
        private://disable write api exposed by xstringvar_t
            using  base::insert;
            using  base::erase;
            using  base::reset;
        };
        
        //manage all keys of accounts
        //note:not safe at multiple thread,ensure use at same thread of state or lock it before use
        class xmkeys_t : public xmapvar_t<std::string>
        {
            friend class xvbstate_t;
            typedef xmapvar_t<std::string> base;
        public:
            static  const std::string   name(){ return std::string("xmkeys");}
            virtual std::string         get_obj_name() const override {return name();}
            
        private:
            static  void register_object(xcontext_t & context);
            enum{enum_obj_type = enum_xobject_type_vprop_mkeys};//allow xbase create xvstate_t object from xdataobj_t::read_from()
            
        protected:
            xmkeys_t();
            xmkeys_t(const std::string & property_name, const std::map<std::string,std::string> & property_value);
            xmkeys_t(const xmkeys_t & obj);
            virtual ~xmkeys_t();
            
        private://dont implement those construction
            xmkeys_t(xmkeys_t &&);
            xmkeys_t & operator = (const xmkeys_t & other);
            
        public:
            //caller need to cast (void*) to related ptr
            virtual void*    query_interface(const int32_t _enum_xobject_type_) override;
            virtual xvexeunit_t*  clone() override; //clone a new object with same state
            
        public:  //read interface
            const  std::string    query_key(const std::string & key_name);
            
        public: //write interface
            bool   deploy_key(const std::string & key_name ,const std::string & key_string);
     
        private://disable write api exposed by xstringvar_t
            using  base::insert;
            using  base::erase;
            using  base::reset;
        };
    
    };//end of namespace of base

};//end of namespace top
