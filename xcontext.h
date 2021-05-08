// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xtls.h"
#include "xobject.h"
#include "xendpoint.h"
#include "xrouter.h"

namespace top
{
    namespace base
    {
        class xrefcount_t;
        class xiothread_t;

        typedef std::function<xobject_t*(int type)> xnew_function_t;
        typedef xobject_t* (*xnew_function_ptr)(int);
         
        //global context by which communicated each other of all xiobjects
        class xcontext_t
        {
            friend class xiothread_t;
            friend class xnode_t;
            friend class xwrouter_t;
            friend class xobject_t;
            friend class xiobject_t;
        public:
            static bool          is_inited(); //determine whether xcontext_t inited
            static xcontext_t &  instance();  //init the global context if not exist
            static bool          register_xobject(enum_xobject_type type,xnew_function_ptr creator_func_ptr);
            static bool          register_xobject(xcontext_t & _context,enum_xobject_type type,xnew_function_ptr creator_func_ptr);
            static bool          register_xobject2(enum_xobject_type type,xnew_function_t creator_t);
            static bool          register_xobject2(xcontext_t & _context,enum_xobject_type type,xnew_function_t creator_t);
            static xobject_t*    create_xobject(enum_xobject_type type);
            static uint32_t      get_version_code(); //return bin version code for xbase

            enum enum_global_object_key
            {
                enum_global_object_key_invalid                      =  0,  //reserved for internal
                enum_global_xbase_object_key_shared_mem_pool        =  1,  //xmalloc need global memory object
                //internal reserved for [0,255]
                
                //must register xhash_t object for below types if need,refer xhash.h
                enum_global_object_key_hash_plugin                  =  20, //support external hash funtion,refer enum_xhash_type at xbase.h
                enum_global_object_key_debug_plugin                 =  21, //tracking mem & xiobject lifecycle
            
                enum_global_max_xbase_object_key                    =  64, //application may using below keys
                ////////////////////////////////xbase objects key end/////////////////////////////////////////

                //application objects key start                
                enum_global_max_key_id                 = 255
            };
            
            enum
            {
                enum_max_loopback_entry = 256,
                #if defined(__DEBUG_TEST_NET__)
                enum_max_nodes_count   = 1024,
                #else
                enum_max_nodes_count   = 32,
                #endif
                
                #if defined(__LINUX_PLATFORM__) || defined(__MAC_PLATFORM__)
                    enum_recap_timer_interval           = 2000,   //every 2 seconds
                    enum_recap_object_waiting_duration  = 10000,
                #else
                    enum_recap_timer_interval           = 1000,   //every 1 seconds
                    enum_recap_object_waiting_duration  = 3000,
                #endif
                enum_system_scan_interval               = 60000,  //every 60 seconds(1minutes) to scan cpu/memory/network speed etc
            };
        
            enum enum_debug_mode //define specific debug mode
            {
                enum_release_mode                = 0, //default
                enum_debug_mode_memory_check     = 1,
                enum_debug_mode_reference_check  = 2,
                enum_debug_mode_packet_check     = 4,
            };
        protected:
            xcontext_t(const int32_t process_id);
        protected: //not allow destory it ,because it is a global object
            virtual ~xcontext_t();
        private:
            xcontext_t();
            xcontext_t(const xcontext_t &);
            xcontext_t & operator = (const xcontext_t &);
            //static bool          init_context(xcontext_t * new_context); //init_context  fail if global instance already init before
        public:
            //global variable
            xobject_t*                      set_debug_plugin(xobject_t* _plugin_ptr)//return existing one
            {
                xassert(_plugin_ptr != nullptr);
                if(nullptr == _plugin_ptr)
                    return nullptr;
                
                xdbgplugin_t*  _dbg_plugin_ptr = (xdbgplugin_t*)_plugin_ptr->query_interface(enum_xobject_type_xdbgplugin);
                xassert(_dbg_plugin_ptr != nullptr);
                if(nullptr == _dbg_plugin_ptr)
                    return nullptr;
                
                return set_global_object(enum_global_object_key_debug_plugin,_plugin_ptr);
            }
            
            xobject_t*                      set_hash_plugin(xobject_t* _plugin_ptr)//return existing one
            {
                xassert(_plugin_ptr != nullptr);
                if(nullptr == _plugin_ptr)
                    return nullptr;
                
                xhashplugin_t*  _hash_plugin_ptr = (xhashplugin_t*)_plugin_ptr->query_interface(enum_xobject_type_xhashplugin);
                xassert(_hash_plugin_ptr != nullptr);
                if(nullptr == _hash_plugin_ptr)
                    return nullptr;

                return set_global_object(enum_global_object_key_hash_plugin,_plugin_ptr);
            }
            
            xobject_t*                      set_global_object(enum_global_object_key global_key_id,xobject_t* pPtr)//return existing one
            {
                if( (int)global_key_id > enum_global_max_key_id) //exception protect
                    return NULL;
                
                if(pPtr != NULL)//add reference to hold this object
                    pPtr->add_ref();
                
                xobject_t* old_object = m_global_object_slots[global_key_id];
                m_global_object_slots[global_key_id] = pPtr;
                
                _ATOMIC_FULL_MEMORY_BARRIER();//refresh cpu cache and execute order
                
                //note: caller response to release the returned pointer
                return old_object;
            }
            inline xobject_t*               get_global_object(enum_global_object_key global_key_id){return m_global_object_slots[global_key_id];}
            inline xtls_t*                  get_xtls_instance() const {return m_ptr_tls_instance;}//manage thread local objects
            int32_t                         get_total_threads(); //return how many xiothread_t are created
            xiothread_t*                    get_thread(); //return current execture thread ' xiothread_t ptr
            xiothread_t*                    get_thread(const int32_t query_thread_id);//query which xiothread_t has query_thread_id
            xiothread_t*                    find_thread(const int thread_type,bool use_dedicated_thread);//find a thread with specified type,use_dedicated_thread decide whether ask full-match when search
            bool                            get_aes_keyid_range(uint8_t & min_key_id,uint8_t & max_key_id); //available key id list is be [min_key_id,max_key_id);
            bool                            get_buildin_aeskey(const uint8_t key_id, uint8_t aes_128bit_key[16]);
            inline int32_t                  get_process_id() const   {return m_process_id;} //return logic process id
            inline enum_xprocess_run_mode   get_process_cpu_mode() const {return m_process_cpu_mode;}
            inline int32_t                  get_current_thread_id()  {return get_xtls_instance()->get_cur_thread_id(false);} //return current execute thread id
            inline void                     set_process_id(const int32_t process_id){m_process_id = process_id;}
            //xcontext/xbase has master build-in keys that are rotated time by time

            const std::string               hash(const std::string & input,enum_xhash_type type); //redirect xhash object
            
        public: //debug use only
            inline int32_t    get_debug_modes() const {return m_debug_modes;}
            int32_t    set_debug_modes(const int32_t modes); //return last setting,refer enum_debug_mode
            
        protected:
            bool       on_object_create(xobject_t* target);
            bool       on_object_destroy(xobject_t* target);
            
            bool       on_object_addref(xobject_t* target);
            bool       on_object_releaseref(xobject_t* target);
        public:
            //put to queue and release at next time when term(recap) thread wake up
            virtual  bool       delay_release_object(xrefcount_t* object_release);
            //allow trigger to clean the delayed objects of delay_release_object
            virtual bool        on_timer_recap(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms);
            
        public: // multiple thread safe,return error code if fail, refer enum_xerror_code
            //Note: signal/post api execute the xcall_t at target thread through it's own mailbox or the thread'mailbox
            //send() is 100% asynchronize,it ensure to execute call at target thread as the order,
            virtual int32_t     send_call(const uint32_t target_thread_id,xcall_t & call); //send cmd and wakeup target io-thread,can be called at any thread
            //dispatch() might execute immediately if now it is at target thread,otherwise do send()
            virtual int32_t     dispatch_call(const uint32_t target_thread_id,xcall_t & call);
            
            //post is the optmization for larget amount xcall_t who need to deliver to target thread as bunch mode
            virtual int32_t     post_call(const uint32_t target_thread_id,xcall_t & call);//just pass data ,not singal to wake up thread immidiately
            virtual int32_t     signal_call(const uint32_t target_thread_id);             //just wakeup the io-thread
            
        public:// multiple thread safe, handle packet
            int32_t   send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_end);
 
            int32_t   recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_end);
            
        public://only for Linux OS right now,and just return estimated value of most recent
            void                    monitor_system_metric(bool enable_or_disable);//true for enable(default) and false for disable
            
            int32_t                 get_sys_cpu_load();             //0-100
            int32_t                 get_sys_mem_load();             //0 -100
            int32_t                 get_sys_net_in_speed();         //Kbits/s
            int32_t                 get_sys_net_in_throughout();    //packets/s
            int32_t                 get_sys_net_in_drop();          //packets/s
            int32_t                 get_sys_net_out_speed();        //Kbits/s
            int32_t                 get_sys_net_out_throughout();   //packets/s
            int32_t                 get_sys_net_out_drop();         //packets/s
            
        protected:
            virtual int32_t     register_thread(xiothread_t * io_thread_obj); //return thread_id associated with thread object
            virtual bool        unregister_thread(const int32_t thread_id);   //return false if not find related thread;
            
            virtual bool        register_node(const xnode_t* node_ptr);
            virtual bool        unregister_node(const xnode_t* node_ptr);
            
            virtual bool        register_loopback_endpoint(const xendpoint_t* endpoint_ptr);
            virtual bool        unregister_loopback_endpoint(const xendpoint_t* endpoint_ptr);
            
            void                set_processs_cpu_mode(enum_xprocess_run_mode new_mode){m_process_cpu_mode = new_mode;}
        private:
            xiothread_t*        find_thread_nolock(const uint32_t request_thread_type,bool use_dedicated_thread);
        private:
            void*                    m_recap_objects_queue;
            void*                    m_recap_timer;
            xtls_t*                  m_ptr_tls_instance;

            int32_t                  m_process_id;  //process id of current application(logic process id instead of system process id)
            enum_xprocess_run_mode   m_process_cpu_mode;//default it is enum_xprocess_run_mode_single_cpu_core
            std::recursive_mutex     m_lock;            //general lock for xcontext_t
            std::recursive_mutex     m_recap_lock;      //dedicate lock for recap/recycle objects
        private:
            int32_t                  m_recap_thread_id;  //thread id to recycle objects
            int32_t                  m_max_node_index;   //index indicate what is max offset/index at m_nodes array
            xnode_t*                 m_nodes[const_max_xnetwork_types_count];
            xendpoint_t*             m_lan_routers[const_max_xnetwork_types_count];
            xendpoint_t*             m_wan_routers[const_max_xnetwork_types_count];
            xendpoint_t*             m_vnodes[enum_max_nodes_count];//one physical node(instance) may create multiple role(node)
            xendpoint_t*             m_loopback_bus[enum_loopback_network_max_entry+1];//present 0.0.x network
        private:
            //total allow xtls_t::enum_max_thread_count threads object hold
            //note:once create xiothread_t object ptr never be release until whole process quit, even call xiothread_t close to release some resource. which bring much better performance
            xiothread_t*    m_global_iothread_slots[enum_max_xthread_count + 1];
            //global shared objects
            xobject_t*      m_global_object_slots[enum_global_max_key_id + 1];
        private:
            std::multimap<uint64_t, xrefcount_t*> m_recap_waiting_objects;
        private:
            int32_t     m_debug_modes;              //refer
        private: //system metric information
            int32_t     m_sys_cpu_load;             //0-100
            int32_t     m_sys_mem_load;             //0-100
            
            int32_t     m_sys_net_in_speed;         //kbits /s
            int32_t     m_sys_net_in_throughout;    //packets/s
            int32_t     m_sys_net_in_drop;          //packets/s
            
            int32_t     m_sys_net_out_speed;        //kbits /s
            int32_t     m_sys_net_out_throughout;   //packets/s
            int32_t     m_sys_net_out_drop;         //packets/s
        private:
            uint64_t    m_last_scan_timestamp;
            uint64_t    m_last_cpu_used_since_boot;
            uint64_t    m_last_cpu_idle_since_boot;
            
            uint64_t    m_last_sys_rx_bytes;        //received bytes
            uint64_t    m_last_sys_rx_packets;      //received packets
            uint64_t    m_last_sys_rx_drop_packets; //error or dropped packets when recv
            uint64_t    m_last_sys_tx_bytes;        //sent bytes
            uint64_t    m_last_sys_tx_packets;      //sent packets
            uint64_t    m_last_sys_tx_drop_packets; //error or dropped packets when send
            
            std::string m_default_net_interface_name;
        };
        
    #define IMPL_REGISTER_OBJECT(T) void T::register_object(xcontext_t & context) { \
        auto lambda_new_func = [](const int type)->xobject_t*{ \
            return new T(); \
        }; \
        xcontext_t::register_xobject2(context,(enum_xobject_type)T::enum_obj_type,lambda_new_func); \
        }\
        
        template<typename T>
        class auto_new_registor
        {
        public:
            auto_new_registor()
            {
                _register(xcontext_t::instance());
            }
        public:
            static void _register(xcontext_t & _context)
            {
                //T must support default construction function
                auto lambda_new_func = [](const int type)->xobject_t*{
                    return new T();
                };
                //T must have  enum_type definition
                xcontext_t::register_xobject2(_context,(enum_xobject_type)T::enum_obj_type,lambda_new_func);
            }
        };
        
        template<typename T,const int _type>
        class auto_new_registor2
        {
        public:
            auto_new_registor2()
            {
                _register(xcontext_t::instance());
            }
        public:
            static void _register(xcontext_t & _context)
            {
                //T must support default construction function
                auto lambda_new_func = [](const int type)->xobject_t*{
                    return new T();
                };
                //T must have  enum_type definition
                xcontext_t::register_xobject2(_context,(enum_xobject_type)_type,lambda_new_func);
            }
        };
    };//end of namespace base
}; //end of namespace top
