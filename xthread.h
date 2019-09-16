// Copyright (c) 2017-2018 Telos Foundation & contributors
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
        };

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
                
                enum_xthread_type_private    = 0x1000,   //dedicated thread: can not be shared by others
                
                enum_xthread_type_customize_start = 0x2000,   //application may define own type over this point
                
                enum_xthread_type_all       =  0xFFFF
            };
        public:
            static xiothread_t*   create_thread(xcontext_t & _context,const int32_t thread_type,const int32_t time_out_ms);
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

    };//end of namespace of base
}; //end of namespace of top
