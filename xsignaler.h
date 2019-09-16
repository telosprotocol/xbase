// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#pragma once

#include "xobject.h"
#include "xatom.h"
#include <atomic>

namespace top
{
    namespace base
    {
        //Note: here term "signaler" is different from system signal(that is sigset_t type)
        //xsignaler_t exchange signal between write and read thread safely,  may use most efficient way based on platform
        //xsignaler_t can only be read/recv_signal at single thread,it follow mulitple-produce and single-consume mode
        class xsignaler_t : public xobject_t
        {
        public:
            xsignaler_t();
            ~xsignaler_t ();
        protected:
            xsignaler_t(const int object_type);
        private:
            xsignaler_t(const xsignaler_t &);
            xsignaler_t & operator = (const xsignaler_t &);
        public:
            virtual bool  close(bool force_async = false) override;  //clean up resource
        public:
            int         send_signal();  //return 1 when signal send out
            int         recv_signal();  //return how many signal read out
            bool        has_signal();   //test whether has signal is pending,strong_check decide whether use atomic to test whether has signal or not;
            int         get_read_handle() {return m_r_handle;}
        protected:
            //note::need conver to block mode,then waiting for it
            int         wait_signal(int32_t timeout_); //return 0 means timeout, return 1 means has signal in,otherwise has error(check errno)
        private:
            //Returns -1 if fail to make the socket pair
            int         create_pair(int *r_handle, int *w_handle);
        protected:
            //  Underlying write & read file descriptor
            int         m_w_handle;
            int         m_r_handle;
        private:
            int         m_signal_pending;
        };
        
        //xiosignaler_t send notification to owner objects by xobject_t::on_signal_up & xobject_t::on_signal_close callback
        class xiosignaler_t : public xsignaler_t,public xiosink_t
        {
            enum
            {
                enum_max_owner_count    = 4,
            };
        public:
            //callback might be NULL,and may attach to add callback again later
            xiosignaler_t(xiothread_t & owner_thread,xobject_t * callback); //bind xiosignaler_t at thread of  owner_object
        protected:
            virtual ~xiosignaler_t();
        private:
            xiosignaler_t();
            xiosignaler_t(const xiosignaler_t &);
            xiosignaler_t & operator = (const xiosignaler_t &);
        public:
            virtual bool        attach(xobject_t * callback); //attach one more xobject_t owner to receive the callback from xiosignaler_t
            virtual bool        detach(xobject_t * callback); //detach one xobject_t owner/callback from signaler;
            virtual bool        start(); //start signaler to work
            virtual bool        close(bool force_async = false) override;  //stop signaler and clean up resource
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t type) override;
            
            bool                is_empty(); //test whether has any sink/callback for singaler
            int32_t             get_thread_id();
            uint64_t            get_time_now();
            uint64_t            update_time_now(); //trigger to refresh time
            xcontext_t*         get_context() const {return m_ptr_context;}
        protected: //return true when the event is handled
            //xiohandle_t attached into io-thread of the target thread(host) ,return true when the event is handled
            virtual bool        on_iohandle_attach(xfd_handle_t handle,const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            
            //xiohandle_t detached from io-thread of the target(host) thread,return true when the event is handled
            virtual bool        on_iohandle_detach(xfd_handle_t handle,const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms) override;//detach means it detach from io-thread but maybe the   fdhandle(socket) is still valid
            
            //handle is closed and inited by caller if error_code is 0
            virtual bool        on_iohandle_close(const int32_t error_code,xfd_handle_t handle,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            
            //readable event; return new fd_events_t if want change listened events,and b_handled indicate whether it is processed
            //when no-longer need this event set watchEvents to 0,which emove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_read(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            
            //writeable event;return new fd_events_t if want change listened; and b_handled indicate whether event is handled or not
            //when no-longer need this event set watchEvents to 0 which will remove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_write(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_msd) override;
        private:
            xobject_t*    m_pr_owner_objects[enum_max_owner_count]; //max receiver/owner for one signaler
            xiohandle_t*  m_ptr_raw_handle;
            xcontext_t*   m_ptr_context;     //point to the global context object
            int32_t       m_target_thread_id;
        };

    }; //end of namespace base
}; //end of namespace top
