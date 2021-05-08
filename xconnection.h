// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xendpoint.h"
#include "xsocket.h"

namespace top
{
    namespace base
    {
        //xconnection_t at Layer 3(XIP layer) and xsocket_t work at Layer2(Link),each connection bind one socket
        class xconnection_t : public xendpoint_t
        {
            enum
            {
                #ifdef __DEBUG_SOCKET_WITHOUT_TIMEOUT_BOTHER__
                enum_idle_time_out     = 150000, //timeout after idle 150 seconds
                #else
                enum_idle_time_out     = 15000,   //timeout after idle 15 seconds
                #endif
            };
        public:
            //child is
            xconnection_t(xcontext_t & _context,const int32_t thread_id,xendpoint_t * parent,xsocket_t* child);
            virtual ~xconnection_t();
        private:
            xconnection_t();
            xconnection_t(const xconnection_t &);
            xconnection_t & operator = (const xconnection_t &);
        public:
            virtual int         is_alive(uint64_t timenow_ms) override;
            virtual void*       query_interface(const int32_t type) override;//caller respond to cast (void*) to related  interface ptr
        public:
            //if cur_thread_id 0 xsocket_t do query current thread id again. same for timenow_ms.
            //return errorcode -refer  enum_error_code,return enum_code_successful if the packet write to system buffer
            //if packet is just caching at xsocket_t buffer it return enum_code_queue_up;
            //"from_parent_end" is the upper endpoint that fire packet,it might be released since it just be hint, and might be NULL if it is from othes intead of parent node
            //send  search to_xip_addr from top to down and handle it per address match or endpoint type
            //from_xip_addr_high and to_xip_addr_high is 0 for XIP1 Address
            virtual int32_t          send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end) override;
            
        protected: //recv may be called from io-thread when packets is ready to handle
            //recv search target from lower layer(child) to upper layer(parent) by to_xip_addr.
            //usally it been as callback from childnode'host thread,from_child_end indicate where is the packet from, if from_child_end is NULL which means it is from other instead of it'childs
            //Note:from_child_end might be released since it is just hint.
            //recv search to_xip_addr from down to up and handle it per address match and endpoint type
            //from_xip_addr_high and to_xip_addr_high is 0 for XIP1 Address
            virtual int32_t          recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override;
        private:
            xsocket_t*  m_socket_ptr; //must be valid
            uint32_t    m_last_data_seq_id;
        };
    }
};
