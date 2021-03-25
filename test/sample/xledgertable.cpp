// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xatom.h"
#include "xutl.h"
#include "xledgertable.h"

#ifndef __XLEDGER_ASYNC_WRITE_TO_DB__
    #define __XLEDGER_ASYNC_WRITE_TO_DB__  //enable asynchronize mode for writing as default
#endif 

namespace top
{
    namespace ledger
    {
        xledgertable_t::xledgertable_t(base::xcontext_t & _context,const int32_t target_thread_id,const uint32_t table_id,xdb_face_t* db,xledgerbook_t * parent_book_ptr)
            :base::xiobject_t(_context,target_thread_id,base::enum_xobject_type_dataobj)
        {
            memset(m_cache_objects,0,sizeof(m_cache_objects));
            
            m_db_ptr = db;
            xassert(m_db_ptr != NULL);
            m_parent_book_ptr = parent_book_ptr;
            xassert(m_parent_book_ptr != NULL);
            m_parent_book_ptr->add_ref();
            
            m_table_id = table_id;
            m_book_id = m_parent_book_ptr->get_book_id();
        }
 
        xledgertable_t::~xledgertable_t()
        {
            if(m_parent_book_ptr != NULL)
            {
                m_parent_book_ptr->release_ref();
            }
        }
        
        //on_object_close be called when close command processed at host thread,logic flow: Caller(Thread#A)->Close()->Wake this object thread(B)->clean and then execute: on_object_close
        bool xledgertable_t::on_object_close()//notify the subclass the object is closed
        {
            base::xiobject_t::on_object_close(); //set close flag first
            
            xledgerbook_t * parent_ptr = base::xatomic_t::xexchange(m_parent_book_ptr, (xledgerbook_t*)NULL);
            if(parent_ptr != NULL)
            {
                for(int i = 0; i < enum_const_cache_max_objects_count; ++i)
                {
                    base::xdataobj_t* cached_ptr = base::xatomic_t::xexchange(m_cache_objects[i], (base::xdataobj_t*)NULL);
                    if(cached_ptr != NULL)
                    {
                        cached_ptr->close(false);
                        get_context()->delay_release_object(cached_ptr);
                    }
                }
                m_cache_timeout_map.clear(); //clean map at host thread context
                get_context()->delay_release_object(parent_ptr);
            }
            return true;
        }
        
        void  xledgertable_t::clear()
        {
            for(int i = 0; i < enum_const_cache_max_objects_count; ++i)
            {
                base::xdataobj_t* cached_ptr = base::xatomic_t::xexchange(m_cache_objects[i], (base::xdataobj_t*)NULL);
                if(cached_ptr != NULL)
                {
                    cached_ptr->close(false);
                    get_context()->delay_release_object(cached_ptr);
                }
            }
            
            if( (is_close() == false) && (m_parent_book_ptr != NULL) )
            {
                auto on_clear_function = [this](base::xcall_t & call, const int32_t cur_thread_id, const uint64_t timenow_ms)->bool{
                    m_cache_timeout_map.clear(); //clean map at host thread context
                    return true;
                };
                base::xcall_t asyn_call(on_clear_function,(base::xobject_t*)this);
                m_parent_book_ptr->dispatch_call(asyn_call);
            }
        }
        
        //directly access persist DB
        int xledgertable_t::get_key(const uint32_t key_hash, const std::string & key,std::string & value,const uint32_t cache_expired_after_seconds)
        {
            if((0 == key_hash) || key.empty())
                return enum_xerror_code_bad_key;
            
            if(is_close() || (NULL == m_parent_book_ptr) )
                return enum_xerror_code_closed;
            
            const int index = key_hash % enum_const_cache_max_objects_count;
            base::xunknowobj_t* hit_cache_container = (base::xunknowobj_t*)m_cache_objects[index];
            if( (hit_cache_container != NULL) && (key_hash == hit_cache_container->get_hash_of_key()) && (key == hit_cache_container->get_key()))
            {
                hit_cache_container->get(value);
                return enum_xcode_successful;
            }
            auto ret = m_db_ptr->read(key, value);  //  todo use slice return and set xstream
            if (!ret){
                xwarn("xledgertable_t::get_key, db not found key(%s)", key.c_str());
                return enum_xerror_code_not_found;
            }
            
            if(cache_expired_after_seconds > 0) //ask to cache
            {
                base::xunknowobj_t* cache_container_ptr = new base::xunknowobj_t(key);
                cache_container_ptr->set(value);//assume has changed
                cache_container_ptr->set_hash_of_key(key_hash);
                cache_container_ptr->set_last_access_time(base::xtime_utl::gmttime_ms());
                cache_container_ptr->set_cache_expire_ms(cache_container_ptr->get_last_access_time() + (cache_expired_after_seconds << 10) );//convert to ms
                base::xdataobj_t* old_ptr = base::xatomic_t::xexchange(m_cache_objects[index], (base::xdataobj_t*)cache_container_ptr);
                if(old_ptr != NULL)
                    get_context()->delay_release_object(old_ptr);
                
                auto on_get_function = [this](base::xcall_t & call, const int32_t cur_thread_id, const uint64_t timenow_ms)->bool{
                    base::xunknowobj_t * _object_param = reinterpret_cast<base::xunknowobj_t*>(call.get_param1().get_object());
                    
                    m_cache_timeout_map.insert(std::multimap<uint64_t,uint32_t>::value_type(_object_param->get_cache_expire_ms(),_object_param->get_hash_of_key()));
                    return true;
                };
                base::xcall_t asyn_call(on_get_function,cache_container_ptr,(base::xobject_t*)this);
                m_parent_book_ptr->send_call(asyn_call);
            }
            return enum_xcode_successful;
        }
        
        int xledgertable_t::get_key(const uint32_t key_hash, const std::string & key,std::function<void(int,const std::string &,const std::string &)> & aysnc_get_notify,const uint32_t cache_expired_after_seconds)
        {
            if((0 == key_hash) || key.empty())
                return enum_xerror_code_bad_key;
            
            if(is_close() || (NULL == m_parent_book_ptr) )
                return enum_xerror_code_closed;
            
            base::xunknowobj_t* cache_container_ptr = new base::xunknowobj_t(key);//create first to carry other information
            cache_container_ptr->set_hash_of_key(key_hash);
            cache_container_ptr->set_last_access_time(base::xtime_utl::gmttime_ms());
            cache_container_ptr->set_cache_expire_ms(cache_container_ptr->get_last_access_time() + (cache_expired_after_seconds << 10) );//convert to ms

            auto on_get_function = [this](base::xcall_t & call, const int32_t cur_thread_id, const uint64_t timenow_ms)->bool{
                base::xunknowobj_t * _object_param = reinterpret_cast<base::xunknowobj_t*>(call.get_param1().get_object());
                std::function<void(int,const std::string &,const std::string &)>* _notify_param = (std::function<void(int,const std::string &,const std::string &)>*)call.get_param2().get_int64();
                
                const uint32_t _key_hash = _object_param->get_hash_of_key();
                const int index = _key_hash % enum_const_cache_max_objects_count;
                
                std::string _value_param;
                auto ret = m_db_ptr->read(_object_param->get_key(), _value_param);  //  todo use slice return and set xstream
                if (ret)
                {
                    _object_param->set(_value_param);
                    
                    //note: _object_param was create by new operation,so here no need to add_ref again
                    base::xdataobj_t* old_ptr = base::xatomic_t::xexchange(m_cache_objects[index], (base::xdataobj_t*)_object_param);
                    if(old_ptr != NULL)
                        get_context()->delay_release_object(old_ptr);
                    
                    m_cache_timeout_map.insert(std::multimap<uint64_t,uint32_t>::value_type(_object_param->get_cache_expire_ms(),_key_hash));

                    //notify all operation is finished
                    (*_notify_param)(enum_xcode_successful,_object_param->get_key(),_value_param);
                }
                else
                {
                    xwarn("xledgertable_t::get_key, db not found key(%s)", _object_param->get_key().c_str());
                    (*_notify_param)(enum_xerror_code_not_found,_object_param->get_key(),_value_param);
                    _object_param->release_ref(); //destroy object(paired with above code of 'new base::xunknowobj_t(key)'
                }
                
                delete _notify_param; //destroy object that created before send_call
                return true;
            };
            std::function<void(int,const std::string &,const std::string &)> * _copy_function_obj = new std::function<void(int,const std::string &,const std::string &)>(aysnc_get_notify);
            const int64_t _cast_to_int64_from_ptr = (int64_t)_copy_function_obj;
            base::xcall_t asyn_call(on_get_function,cache_container_ptr,_cast_to_int64_from_ptr,(base::xobject_t*)this);
            
            const int res = m_parent_book_ptr->send_call(asyn_call);//send to target thread
            if(res != enum_xcode_successful)//fail to deliver as exception
            {
                cache_container_ptr->release_ref(); //destroy object(paired with above code of 'new base::xunknowobj_t(key)'
                delete _copy_function_obj; //destroy function that created before send_call
            }
            return res;
        }
        
        //asynronize mode to access persist DB
        int xledgertable_t::set_key(const uint32_t key_hash, const std::string & key,const std::string & value,const uint32_t cache_expired_after_seconds)
        {
            if((0 == key_hash) || key.empty())
                return enum_xerror_code_bad_key;
            
            if(is_close() || (NULL == m_parent_book_ptr) )
                return enum_xerror_code_closed;
            
            #ifdef DEBUG
                xdbg("xledgertable_t::set_key, key(%s) set to db", key.c_str());
            #endif
            
            const int index = key_hash % enum_const_cache_max_objects_count;
            #ifdef __XLEDGER_ASYNC_WRITE_TO_DB__
            base::xunknowobj_t* cache_container_ptr = new base::xunknowobj_t(key);
            cache_container_ptr->set(value);
            cache_container_ptr->set_hash_of_key(key_hash);
            cache_container_ptr->set_last_access_time(base::xtime_utl::gmttime_ms());
            cache_container_ptr->set_cache_expire_ms(cache_container_ptr->get_last_access_time() + ((cache_expired_after_seconds + 1) << 10) );//convert to ms
            base::xdataobj_t* old_ptr = base::xatomic_t::xexchange(m_cache_objects[index], (base::xdataobj_t*)cache_container_ptr);//force to update cache
            if(old_ptr != NULL)
                get_context()->delay_release_object(old_ptr);
            
            auto on_write_function = [this](base::xcall_t & call, const int32_t cur_thread_id, const uint64_t timenow_ms)->bool{
                base::xunknowobj_t * _object_param = reinterpret_cast<base::xunknowobj_t*>(call.get_param1().get_object());
                
                m_db_ptr->write(_object_param->get_key(), _object_param->get_value());//write db first
                m_cache_timeout_map.insert(std::multimap<uint64_t,uint32_t>::value_type(_object_param->get_cache_expire_ms(),_object_param->get_hash_of_key()));
                
                return true;
            };
            base::xcall_t asyn_call(on_write_function,cache_container_ptr,(base::xobject_t*)this);
            return m_parent_book_ptr->send_call(asyn_call);
            
            #else
            m_db_ptr->write(key, value); //write db first
            base::xdataobj_t* old_ptr = base::xatomic_t::xexchange(m_cache_objects[index], (base::xdataobj_t*)NULL); //clean cache
            if(old_ptr != NULL)
                get_context()->delay_release_object(old_ptr);
            
            return enum_xcode_successful;
            #endif
        }
        
        //asynronize mode to access persist DB
        int xledgertable_t::set_key(const uint32_t key_hash, const std::string & key,const std::string & value,std::function<void(int,const std::string &,const std::string &)> & aysnc_set_notify,const uint32_t cache_expired_after_seconds)
        {
            if((0 == key_hash) || key.empty())
                return enum_xerror_code_bad_key;
            
            if(is_close() || (NULL == m_parent_book_ptr) )
                return enum_xerror_code_closed;
            
            #ifdef DEBUG
            xdbg("xledgertable_t::set_key with notify, key(%s) set to db", key.c_str());
            #endif
            
            
            const int index = key_hash % enum_const_cache_max_objects_count;
            #ifdef __XLEDGER_ASYNC_WRITE_TO_DB__
            base::xunknowobj_t* cache_container_ptr = new base::xunknowobj_t(key);
            cache_container_ptr->set(value);
            cache_container_ptr->set_hash_of_key(key_hash);
            cache_container_ptr->set_last_access_time(base::xtime_utl::gmttime_ms());
            cache_container_ptr->set_cache_expire_ms(cache_container_ptr->get_last_access_time() + ((cache_expired_after_seconds + 1) << 10) );//convert to ms
            base::xdataobj_t* old_ptr = base::xatomic_t::xexchange(m_cache_objects[index], (base::xdataobj_t*)cache_container_ptr);//force to update cache
            if(old_ptr != NULL)
                get_context()->delay_release_object(old_ptr);
            
            #else
            m_db_ptr->write(key, value); //write db first
            base::xdataobj_t* old_ptr = base::xatomic_t::xexchange(m_cache_objects[index], (base::xdataobj_t*)NULL); //clean cache
            if(old_ptr != NULL)
                get_context()->delay_release_object(old_ptr);
            #endif
 
            auto on_write_function = [this](base::xcall_t & call, const int32_t cur_thread_id, const uint64_t timenow_ms)->bool{
                base::xunknowobj_t * _object_param = reinterpret_cast<base::xunknowobj_t*>(call.get_param1().get_object());
                
                #ifdef __XLEDGER_ASYNC_WRITE_TO_DB__
                m_db_ptr->write(_object_param->get_key(), _object_param->get_value());//write db first
                m_cache_timeout_map.insert(std::multimap<uint64_t,uint32_t>::value_type(_object_param->get_cache_expire_ms(),_object_param->get_hash_of_key()));
                #endif
                
                std::function<void(int,const std::string &,const std::string &)>* _notify_param = (std::function<void(int,const std::string &,const std::string &)>*)call.get_param2().get_int64();
                (*_notify_param)(enum_xcode_successful,_object_param->get_key(), _object_param->get_value());
                delete _notify_param; //destroy object that created before send_call
                
                return true;
            };
            //manually create new function object
            std::function<void(int,const std::string &,const std::string &)> * _copy_function_obj = new std::function<void(int,const std::string &,const std::string &)>(aysnc_set_notify);
            const int64_t _cast_to_int64_from_ptr = (int64_t)_copy_function_obj;
            
            base::xcall_t asyn_call(on_write_function,cache_container_ptr,_cast_to_int64_from_ptr,(base::xobject_t*)this);
            const int res = m_parent_book_ptr->send_call(asyn_call);
            if(res != enum_xcode_successful)
                delete _copy_function_obj; //destroy function that created before send_call
            
            return res;
        }
        
        //clean cache & remove key from persist DB
        int xledgertable_t::remove_key(const uint32_t key_hash, const std::string & key)
        {
            if((0 == key_hash) || key.empty())
                return enum_xerror_code_bad_key;
            
            if(is_close() || (NULL == m_parent_book_ptr) )
                return enum_xerror_code_closed;
            
            #ifdef DEBUG
                xdbg("xledger_t::remove_key, key(%s) from db", key.c_str());
            #endif
            
            try_clean_cache(key_hash,key); //directly clean first
            m_db_ptr->erase(key);
            return enum_xcode_successful;
        }
        
        //clean cache
        int xledgertable_t::close_key(const uint32_t key_hash, const std::string & key)
        {
            #ifdef DEBUG
                xdbg("xledgertable_t::close_key, key(%s)", key.c_str());
            #endif
            
            try_clean_cache(key_hash,key); //clean first
            return enum_xcode_successful;
        }
        
        //do search cache first ,then try to load from persist DB
        base::xdataobj_t* xledgertable_t::get_object(const uint32_t key_hash, const std::string & key,const uint32_t cach_expired_after_seconds)
        {
            if((0 == key_hash) || key.empty())
                return NULL;
            
            if(is_close() || (NULL == m_parent_book_ptr) )
                return NULL;
            
            const int index = key_hash % enum_const_cache_max_objects_count;
            base::xunknowobj_t* hit_cache_container = (base::xunknowobj_t*)m_cache_objects[index];
            if( (hit_cache_container != NULL) && (key_hash == hit_cache_container->get_hash_of_key()) && (key == hit_cache_container->get_key()))
            {
                const std::string & cache_value = hit_cache_container->get_value();
                base::xstream_t stream(base::xcontext_t::instance(), (uint8_t*)cache_value.data(), (uint32_t)cache_value.size());
                base::xdataobj_t* raw_data_obj = base::xdataobj_t::read_from(stream);//new_data_obj already take one reference
                if(raw_data_obj != NULL)
                {
                    xassert(key == raw_data_obj->get_key());//should be exactly same
                    
                    raw_data_obj->set_hash_of_key(key_hash);
                    raw_data_obj->set_last_access_time(base::xtime_utl::gmttime_ms());
                    return raw_data_obj;
                }
                //exception data(might be corrupt, reload again
            }
             
            std::string value;
            auto ret = m_db_ptr->read(key, value);  //  todo use slice return and set xstream
            if (!ret) {
                xwarn("xledgertable_t::get_object, db not found, key(%s)", key.c_str());
                return NULL;
            }
            
            base::xstream_t stream(base::xcontext_t::instance(), (uint8_t*)value.data(), (uint32_t)value.size());
            base::xdataobj_t* new_data_obj = base::xdataobj_t::read_from(stream);//new_data_obj already take one reference
            xassert(nullptr != new_data_obj);
            if (nullptr == new_data_obj) {
                xwarn("xledgertable_t::get_object, object unserialize fail, key(%s)", key.c_str());
                return NULL;
            }
            xassert(key == new_data_obj->get_key());//should be exactly same
            new_data_obj->set_hash_of_key(key_hash);
            new_data_obj->set_last_access_time(base::xtime_utl::gmttime_ms());
            
            if(cach_expired_after_seconds > 0)
            {
                //update cache
                base::xunknowobj_t* cache_container_ptr = new base::xunknowobj_t(key);
                cache_container_ptr->set(value);
                cache_container_ptr->set_hash_of_key(key_hash);
                cache_container_ptr->set_last_access_time(base::xtime_utl::gmttime_ms());
                cache_container_ptr->set_cache_expire_ms(cache_container_ptr->get_last_access_time() + (cach_expired_after_seconds << 10) );//convert to ms
                
                base::xdataobj_t* old_ptr = base::xatomic_t::xexchange(m_cache_objects[index], (base::xdataobj_t*)cache_container_ptr);
                if(old_ptr != NULL)
                    get_context()->delay_release_object(old_ptr);
                
                auto on_monitor_cache_function = [this](base::xcall_t & call, const int32_t cur_thread_id, const uint64_t timenow_ms)->bool{
                    const uint64_t _cache_expire_ms = call.get_param1().get_uint64();
                    const uint32_t _key_hash = (const uint32_t)call.get_param2().get_int64();
                    m_cache_timeout_map.insert(std::multimap<uint64_t,uint32_t>::value_type(_cache_expire_ms,_key_hash));
                    return true;
                };
                base::xcall_t asyn_call(on_monitor_cache_function,cache_container_ptr->get_cache_expire_ms(),(int64_t)key_hash,(base::xobject_t*)this);
                m_parent_book_ptr->send_call(asyn_call);
            }
            return new_data_obj;
        }
                
        bool xledgertable_t::try_clean_cache(const uint32_t key_hash, const std::string & key)
        {
            const int index = key_hash % enum_const_cache_max_objects_count;
            base::xdataobj_t* hit_object = m_cache_objects[index];
            if(hit_object != NULL)
            {
                if( hit_object->is_close() || (key_hash == hit_object->get_hash_of_key()) || (key == hit_object->get_key()) )
                {
                    base::xdataobj_t* old_ptr = base::xatomic_t::xcompare_exchange(m_cache_objects[index], hit_object, (base::xdataobj_t*)NULL);
                    if(old_ptr == hit_object)
                        get_context()->delay_release_object(hit_object);

                    return true;
                }
            }
            return false;
        }
    
        bool xledgertable_t::on_timer_fire(const int32_t thread_id, const int64_t timer_id, const int64_t current_time_ms, const int32_t start_timeout_ms, int32_t in_out_cur_interval_ms)
        {
            if(m_cache_timeout_map.empty())
                return true;
            
            const int64_t  current_gms_time = base::xtime_utl::gmttime_ms(current_time_ms);
            const uint32_t max_clean_items_count = enum_const_clean_max_objects_count; //Add max limit here
            uint32_t       expired_items_count = 0;
            std::multimap<uint64_t,uint32_t>::iterator expire_it = m_cache_timeout_map.begin();
            while(expire_it != m_cache_timeout_map.end())
            {
                //map or multiplemap are sorted as < operation as default
                if((uint64_t)current_gms_time < expire_it->first )
                    break;
                
                const uint32_t target_keyhash = expire_it->second; //copy hash value first
                std::multimap<uint64_t,uint32_t>::iterator old = expire_it; //just copy the old value
                ++expire_it;
                m_cache_timeout_map.erase(old);
                ++expired_items_count;
                
                if(target_keyhash > 0)
                {
                    const int index = target_keyhash % enum_const_cache_max_objects_count;
                    base::xdataobj_t* hit_object = m_cache_objects[index];
                    if(hit_object != NULL)
                    {
                        if( hit_object->is_close() || (hit_object->get_hash_of_key() == target_keyhash) )//matched
                        {
                            base::xdataobj_t* old_ptr = base::xatomic_t::xcompare_exchange(m_cache_objects[index], hit_object, (base::xdataobj_t*)NULL);
                            if(old_ptr == hit_object)
                                get_context()->delay_release_object(hit_object);
                        }
                    }
                }
                if(expired_items_count > max_clean_items_count)
                    break;
            }
            return true;
        }
        
    };// namespace ledger
};// namespace top
