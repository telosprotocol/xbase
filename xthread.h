// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xobject.h"
#include "xlock.h"
#include "xtimer.h"
#include "xmailbox.h"
#include "xcontext.h"


namespace top
{
    namespace base
    {
        //note: general implement for thread
        class xthread_t : public xobject_t
        {
            static void* static_thread_entry(void *arg_);
        public:
            xthread_t(xcontext_t & _context);
        protected:
            virtual ~xthread_t();
        private:
            xthread_t();
            xthread_t(const xthread_t & obj);
            xthread_t & operator = (const xthread_t &);
        public:
            virtual bool        start();  //start thread
            virtual void*       query_interface(const int32_t type);
            virtual int32_t     get_load()   const {return m_load;}
            virtual bool        is_running() const {return (m_running != 0);}
            virtual bool        is_stopped() const {return (m_stopped != 0);}
            xcontext_t*         get_context()     const {return m_ptr_context;}
            virtual bool        wait_to_ready(const int32_t time_out_ms);  //wait the until thread really started
            virtual bool        wait_to_close(const int32_t time_out_ms);  //wait the until thread really closed
        protected:
            virtual void        enter_loop();
            virtual void        loop();
            virtual void        leave_loop();
            virtual bool        register_thread();  //let subclass register to sowewhere when thread completely started
            virtual bool        unregister_thread(); //let subclass unregister when thread completely stopped
        private:
            virtual void        on_thread_run();
        private:
            xcond_event_t       m_cond_event;
        protected:
            int64_t             m_system_id; //thread identify at system scope
            int32_t             m_load;  //according the thread_callback frequent to estimate how busy
            uint8_t             m_running;
            uint8_t             m_stopped;
        private:
            uint8_t             m_started;
            uint8_t             m_padding8;
            xcontext_t*         m_ptr_context;       //point to global context object
            void*               m_thread_handle;     //for posix thread
            void*               m_os_padding;        //padding for m_thread_handle
        };

        //xiothread_t is raw thread of xbase,please xworker_t for easy case
        class xiothread_t : public xthread_t
        {
            friend class xiobject_t;
        public:
            enum enum_xthread_type
            {
                enum_xthread_type_general    = 0,        //normal thread
                
                enum_xthread_type_io         = 0x001,     //io thread(for file/socket/pipe/event/command io)
                enum_xthread_type_listener   = 0x002,     //tcp listen thread
                enum_xthread_type_connector  = 0x004,     //connect thread
                enum_xthread_type_recape     = 0x008,     //recap(clean/delete object resource) thread
                
                enum_xthread_type_monitor    = 0x010,     //monitor purpose
                enum_xthread_type_worker     = 0x020,     //execute heavy job
                enum_xthread_type_service    = 0x040,     //thread for service
                enum_xthread_type_consensus  = 0x080,     //thread for consensus

                enum_xthread_type_manage     = 0x100,     //internal manage-use purpose,to using this thread exchange information
                enum_xthread_type_routing    = 0x200,     //internal router and switcher use
                enum_xthread_type_rpc        = 0x400,     //internal rpc-call thread
                enum_xthread_type_voice      = 0x500,     //internal voice thread
                enum_xthread_type_video      = 0x600,     //internal video thread
                enum_xthread_type_tunnel     = 0x700,     //internal xtunnel thread
                enum_xthread_type_gateway    = 0x800,     //internal shared by all net gateway
                enum_xthread_type_vpn        = 0x900,     //internal vpn(tun) dedicated thread
                enum_xthread_type_db         = 0xa00,     //internal DB use only
                
                enum_xthread_type_private    = 0x1000,   //dedicated thread: can not be shared by others
                
                enum_xthread_type_customize_start = 0x2000,   //application may define own type over this point
                
                enum_xthread_type_all       =  0xFFFF
            };
        public:
            static xiothread_t*   create_thread(xcontext_t & _context,const int32_t thread_type,const int32_t time_out_ms);//thread_type refer enum_xthread_type
            static xiothread_t*   async_create_thread(xcontext_t & _context,const int32_t thread_type); //return immediately without waiting
        public:
            xiothread_t(xcontext_t & _context,const int thread_type);//refer enum_xthread_type
        protected:
            virtual ~xiothread_t();
        private:
            xiothread_t();
            xiothread_t(const xiothread_t &);
            xiothread_t & operator = (const xiothread_t &);
        public:
            virtual int32_t     add_ref() override;
            virtual int32_t     release_ref() override;
            virtual bool        close(bool force_async = true) override;
            virtual void*       query_interface(const int32_t type) override;
            virtual void*       get_io_loop() const = 0;  //return the raw libuv'io_loop
            
            //The timestamp increases monotonically from some arbitrary point in time,so just use relative compare(duration = now-lasttime). and the time is just updated as every loop iterate, so the may have max hundreds milliseconds offset
            //for acureate time to use xtime_utl:timenowms()
            virtual uint64_t    get_time_now()    const = 0;//as milliseond level,update it at every loop
            virtual uint64_t    update_time_now() const = 0; //trigger refresh time to more accurately and return latest time now.carefully: it ask call under this thread
            int32_t             get_thread_id()   const {return m_thread_id;} //return real thread' id
            int                 get_thread_type() const {return m_thread_type;}
        public:
            //multiple thread safe,return error code if fail, refer enum_xerror_code
            //Note: signal/post api execute the xcall_t at target thread through it's own mailbox or the thread'mailbox
            //send() is 100% asynchronize,it ensure to execute call at target thread as the order,
            virtual int32_t     send_call(xcall_t & call,int32_t cur_thread_id = 0);       //send cmd and wakeup target io-thread,can be called at any thread
            //dispatch() might execute immediately if now it is at target thread,otherwise do send()
            virtual int32_t     dispatch_call(xcall_t & call,int32_t cur_thread_id = 0);
            
            //post is the optmization for larget amount xcall_t who need to deliver to target thread as bunch mode
            virtual int32_t     post_call(xcall_t & call,int32_t cur_thread_id = 0);       //just pass data ,not singal to wake up thread immidiately
            virtual int32_t     signal_call(int32_t cur_thread_id = 0); //wakeup the io-thread of this io object,and handle pending xcall_t
            
            virtual int32_t     count_calls(int64_t & total_in, int64_t & total_out);//count pending calls at  queue
            
        public://create related io object from iothread
            virtual xtimer_t*   create_timer(xtimersink_t * event_receiver) = 0;
            
            //rawHandle can be any file_description that support io_event
            virtual xiohandle_t* create_io_handle(xfd_handle_t rawHandle,xiosink_t * event_receiver) = 0;

        public: //allow send/post/dispatch general lambda function to execute
   
        protected:
            virtual void        enter_loop() override;
            virtual void        loop() override;
            virtual void        leave_loop() override;
            virtual bool        register_thread() override;  //let subclass register to sowewhere when thread completely started
            virtual bool        unregister_thread() override; //let subclass unregister when thread completely stopped
            //on_object_close be called when close command processed at host thread,logic flow: Caller(Thread#A)->Close()->Wake this object thread(B)->clean and then execute: on_object_close
            virtual bool        on_object_close(); //notify the subclass the object is closed
            inline xmailbox_t*  get_mailbox() const {return m_ptr_mailbox;}
        private:
            xiosignaler_t*      m_ptr_signaler;      //each thread has own
            xmailbox_t*         m_ptr_mailbox;       //each thread has own mailbox
            int32_t             m_thread_id;         //logic thread id instead of system'pthread
            int32_t             m_thread_type;       //different thread type
        };

        //worker thread that has own dedicated timer
        class xworker_t : public xobject_t,public xtimersink_t
        {
        public:
            //-1 of waiting_for_ms means wait until thread is completely create
            xworker_t(xcontext_t & context,const int32_t thread_types,const int32_t waiting_for_ms = -1);//thread_type refer enum_xthread_type
        protected:
            virtual ~xworker_t();
        private:
            xworker_t();
            xworker_t(const xworker_t & );
            xworker_t & operator = (const xworker_t &);
        public:
            operator xiothread_t*(){return m_raw_thread_ptr;}//convertor
            
            xcontext_t &        get_context() {return m_context;}
            int32_t             get_thread_id() const {return m_raw_thread_ptr->get_thread_id();}
            
            int                 send_call(xcall_t & task){return m_raw_thread_ptr->send_call(task);}
            //close and stop workers,then call release_ref() to clean up resource
            //must call close before release object,otherwise object never be cleanup
            virtual bool        close(bool force_async = true) override;
            
        public://thread timer function
            //if timeout_ms is zero, the callback fires on the next event loop iteration. If repeat is non-zero, the callback fires first after timeout milliseconds and then repeatedly after repeat milliseconds. the return error code  refer enum_error_code
            int32_t             start_timer(const int32_t timeout_ms,const int32_t repeat_interval_ms);
            int32_t             stop_timer(); //after stop, may call start again
            
        private: //following api are from xtimersink_t, and  be called from timer thread
            
            //return true if the event is already handled,return false to stop timer as well,
            //start_timeout_ms present when the duration of first callback
            //in_out_cur_interval_ms carry back the new setting() for timer,stop timer if <= 0 means; otherwise ask timer repeat with this new interval
            virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) override;
            
            virtual bool        on_timer_start(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override;    //attached into io-thread
            
            virtual bool        on_timer_stop(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override;  //detach means it detach from io-thread
            
        protected://on_object_close be called from it's own thread ,when xworker_t close
            virtual bool        on_object_close();
            
        private:
            xiothread_t *       m_raw_thread_ptr;
            xtimer_t*           m_raw_timer_ptr;
            xcontext_t &        m_context;
        };
        
        //just provide abstract interface for worker pool
        class xworkerpool_t : public xobject_t
        {
        public:
            static std::string  name(); //"xworkerpool"
        protected:
            xworkerpool_t();
            virtual ~xworkerpool_t();
        private:
            xworkerpool_t(const xworkerpool_t &);
            xworkerpool_t & operator=(const xworkerpool_t &);
        public:
            virtual uint32_t        get_count() { return 0; } //how many threads under runtime
            virtual xworker_t*      get_thread(const uint32_t thread_identify) { return NULL; }// must ensure thread_identify < get_count
            virtual std::vector<uint32_t> get_thread_ids() = 0; //return all thread ids
            
            virtual std::string     get_obj_name() const override {return name();}
        public:
            virtual int             send_call(xcall_t & task) = 0;
            //stop and close all threads
            virtual bool            close(bool force_async = true) override {return false;}
        };
        
        template<const uint8_t  _max_threads_count,typename  _xworker_object_t = xworker_t>
        class xworkerpool_t_impl : public xworkerpool_t
        {
        public:
            xworkerpool_t_impl(xcontext_t & _context,const int raw_thread_types = xiothread_t::enum_xthread_type_worker | xiothread_t::enum_xthread_type_private)
            {
                for (int i = 0; i < _max_threads_count; ++i)
                {
                    m_worker_threads[i] = new _xworker_object_t(_context,raw_thread_types);
                }
            }
        protected:
            virtual ~xworkerpool_t_impl()
            {
                for (int i = 0; i < _max_threads_count; ++i)
                {
                    if (m_worker_threads[i] != NULL)
                    {
                        m_worker_threads[i]->close();
                        m_worker_threads[i]->release_ref();
                    }
                }
            }
        private:
            xworkerpool_t_impl();
            xworkerpool_t_impl(const xworkerpool_t_impl &);
            xworkerpool_t_impl & operator=(const xworkerpool_t_impl &);
        public:
            virtual uint32_t get_count() override //how many threads under runtime
            {
                return _max_threads_count;
            }
            
            virtual xworker_t* get_thread(const uint32_t thread_identify) override// must ensure thread_identify < get_threads_count
            {
                #ifdef DEBUG
                xassert(thread_identify < _max_threads_count);
                #endif
                return m_worker_threads[thread_identify]; //dose not check for performance reasone
            }
            
            virtual std::vector<uint32_t> get_thread_ids() override //return all thread ids
            {
                std::vector<uint32_t> _thread_ids;
                for (int i = 0; i < _max_threads_count; ++i)
                {
                    if (m_worker_threads[i] != NULL)
                    {
                        _thread_ids.push_back(m_worker_threads[i]->get_thread_id());
                    }
                }
                return _thread_ids;
            }
            
            virtual int   send_call(xcall_t & task) override
            {
                return m_worker_threads[task.get_taskid() % _max_threads_count]->send_call(task);
            }
            
            //stop and close all threads
            virtual bool   close(bool force_async = true) override
            {
                for (int i = 0; i < _max_threads_count; ++i)
                {
                    if (m_worker_threads[i] != NULL)
                    {
                        m_worker_threads[i]->close(force_async);
                    }
                }
                return true;
            }
        private:
            xworker_t * m_worker_threads[_max_threads_count];
        };
    };//end of namespace of base
}; //end of namespace of top
