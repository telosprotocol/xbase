// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase.h"
#include "xsignaler.h"
#include "xpacket.h"

namespace top
{
    namespace base
    {
        class xcontext_t;
        
        //Note: xiopipe_t allow be called from multiple-thread to write, but single-thread read.
        //Note:  the write just guagreente the order from same thread
        //Design requirement:  multiple-thread safe + high performance for multiple writer with single reader case
        //xiopipe_t live at same thread of the owner object
        class xiopipe_t : public xobject_t
        {
        protected:
            enum
            {
                #if defined(XCPU_ARCH_X86) || defined(__MAC_PLATFORM__) || defined(__LINUX_PLATFORM__)
                enum_mailbox_lowwater          = 512,   //low water for left unprocessed items
                enum_mailbox_highwater         = 4096,  //high water for left unprocessed items
                
                enum_mailbox_min_batch_read    = 512,   //default value to read out events at one time
                enum_mailbox_avg_batch_read    = 2048,  //default value to read out events at one time
                enum_mailbox_max_batch_read    = 4096,  //default value to read out events at one time
                
                #else //mobile
                enum_mailbox_lowwater          = 128,   //low water for left unprocessed items
                enum_mailbox_highwater         = 512,   //high water for left unprocessed items
                
                enum_mailbox_min_batch_read    = 64,    //default value to read out events at one time
                enum_mailbox_avg_batch_read    = 512,   //default value to read out events at one time
                enum_mailbox_max_batch_read    = 1024,  //default value to read out events at one time
                #endif
            };
        protected:
            //callback might be NULL
            xiopipe_t(xiosignaler_t & init_signaler_object,xiobject_t * callback,const int32_t min_batch_read,const int32_t max_batch_read,enum_xobject_type obj_type);
            virtual ~xiopipe_t();
        private:
            xiopipe_t();
            xiopipe_t(xiopipe_t & );
            xiopipe_t & operator = (const xiopipe_t &);
        public:
            virtual bool        start();    //init mailbox and start it
            virtual bool        close(bool force_async) override;
        protected:
            xiosignaler_t*      get_signaler() const {return m_signaler_object;}
            xiobject_t*         get_callback() const {return m_callback_object;}
            xcontext_t*         get_context()  const {return m_ptr_context;}
            inline int32_t      get_status()   const {return m_status;}
            inline int32_t      get_thread_id()const {return m_owner_thread_id;}//0 means no-bind to any thread yet
        protected:
            xcontext_t*         m_ptr_context;
            xiosignaler_t*      m_signaler_object;      //the associated xiosignaler_t object
            xiobject_t*         m_callback_object;      //who want get callback from this mailbox
            int32_t             m_owner_thread_id;      //mailbox running at which thread
            int32_t             m_status;               //
            int32_t             m_max_batch_process_count;
            int32_t             m_min_batch_read;       //min amount to allow to read out events at one time
            int32_t             m_max_batch_read;       //max amount to allow to read out events at one time
        };
        
        //allow post & execute xcall_t at target thread
        class xmailbox_t : public xiopipe_t
        {
            friend class xiobject_t;
            friend class xiothread_t;
        protected:
            //callback might be NULL
            xmailbox_t(xiosignaler_t & init_signaler_object,xiobject_t * callback,const int32_t min_batch_read = enum_mailbox_min_batch_read,const int32_t max_batch_read = enum_mailbox_max_batch_read,const int32_t max_queue_size = -1);
            virtual ~xmailbox_t();
        private:
            xmailbox_t();
            xmailbox_t(xmailbox_t & );
            xmailbox_t & operator = (const xmailbox_t &);
        public:
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t type) override;
        public:
            //multiple thread safe,return error code if fail, refer enum_xerror_code
            //Note: signal/post api execute the xcall_t at target thread through it's own mailbox or the thread'mailbox
            //send() is 100% asynchronize,it ensure to execute call at target thread as the order,
            virtual int32_t     send_call(xcall_t & call,int32_t cur_thread_id = 0);       //send cmd and wakeup target io-thread,can be called at any thread
            //dispatch() might execute immediately if now it is at target thread,otherwise do send()
            virtual int32_t     dispatch_call(xcall_t & call,int32_t cur_thread_id = 0);
            
            //post is the optmization for larget amount xcall_t who need to deliver to target thread as bunch mode
            virtual int32_t     post_call(xcall_t & call,int32_t cur_thread_id = 0);       //just pass data ,not singal to wake up thread immidiately
            virtual int32_t     signal_call(int32_t cur_thread_id = 0); //wakeup the io-thread of this io object,and handle xcall_t
            
            virtual int32_t     count_calls(int64_t & total_in, int64_t & total_out);//count pending calls at  queue
        private:
            //xiobject may bind a xiosignal object that may trigger and wakeup the host thread when need
            //signal is going close when receive error_code as enum_xerror_code_close
            virtual  bool       on_signal_up(int32_t error_code,int32_t cur_thread_id, uint64_t time_now_ms) override;
            int32_t             process_commands(const int timeout_ms,const uint32_t max_batch_process_count,const int32_t cur_thread_id,const uint64_t timenow_ms);
        private:
            void*               m_calls_queue;
        };
        
        //allow post xpacket at target thread
        class xdatabox_t : public xiopipe_t
        {
            friend class xiobject_t;
            friend class xiothread_t;  
        protected:
            //callback must be valid
            xdatabox_t(xiosignaler_t & init_signaler_object,xiobject_t * callback,const int32_t min_batch_read = enum_mailbox_min_batch_read,const int32_t max_batch_read = enum_mailbox_max_batch_read,const int32_t max_queue_size = -1);
            virtual ~xdatabox_t();
        private:
            xdatabox_t();
            xdatabox_t(xdatabox_t & );
            xdatabox_t & operator = (const xdatabox_t &);
        public:
            //caller respond to cast (void*) to related  interface ptr
            virtual void*       query_interface(const int32_t type) override;
            
            //multiple thread safe,return error code if fail, refer enum_xerror_code
            //Note: signal/post api execute the xcall_t at target thread through it's own mailbox
            //send() is 100% asynchronize,it ensure to execute call at target thread as the order,
            virtual int32_t     send_packet(xpacket_t & packet,int32_t cur_thread_id = 0);       //send packet and wakeup target io-thread,can be called at any thread
            //dispatch() might execute immediately if now it is at target thread,otherwise do send()
            virtual int32_t     dispatch_packet(xpacket_t & packet,int32_t cur_thread_id = 0) ;
            
            //post is the optmization for larget amount xpacket_t who need to deliver to target thread as bunch mode
            virtual int32_t     post_packet(xpacket_t & packet,int32_t cur_thread_id = 0); //just pass data ,not singal to wake up thread immidiately
            virtual int32_t     signal_packet(int32_t cur_thread_id = 0);  //wakeup the io-thread of this io object and handle data queue
            
            virtual int32_t     count_packets(int64_t & total_in, int64_t & total_out);//count pending packets at  queue
        private:
            //xiobject may bind a xiosignal object that may trigger and wakeup the host thread when need
            //signal is going close when receive error_code as enum_xerror_code_close
            virtual  bool      on_signal_up(int32_t error_code,int32_t cur_thread_id, uint64_t time_now_ms) override;
            //can only be called at reader thread
            int32_t            process_packets(const int timeout_ms,const uint32_t ask_max_batch_process_count,const int32_t cur_thread_id,const uint64_t timenow_ms);
        private:
            void*               m_packets_queue;
        };
    } //end of namespace base
} //end of namespace top
