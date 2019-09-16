// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <map>
#include "xendpoint.h"
#include "xtimer.h"

namespace  top
{
    namespace base
    {
        //xservice_t represent a service at special xip address
        //note: each xservice_t has own mailbox, databox,and timer(every 1 second)
        class xservice_t : public xendpoint_t,public xtimersink_t
        {
            friend class xcontext_t;
            friend class xnode_t;
        public:
            enum enum_service_mode
            {
                //directly process packet at io thread even it is not same as service ' thread
                enum_service_mode_direct_handle           = 0,
                //guanrente every packet is processed at host thread of service
                enum_service_mode_async_handle            = 1, //without lock but async post the read to host thread where do realy read
            };
        protected:
            xservice_t(xcontext_t & context,const int32_t thread_id,xnode_t & owner,enum_xnetwork_type net_type,enum_xip_service_id service_id,enum_service_mode mode = enum_service_mode_direct_handle);
            virtual ~xservice_t();
        private:
            xservice_t();
            xservice_t(const xservice_t &);
            xservice_t & operator = (const xservice_t &);
            
        protected: //subclass may overwrite this function to fill own logic,here just give default empty implementation
            virtual int32_t     on_packet_handle(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_end) override{return enum_xcode_successful;}
            
            //return true if the event is already handled,return false to stop timer as well,
            //start_timeout_ms present when the duration of first callback
            //in_out_cur_interval_ms carry back the new setting() for timer,stop timer if <= 0 means; otherwise ask timer repeat with this new interval
            virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) override {return true;}
            
        protected: //following api are from xtimersink_t, and  be called from timer thread,
            virtual bool        on_object_close() override;
            
            virtual bool        on_timer_start(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override;    //attached into io-thread
            virtual bool        on_timer_stop(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override;  //detach means it detach from io-thread
        private:
            //just allow xcontext_t call send
            virtual int32_t     send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end) override;
            
            //just allow xrouter call recv
            virtual int32_t     recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_end) override;
            
            virtual  bool      on_databox_open(xpacket_t & packet,int32_t cur_thread_id, uint64_t time_now_ms) override;
        private:
            xtimer_t*         m_raw_timer;
            enum_service_mode m_handle_mode;
        };
        
        //provide basic function to monitor sockets, endpoint by checking keep-alive
        class xendpoint_monitor_t : public xtimersink_t
        {
            typedef std::map <uint64_t,xendpoint_t*>       std_checkalive_map;  //map address(low64bit) and xendpoint_t,and check if it alive
            typedef std::multimap<uint64_t, xendpoint_t*>  std_checkexpire_map; //map expired time ->xendpoint_t,and check whether it is expired
            const int  enum_batch_check_endpoint_count = 512;
        protected:
            //create dedicated monitor thread if thread_id is 0,check every endpoint every monitor_interval_ms
            xendpoint_monitor_t(xcontext_t & context,int32_t monitor_thread_id,int32_t monitor_interval_ms);
            virtual ~xendpoint_monitor_t();
        private:
            xendpoint_monitor_t();
            xendpoint_monitor_t(const xendpoint_monitor_t &);
            xendpoint_monitor_t & operator = (const xendpoint_monitor_t &);
        public:
            bool    add_to_keepalive_monitor(xendpoint_t * endpoint);
            bool    remove_from_keepalive_monitor(xendpoint_t * endpoint);
            bool    add_to_expire_monitor(xendpoint_t * endpoint,const uint64_t expired_time);
            bool    remove_from_expire_monitor(xendpoint_t * endpoint,const uint64_t expired_time = 0);//expired_time should be same as when call add_to_expire_monitor,or pass 0 do search fully
        protected:
            virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) override;
        protected:
            virtual bool        on_timer_start(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override {return true;}    //attached into io-thread
            virtual bool        on_timer_stop(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override {return true;}  //detach means it detach from io-thread
        private:
            xtimer_t*           m_monitor_timer;
            xiothread_t*        m_monitor_thread;
            uint64_t            m_last_checkalive_addr;
            std_checkalive_map  m_checkalive_endpoints;
            std_checkexpire_map m_checkexpire_endpoints;
        };
        

        typedef std::function<xsocket_t*(xfd_handle_t handle,xsocket_property & property,int32_t thread_id,uint64_t timenow_ms,void* cookie) > accept_socket_callback_t; //for physical socket with dedicated handle

        //xlisten_service_t listen TCP/UDP ,and manage accepted sockets
        class  xlisten_service_t : public xservice_t,public xendpoint_monitor_t
        {
        protected:
            xlisten_service_t(xcontext_t & context,const int32_t thread_id,xnode_t & owner,enum_xnetwork_type net_type,enum_xip_service_id service_id,enum_service_mode mode = enum_service_mode_direct_handle);
            virtual ~xlisten_service_t();
        private:
            xlisten_service_t();
            xlisten_service_t(const xlisten_service_t &);
            xlisten_service_t & operator = (const xlisten_service_t &);
        public:
            //return the object id of listening socket
            //virtual int64_t xtcp_listen(const std::string listen_ip,int listen_port,void* cookie,xlisten_xtcp_callback_t callback);
            virtual bool    stop_listen(); //stop listen every one
            virtual bool    stop_listen(int64_t listen_socket_id);//stop the specified socket
        private:
            xiothread_t*            m_listen_thread; //dedicated thread to listen socket
            std::vector<xsocket_t*> m_listeners; //listening sockets
        };
    }
}

