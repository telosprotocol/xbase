// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include "xendpoint.h"
#include "xatom.h"
#include "xlock.h"
#include "xbasepdu.h"
#include "xutl.h"

namespace  top
{
    namespace base
    {
        enum enum_socket_type
        {
            enum_socket_type_invalid    = 0,
            //stream(or like) socket family
            enum_socket_type_tcp        = 0x01,    //TCP(must be non-block)
            enum_socket_type_xtcp       = 0x02,    //
 
            //datagram(or like) socket family
            enum_socket_type_udp        = 0x20,    //non-block UDP using shared io-thread
            enum_socket_type_xudp       = 0x21,    //non-reliable xudp
            enum_socket_type_xrudp      = 0x22,    //reliable xudp
            
            //icmp socket
            enum_socket_type_icmp       = 0x3D,   //ICMP
 
            
            enum_socket_type_virtual    = 0xFF,    //virtual and proxy socket
        };
        
        enum enum_socket_status
        {
            enum_socket_status_invalid     = 0, //error/uninited status
            enum_socket_status_inited      = 1, //inited status
            enum_socket_status_connecting  = 2, //tcp/sctp/fullrudp has connecting stage
            enum_socket_status_listening   = 3, //tcp/sctp only
            enum_socket_status_connected   = 4, //when ready to write/read socket
            enum_socket_status_idle        = 5, //socket still valid but keep-alive check fail already
            enum_socket_status_disconnected =6, //socket disconnected
            enum_socket_status_error       = 7, //socket hit error,it must be already disconnected
            enum_socket_status_closed      = 8  //socket is closed
        };
        
        //for TCP/UDP, rcommend using enum_io_mode_atom_lock_write at most case
        //Note: socket read always at single-thread to read
        enum enum_io_write_mode
        {
            //only avaiable for UDP socket or datagram handle that rely on the system kernal spin-lock
            //or application guarantee multiple-thread safe
            enum_io_mode_direct_write           = 0,
            
            //use case: very busy tcp socket under multiple-write->single-read mode
            //best performance when write and read at same thread always,most used by internal implementation that can match this condition
            enum_io_mode_async_write            = 1, //without lock but async post the write to host thread where do realy write
            
            //acquired very light atom lock to write at single thread,and turn to non_block write if fail to acquired lock
            //better real-time than enum_io_mode_async_write,and similar performance as  enum_io_mode_direct_write(a little less than it when has large amount thread to write).
            enum_io_mode_atom_lock_write        = 2, //using atom lock to let only one thread do really write to socket buffer
        };
 
        struct xsocket_property
        {
        public:
            xsocket_property()
            {
                _socket_type = enum_socket_type_invalid;
                _local_logic_port = 0;
                _local_logic_port_token = 0;
                _peer_logic_port = 0;
                _peer_logic_port_token = 0;
                _local_real_port = 0;
                _peer_real_port = 0;
                
                _local_cookie_hash = 0;
                _peer_cookie_hash = 0;
            }
        public:
            enum_socket_type _socket_type;
            uint16_t     _local_logic_port;         //for virtual socket or non-connected socket that has logic port to identify,host order(little endian)
            uint16_t     _local_logic_port_token;   //paired with m_local_logic_port;
            uint16_t     _peer_logic_port;          //for virtual socket or non-connected socket that has logic port to identify ,host order(little endian)
            uint16_t     _peer_logic_port_token;    //paired with m_peer_logic_port;
            
            uint32_t     _local_cookie_hash;        //use for handsahke verification
            uint32_t     _peer_cookie_hash;         //use for handsahke verification
            
            std::string  _local_ip_addr;            //locally bind IPv4 or IPv6 address
            uint16_t     _local_real_port;          //socket 'listen port
            uint16_t     _peer_real_port;           //peer socket 'port
            std::string  _peer_ip_addr;             //peer IPv4 or IPv6 address
            
            std::string  _peer_account_id;          //account address,verify by _peer_signature
            std::string  _peer_signature;           //ecc signature(_client_public_key),and may decode out public-key from signature
            std::string  _peer_payload;             //carry on any addtional information
        public:
            int32_t     _on_close_notify_thread_id; //where to excute _on_close_notify_callback
            xcall_t     _on_close_notify_call;      //notify the socket when close
        };
        
        class xsocket_t : public xendpoint_t,public xiosink_t
        {
        protected:
            enum
            {
                #ifdef __DEBUG_SOCKET_WITHOUT_TIMEOUT_BOTHER__
                enum_idle_time_out     = 150000, //timeout after idle 150 seconds
                #else
                enum_idle_time_out     = 15000,  //timeout after idle 30 seconds
                #endif
                enum_keepalive_timer_interval = 1000, //as default send keepalive packet at every 1000ms from client to server(then server respond to client)
            };
            friend class xudplisten_t;
            friend class tcp_listen_t;
        protected:
            xsocket_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,xfd_handle_t native_handle,enum_socket_type type,enum_io_write_mode mode = enum_io_mode_atom_lock_write);
            virtual ~xsocket_t();
        private:
            xsocket_t();
            xsocket_t(const xsocket_t &);
            xsocket_t & operator = (const xsocket_t &);
            
        public:
            inline enum_socket_type    get_socket_type() const {return m_socket_type;}
            inline enum_io_write_mode  get_io_write_mode() const {return m_io_write_mode;}
            
            virtual xfd_handle_t       get_handle(); //Get OS native handle
            //caller respond to cast (void*) to related  interface ptr
            virtual void*              query_interface(const int32_t type) override;
            
            virtual  int               is_alive(uint64_t timenow_ms) override;//return how many ms left before expire, return <= 0 if expired
            virtual  bool              is_close() override;
        public: //logic & virtual socket address information
            inline  int                get_local_logic_port() const {return m_local_logic_port;}
            inline  int                get_local_logic_port_token() const {return m_local_logic_port_token;}
            inline  int                get_peer_logic_port() const {return m_peer_logic_port;}
            inline  int                get_peer_logic_port_token() const {return m_peer_logic_port_token;}
        public://physical & real socket 'address information
            inline  std::string        get_local_ip_address() const {return m_local_ip_addr;}
            inline  uint16_t           get_local_real_port()  const {return m_local_real_port;}
            inline  std::string        get_peer_ip_address() const {return m_peer_ip_addr;}
            inline  uint16_t           get_peer_real_port()  const {return m_peer_real_port;}
        
            inline  uint32_t           get_socket_MTU() const {return m_socket_MTU;}
            inline  int                get_idle_timeout_ms() const {return m_idle_timeout_ms;}
        public:
            //start/stop receive data,it must be call to receive data because all socket is under non-block & aysnchronize mode
            virtual int32_t          start_read(int32_t cur_thread_id);
            virtual int32_t          stop_read(int32_t cur_thread_id);
            
            //send keepalive packet to peer, note: socket must already be connected, and peer ' on_endponit_keepalive will be called when recev
            virtual int32_t          send_keepalive(const std::string & _payload,int16_t TTL);
            //send keepalive packet to peer,note: socket must already be connected, and peer ' on_endponit_signal will be called when recev
            virtual int32_t          send_signal(const std::string & _payload,int16_t TTL);
 
            //if cur_thread_id 0 xsocket_t do query current thread id again. same for timenow_ms.
            //return errorcode -refer  enum_error_code,return enum_code_successful if the packet write to system buffer
            //if packet is just caching at xsocket_t buffer it return enum_code_queue_up;
            //"from_parent_end" is the upper endpoint that fire packet,it might be released since it just be hint, and might be NULL if it is from othes intead of parent node
            //send  search to_xip_addr from top to down and handle it per address match or endpoint type
            virtual int32_t          send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end) override;
            
        protected://recv may be called from io-thread when packets is ready to handle
            //recv search target from lower layer(child) to upper layer(parent) by to_xip_addr.
            //usally it been as callback from childnode'host thread,from_child_end indicate where is the packet from, if from_child_end is NULL which means it is from other instead of it'childs
            //Note:from_child_end might be released since it is just hint.
            //recv search to_xip_addr from down to up and handle it per address match and endpoint type
            virtual int32_t          recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override;
        
        public: //using socket ' session key to encrypt and decrypt packet at AES CTR mode
            int32_t                 encrypt_packet(const uint8_t* source_data,const int32_t length,const uint32_t random_seed);
            int32_t                 decrypt_packet(const uint8_t* source_data,const int32_t length,const uint32_t random_seed);
        public: //count api
            inline int32_t          get_bad_packets_count() const { return m_bad_packets;} //how many bad packets that can not decode
            int32_t                 reset_bad_packets_count(){int32_t org = m_bad_packets; m_bad_packets = 0; return org;}
            void                    count_bad_packet(const int add = 1) { m_bad_packets += add;}//not atom guarnetee as default
            
            inline int32_t          get_drop_packets_count() const {return m_droped_packets;} //how many packets are dropped as buffer
            int32_t                 reset_drop_packets_count(){int32_t org = m_droped_packets; m_droped_packets = 0; return org;}
            void                    count_drop_packet(const int add = 1){ m_droped_packets += add;}//not atom guarnetee as default

            inline uint32_t         get_max_batch_read_packets() const {return m_max_batch_read_packets;}
            inline void             reset_max_batch_read_packets(const uint32_t max_packets){ m_max_batch_read_packets = max_packets;}
            inline uint32_t         get_max_batch_read_bytes() const {return m_max_batch_read_bytes;}
            inline void             reset_max_batch_read_bytes(const uint32_t max_bytes){ m_max_batch_read_bytes = max_bytes;}
        protected://return true when the event is handled
            //xiohandle_t attached into io-thread of the target thread(host) ,return true when the event is handled
            virtual bool        on_iohandle_attach(xfd_handle_t handle,const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            //xiohandle_t detached from io-thread of the target(host) thread,return true when the event is handled
            virtual bool        on_iohandle_detach(xfd_handle_t handle,const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms) override;//detach means it detach from io-thread but maybe the   fdhandle(socket) is still valid
            //handle is closed and inited by caller if error_code is 0
            virtual bool        on_iohandle_close(const int32_t error_code,xfd_handle_t handle,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            //writeable event;return new fd_events_t if want change listened,and b_handled indicate whether event is handled or not

            //when no-longer need this event set watchEvents to 0 which will remove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_write(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_msd) override;
            
            //subclass should handle on_iohandle_read
            virtual bool        on_object_close() override; //notify the subclass the object is closed
       
            virtual bool        on_endpoint_open(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child) override;
            
            //when associated io-object close happen,post the event to receiver
            //error_code is 0 when it is closed by caller/upper layer
            virtual bool        on_endpoint_close(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child) override;
 
            virtual int32_t     start_write(int32_t cur_thread_id);
            //return total how many bytes(include OOB) to writed out, < 0 means has error that need close
            virtual int32_t     write_packet(xpacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms) = 0;
        protected:
            virtual std::string dump() override; //dump trace information,just for debug purpose
            enum_socket_status  get_socket_status() const {return m_socket_status;}
            void                set_socket_status(enum_socket_status status){ m_socket_status = status;}
 
            xiohandle_t*        get_iohandle() {return m_ptr_io_handle;}
            void                set_peer_logic_port(const uint16_t peer_logic_port){m_peer_logic_port = peer_logic_port;}
            void                set_peer_logic_port_token(const uint16_t peer_logic_port_token){m_peer_logic_port_token = peer_logic_port_token;}
            
            //on_endpoint_up is a wrap function that finnaly call on_endpoint_open
            bool        on_endpoint_up(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child);
            
        protected:
            //send_packet can only be called after lock the write thread completely
            //Note:write_packets must be under the protection of multiple-thread lock(atom lock or var lock)
            //return 0 if successful handle ,otherwise return -1 or other error code
            int32_t             send_internal(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end);
        private:
            int32_t             send_internal(xpacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms);
            //return how many packets is still pending write out,return < 0 means error, return = 0 means no packet left,
            
            //write_packets can only be called after lock the write thread completely
            //Note:write_packets must be under the protection of multiple-thread lock(atom lock or var lock)
            int32_t             write_packets(const int32_t cur_thread_id, const uint64_t timenow_ms);
            
            //return false when need write_event callback again
            bool                handle_onwrite_event(const int32_t cur_thread_id,const uint64_t timenow_ms);

            //recev specific packet
            int32_t             on_keepalive_packet_recv(xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from);
 
            int32_t             on_signal_packet_recv(xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from);
            
            virtual int32_t     on_ping_packet_recv(xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from);
        private:
            //io write out management
            void*                   m_recv_mqueue;      //pending read quque ,internal use only
            void*                   m_send_mqueue;      //pending write queue,internal use only
            xiohandle_t*            m_ptr_io_handle;    //let subclass initiliaze it
            enum_socket_type        m_socket_type;
            enum_io_write_mode      m_io_write_mode;
            enum_socket_status      m_socket_status;    //status for socket connect
            char                    cpu_cacheline1[_CONST_CPU_CACHE_LINE_BYTES_];
        private:
            xpacket_t               m_last_packet;       //the last unsend packet
            char                    cpu_cacheline2[_CONST_CPU_CACHE_LINE_BYTES_];
        private:
            xatomlock_t             m_io_write_lock;    //just use for enum_io_mode_atom_lock_write
            char                    cpu_cacheline3[_CONST_CPU_CACHE_LINE_BYTES_ - sizeof(xatomlock_t)];
        private:
            xatomlock_t             m_fire_write_event_lock;
            char                    cpu_cacheline4[_CONST_CPU_CACHE_LINE_BYTES_ - sizeof(xatomlock_t)];
        protected: //logic/vlink information
            //full address for socket(link layer) ip:real-port->socket_id:logic_port:port_token
            uint16_t                m_local_logic_port;  //for virtual socket or non-connected socket that has logic port to identify,host order(little endian)
            uint16_t                m_local_logic_port_token; //paired with m_local_logic_port;
            uint16_t                m_peer_logic_port;   //for virtual socket or non-connected socket that has logic port to identify ,host order(little endian)
            uint16_t                m_peer_logic_port_token; //paired with m_peer_logic_port;
            
            uint64_t                m_local_socket_id;      //v-link ' peer socket id
            uint64_t                m_peer_socket_id;       //v-link ' peer socket id
        protected://physical & real socket address information
            xfd_handle_t            m_raw_socket_handle;               //cache the raw socket handle
            uint16_t                m_local_real_port;                 //socket 'listen port
            uint16_t                m_peer_real_port;                  //peer socket 'port
            std::string             m_local_ip_addr;                   //locally bind IPv4 or IPv6 address
            std::string             m_peer_ip_addr;                    //peer IPv4 or IPv6 address
            
            int32_t                 m_keepalive_interval;              //as default it is 1000ms
            int32_t                 m_idle_timeout_ms;                 //socket may auto close if dont recv any packet after   m_socket_idle_timeout
            uint32_t                m_socket_MTU;                      //WHAT IS Max packet size to allow through socket,as default it is 65535
            
            uint8_t                 m_ecc_public_key[32];              //XECDH Public key
            uint8_t                 m_ecc_private_key[32];             //XECDH private key
            uint8_t                 m_ecc_shared_secret[32];           //XECDH shared secret as AES Key
            uint8_t                 m_ecc_peer_public_key[32];         //XECDH Public key
            uint32_t                m_ecc_local_random_seed;           //local random seed
            uint32_t                m_ecc_peer_random_seed;            //peer random seed
        protected:
            int32_t                 m_bad_packets;                     //how many packets can not be decode as wrong format
            int32_t                 m_droped_packets;                  //how many packets are droppped as buffer
            uint32_t                m_out_packets;                     //count how many data packets send out
            uint32_t                m_last_data_seq_id;                //the last data seqid sent
            uint32_t                m_last_keepalive_seq_id;           //keepalive need continued seqid to estimate packet drop
            uint32_t                m_in_keepalive_packets;            //how many keepalive packets received from peer
            uint32_t                m_in_packets;                      //count how many data packets in
        protected:
            uint32_t                m_max_batch_read_packets;          //how many max packets can be read at one event
            uint32_t                m_max_batch_read_bytes;            //how many bytes can be read at one event
        };
        
        //the simple wrap for raw udp socket of OS
        class udp_t : public xsocket_t
        {
        public:
            //all udp or udp based socket using same default udp seed to obufucations
            static uint32_t    get_const_udp_seed();
        public:
            //native_handle must valid
            //transfer the owner of native_handle to udp_t who may close handle when destroy
            udp_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,xfd_handle_t native_handle);
        protected:
            virtual ~udp_t();
        private:
            udp_t();
            udp_t(const udp_t &);
            udp_t & operator = (const udp_t &);
        public:
            //virtual std::string dump() override; //dump trace information
        private:
            //readable event; return new fd_events_t if want change listened events,b_handled indicate whether it is processed
            //when no-longer need this event set watchEvents to 0,which emove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_read(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            //return total how many bytes(include OOB) to writed out, < 0 means has error that need close
        protected:
            virtual int32_t     write_packet(xpacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            bool                read_msg(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms);
            bool                read_mul_msg(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms);
        private:
          #ifdef __CACHE_PEER_IP_FROM_RAW_VALUE__
            struct ipv4_address
            {
            public:
                ipv4_address()
                {
                    ipv4_int = 0;
                }
            public:
                std::string  ipv4_string;
                uint32_t     ipv4_int;
            };
            ipv4_address     m_peer_ipv4_addr[1024];//cache 1024 peer to quickly convert ipv4_int to ipv4_string
        #endif
        };
        
        //the simple wrap for raw tcp socket of OS
        //note: tcp_t not responsible for connecting and listening, it ask one connected tcp socket already
        class tcp_t : public xsocket_t
        {
        public:
            //native_handle must valid and connected already
            //transfer the owner of native_handle to tcp_t who may close handle when destroy
            tcp_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,xfd_handle_t native_handle);
        protected:
            virtual ~tcp_t();
        private:
            tcp_t();
            tcp_t(const tcp_t &);
            tcp_t & operator = (const tcp_t &);
        public:
            //virtual std::string dump() override; //dump trace information
        protected:
            //readable event; return new fd_events_t if want change listened events,b_handled indicate whether it is processed
            //when no-longer need this event set watchEvents to 0,which emove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_read(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
        private:
            //return total how many bytes(include OOB) to writed out, < 0 means has error that need close
            virtual int32_t     write_packet(xpacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
        };
        
        //client side' tcp socket that connecting to server
        class tcp_connect_t : public tcp_t
        {
        public:
            //connect_to_ip and connect_to_port  must valid
            tcp_connect_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,const std::string connect_to_ip,const int connect_to_port,void* connect_cookie);
        protected:
            virtual ~tcp_connect_t();
        private:
            tcp_connect_t();
            tcp_connect_t(const tcp_connect_t &);
            tcp_connect_t & operator = (const tcp_connect_t &);
        private://tcp connect need monitor read & write event both
            //readable event; return new fd_events_t if want change listened events,b_handled indicate whether it is processed
            //when no-longer need this event set watchEvents to 0,which emove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_read(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            
            //when no-longer need this event set watchEvents to 0 which will remove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_write(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_msd) override;
        protected:
            //callback when new tcp socket is connected
            virtual bool  on_tcp_socket_connect(xfd_handle_t newhandle,const std::string peer_ip,const int peer_port,void* connect_cookie,const int32_t cur_thread_id,const uint64_t timenow_ms)
            {
                return true;
            }
        private:
            void*  m_connect_cookie;
        };
        
        //listen tcp at asynchronize mode
        class tcp_listen_t : public tcp_t
        {
        public:
            //listen_ip must valid
            //listen_port may return the bind local port if pass 0
            tcp_listen_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,const std::string listen_ip,uint16_t &  listen_port,const int listen_backlog,void* listen_cookie);
        protected:
            virtual ~tcp_listen_t();
        private:
            tcp_listen_t();
            tcp_listen_t(const tcp_listen_t &);
            tcp_listen_t & operator = (const tcp_listen_t &);
        private://tcp connect need monitor read & write event both
            //readable event; return new fd_events_t if want change listened events,b_handled indicate whether it is processed
            //when no-longer need this event set watchEvents to 0,which emove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_read(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            
            //when no-longer need this event set watchEvents to 0 which will remove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_write(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_msd) override;
        protected:
            //callback when new tcp socket is accepted
            virtual bool  on_tcp_socket_accept(xfd_handle_t newhandle,const std::string local_ip, const int local_port,const std::string peer_ip,const int peer_port,void* listen_cookie,const int32_t cur_thread_id,const uint64_t timenow_ms)
            {
                return true;
            }
        private:
            void* m_listen_cookie;
        };
        
        //xslsocket_t is a enhanced secury socket layer(X-S-S-L) that support handkshaking signal and end to end cryption
        //note: xslsocket is a completely different implementation than stand SSL/TLS, all are customized
        class xslsocket_t : public xsocket_t,public xtimersink_t
        {
            friend class xudplisten_t;
        protected:
            //combining socket status(enum_socket_status_connecting)
            enum enum_handkshake_stage
            {
                enum_handkshake_stage_start     = 0, //just     start and not inited yet
                enum_handkshake_stage_init      = 1, //init     stage, client:sending init
                enum_handkshake_stage_init_ack  = 2, //init_ack stage, server:responsing init_ack
                enum_handkshake_stage_sync      = 3, //sync     stage, client:sending sync
                enum_handkshake_stage_sync_ack  = 4, //sync_ack stage, server:responsing sync_ack
                enum_handkshake_stage_open      = 5, //open     stage, working
            };
            enum enum_const_value_of_xslsocket
            {
                const_max_status_expired_duration = 5000, //5 seconds for each status
                const_max_status_retry_count      = 10,   //10 packets
                const_default_connection_timeout  = 15000,//15 seconds for whole connection
            };

            struct  xhandkshake_status
            {
                uint64_t                      _status_start_time;
                int                           _status_expired_duration;  //ms expired for this status
                int                           _status_retried_count;     //how many retry(by resend packets)
                int                           _status_max_retry_count;   //total how many  may retry
                enum_handkshake_stage         _status_type;              //combining socket status(enum_socket_status_connecting)
            };
        protected:
            xslsocket_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,xfd_handle_t native_handle,enum_socket_type socket_type,enum_io_write_mode io_mode);
            virtual ~xslsocket_t();
        private:
            xslsocket_t();
            xslsocket_t(const xslsocket_t &);
            xslsocket_t & operator = (const xslsocket_t &);
        public:
            //xslsocket allow do authentication check if need, at that case client(connect) side need passin the related information
            void                init_authentication(const std::string _this_account_id,const std::string _this_account_signature,const std::string _this_account_payload)
            {
                m_this_account_id = _this_account_id;
                m_this_account_signature = _this_account_signature;
                m_this_account_payload = _this_account_payload;
            }
            
            virtual  int        is_alive(uint64_t timenow_ms) override;//return how many ms left before expire, return <= 0 if expired
            
            virtual int32_t     send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end) override;
            
            virtual std::string dump() override; //dump trace information,just for debug purpose
        protected:
            int32_t             send_data_pdu(xpacket_t & _data,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end);
 
            int32_t             send_handshake_pdu(enum_handkshake_stage _stage);
            int32_t             send_handshake_pdu(xpdu_t<xlinkhead_t> & pdu);
 
            virtual bool        on_endpoint_open(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child) override;
            
            //when associated io-object close happen,post the event to receiver
            //error_code is 0 when it is closed by caller/upper layer
            virtual bool        on_endpoint_close(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from) override;
            
            virtual bool        on_object_close() override; //notify the subclass the object is closed;
            
        private://received specific pdu ,handled at ineternally
            int32_t             on_fragment_data_packet_recv(_xlink_header & _linkhead,uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from);
            
            virtual int32_t     on_data_packet_recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from);
            
            virtual int32_t     on_handshake_packet_recv(xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from);
   
        private:
            virtual bool        on_timer_start(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override;  //attached into io-thread
            
            virtual bool        on_timer_stop(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override;   //detach means it detach from io-thread
            
            virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) override;
        protected:
            xhandkshake_status      m_handshake_status;
            uint32_t                m_local_cookie_hash;
            uint32_t                m_peer_cookie_hash;
            int32_t                 m_connection_timeout_ms; //ms expired for full connection,as default 15s
            int64_t                 m_connecting_start_time; //when start to connect,we do timeout for them
            xtimer_t*               m_socket_timer;
            
            std::string             m_this_account_id;  //it' account_address or account_id,generated from peer'public key
            std::string             m_this_account_signature;   //it' ecc signature(_public_key) for account_id,decode public key from signature
            std::string             m_this_account_payload; //customized payload data(send to peer for handshake or other)
        private:
            std::map<int32_t,std::map<int32_t,xpacket_t> > m_fragment_packets; //key as sequence_id ,value is related fragments packet(pending with timeout)
            void*                   m_pending_mqueue;      //pending write queue to cache packet before connected,internal use only
        };
        
        //xtcp_t has own handkshake protocol(anti-DDOS) that is handled under xudplisten_t
        //xtcp_t support end-to-end encryption, obufucation, fragment and defragment, and authentication
        class xtcp_t : public xslsocket_t
        {
        public:
            //native_handle must valid and connected already
            //transfer the owner of native_handle to xtcp_t who may close handle when destroy
            xtcp_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,xfd_handle_t native_handle,xsocket_property & property);
        protected:
            virtual ~xtcp_t();
        private:
            xtcp_t();
            xtcp_t(const xtcp_t &);
            xtcp_t & operator = (const xtcp_t &);
        public: //client use only
            virtual int32_t     connect(const int connect_timeout_ms = const_default_connection_timeout,const int keepalive_timeout_ms = enum_idle_time_out); //how many ms to wait before connect,as default it s 15 seconds
        private:
            //readable event; return new fd_events_t if want change listened events,b_handled indicate whether it is processed
            //when no-longer need this event set watchEvents to 0,which emove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_read(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
        
            //return total how many bytes(include OOB) to writed out, < 0 means has error that need close
            virtual int32_t     write_packet(xpacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
        };
        
        //xudp is a virtual socket that send packet through by underly udp_t socket
        //xudp has own handkshake protocol(anti-DDOS) that is handled under xudplisten_t
        //xudp support end-to-end encryption, obufucation, fragment and defragment, and authentication
        class xudp_t : public xslsocket_t
        {
        public:
            xudp_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,int64_t virtual_handle,xsocket_property & property);
        protected:
            virtual ~xudp_t();
        private:
            xudp_t();
            xudp_t(const xudp_t &);
            xudp_t & operator = (const xudp_t &);
        public://client use only
            //peer account_address or account_id,generated from peer'public key
            //peer ecc signature(_public_key) for account_id,decode public key from signature
            //note: xudp_t may stop keep-alive timer if keepalive_timer_interval is 0,which means application is responsible to keep-alive for both socket of client and server side
            virtual int32_t     connect(const std::string target_ip,const uint16_t target_port,const int connect_timeout_ms = const_default_connection_timeout,const int keepalive_timeout_ms = enum_idle_time_out,const int keepalive_timer_interval = enum_keepalive_timer_interval);
            virtual int32_t     connect(const std::string target_ip,const uint16_t target_port,const uint16_t target_logic_port,const uint16_t target_logic_port_token,const int connect_timeout_ms = const_default_connection_timeout,const int keepalive_timeout_ms = enum_idle_time_out,const int keepalive_timer_interval = enum_keepalive_timer_interval);
            
            //provide ping function for outside,and peer may trigger on_ping_packet_recv when recv this ping packet
            //note:send_ping usally do p2p tunnel before reall connect
            virtual int        send_ping(std::string target_ip_addr,uint16_t target_ip_port,const std::string & _payload,uint16_t TTL,uint16_t avg_RTT = 0,uint16_t target_logic_port = 0,uint16_t target_logic_port_token = 0,uint16_t from_logic_port = 0,uint16_t from_logic_port_token = 0);
        protected:
            //return total how many bytes(include OOB) to writed out, < 0 means has error that need close
            virtual int32_t     write_packet(xpacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
            
            //subclass should handle on_iohandle_read
            virtual bool        on_object_close() override; //notify the subclass the object is closed
        private:
            //overide start_read and start_write,just pass them to m_raw_socket_ptr
            virtual int32_t     start_read(int32_t cur_thread_id) override;
            virtual int32_t     start_write(int32_t cur_thread_id) override;
            virtual bool        on_iohandle_read(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
        protected:
            sockaddr_storage    m_peer_sys_sockaddr;
        private:
            int32_t             m_on_close_notify_thread_id; //where to excute _on_close_notify_callback
            xcall_t             m_on_close_notify_call;      //notify the socket when close
        };
        
        typedef std::function<xslsocket_t*(xfd_handle_t handle,xsocket_property & property, int32_t thread_id,uint64_t timenow_ms,void* cookie) > new_xslsocket_function_t;//for virtual socket by sharing same iohandle
        //xudplisten_t listen one bind udp socket and accept virtual socket(based on UDP)
        class xudplisten_t : public udp_t
        {
            class handshake_session_t
            {
            public:
                handshake_session_t();
                handshake_session_t(const handshake_session_t & obj);
                handshake_session_t & operator = (const handshake_session_t & obj);
                ~handshake_session_t();
            public:
                bool  is_block(const uint64_t timenow_ms); //determine whether need block this peer
            public:
                uint64_t     session_expire_time;               //cookie' expire time
                uint32_t     session_local_random_seed;         //local 'random number
                uint32_t     session_peer_random_seed;          //peer ' random number
                
                uint16_t     session_local_sequence_id;         //local 'seq id
                uint16_t     session_peer_sequence_id;          //peer 'seq id
                
                uint16_t     session_local_logic_port;          //local local logci port
                uint16_t     session_local_logic_port_token;    //local_logic_port_token
                uint16_t     session_peer_logic_port;           //peer' local logci port
                uint16_t     session_peer_logic_port_token;     //peer' local logci port
                
                uint16_t     session_local_socket_type;         //refer enum_socket_type
                uint16_t     session_peer_socket_type;          //refer enum_socket_type
                
                uint32_t     session_local_cookie_hash;         //hash at local data
                uint32_t     session_peer_cookie_hash;          //hash at peer cookie
                
                uint8_t      session_local_buildin_key_id;      //local 'buildin encrypt key id
                uint8_t      session_peer_buildin_key_id;       //peer 'buildin encrypt key id
                uint8_t      session_pow_difficulty;            //ask peer provide POW verificiation
                uint8_t      reserved_uint8;
                uint32_t     session_handshake_count;           //count to decide whether has attacking
            };
            
            //present concept of connection endpoint like SCTP
            class peer_endpoint
            {
            public:
                peer_endpoint();
                peer_endpoint(const peer_endpoint & obj);
                peer_endpoint & operator = (const peer_endpoint & obj);
                ~peer_endpoint();
            public:
                bool  is_alive(const uint64_t timenow_ms);
                bool  is_block(const uint64_t timenow_ms); //determine whether need block this peer
            public:
                //map [logic_port_token:logic_port] and handshake_session_t
                std::map<uint32_t,handshake_session_t> _sessions;
                uint64_t  last_alive_time;          //the last time when receive handkshake event
                uint32_t  attacking_level;          //0 means normal without attacking
            };
            
            enum
            {
                const_max_handshake_session_duration  = 15000,//15 seconds
                const_max_pendding_session_count      = 65536, //64K pending session
                const_min_remove_oldest_session_count = 8,     //force to remove 8 sessions if reach const_max_pendding_session_count
                
                const_client_buckets_count            = 16,     //16 * 256 = 4k reserved for client
                const_max_client_slots_count          = const_client_buckets_count * 256, //total 4096
            };
            //mapping "expired utc time" -> "[peer_sys_ip:peer_sys_port]-[peer_logic_port:peer_logic_port_token]";
            typedef std::map<uint64_t,std::string>            std_map_expiretime_to_peer;
            //map "peer_sys_ip:peer_sys_port" -> "peer_endpoint"
            typedef std::unordered_map<std::string,peer_endpoint> std_map_peer_to_end;
            
        public:
            //native_handle must valid,and transfer the owner of native_handle to xudplisten_t who may close handle when destroy
            xudplisten_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,xfd_handle_t native_handle,new_xslsocket_function_t * _accept_create_callback_ptr = NULL,void* _accept_create_callback_cookie = NULL);
        protected:
            virtual ~xudplisten_t();
        private:
            xudplisten_t();
            xudplisten_t(const xudplisten_t &);
            xudplisten_t & operator = (const xudplisten_t &);
        public:
            bool               is_support_type(enum_socket_type vsocket_type); //determine whether vsocket_type can be handled by xudplisten_t
            
            //create client socket for connect,note:create_xslsocket may pick thread of xudplisten as the host thread of xslsocket_t if socket_attach_to_thread_id is <= 0
            xslsocket_t*       create_xslsocket(enum_socket_type vsocket_type,int32_t socket_attach_to_thread_id = 0); //create xudp_t, xrudp_t etc
            
            //provide ping function for outside,and peer may trigger on_ping_packet_recv when recv this ping packet
            virtual int        send_ping(std::string target_ip_addr,uint16_t target_ip_port,const std::string & _payload,uint16_t TTL,uint16_t avg_RTT = 0,uint16_t target_logic_port = 0,uint16_t target_logic_port_token = 0,uint16_t from_logic_port = 0,uint16_t from_logic_port_token = 0);
            
        protected: //create_vsocket  may trigger the callback of new_vsocket_function_t()
            virtual xslsocket_t*     create_xslsocket(xendpoint_t * parent,xfd_handle_t handle,xsocket_property & property, int32_t attach_to_thread_id,uint64_t timenow_ms)
            {
                if(m_new_vsocket_callback_ptr != NULL)
                    return (*m_new_vsocket_callback_ptr)(handle,property,attach_to_thread_id,timenow_ms,m_new_vsocket_callback_cookie);
                
                //default handle
                if(enum_socket_type_xudp == property._socket_type)
                    return new xudp_t(*get_context(),parent,handle,attach_to_thread_id,property);
                else
                    return NULL;
            }
            
            //finally accept the connection from peer,note:cur_thread_id is the thread of xudplisten_t and subclass may overwrite and give different thread for xslsocket
            virtual xslsocket_t*     on_xslsocket_accept(xfd_handle_t handle,xsocket_property & property, int32_t cur_thread_id,uint64_t timenow_ms)
            {
                return create_xslsocket(NULL,handle,property,cur_thread_id,timenow_ms);
            }
        private:
            virtual int32_t             recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override;
            
            //readable event; return new fd_events_t if want change listened events,b_handled indicate whether it is processed
            //when no-longer need this event set watchEvents to 0,which emove it from loop
            //return true when the event is handled
            virtual bool                on_iohandle_read(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms) override;
        private:
            int                         alloc_client_logic_port(int start_search_offset,const uint64_t timenow_ms);//may executed at diffirent thread
            int                         alloc_peer_logic_port(int start_search_offset,const uint64_t timenow_ms); //just executed at one single thread
            bool                        free_logic_port(const uint32_t logic_port,const int32_t logic_port_token,xslsocket_t * target_socket_ptr);
            int                         check_expired_sessions(const uint64_t timenow_ms);
            int                         remove_sessions_by_expiretime(uint32_t max_allow_count);//force remove at exception
            
            int32_t                     handle_handshake_protocol(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end);
        private:
            //mapping "expired utc time" -> "peer_sys_ip:peer_sys_port:peer_logic_port:peer_logic_port_token";
            std_map_expiretime_to_peer  m_expiretime_to_peer_map;
            //map "peer_sys_ip:peer_sys_port:peer_logic_port:peer_logic_port_token" -> "[16bit:local_port][16bit:local_port_token][32bit:session_hash]"
            std_map_peer_to_end         m_peer_to_end_map;
            
            new_xslsocket_function_t*   m_new_vsocket_callback_ptr;
            void*                       m_new_vsocket_callback_cookie;
            int32_t                     m_client_alloced_port_count;    //count total ports alloced for client
            int32_t                     m_server_alloced_port_count;    //count total ports alloced for server
            int32_t                     m_last_client_alloced_offset;   //tag the last allocted point 
            uint32_t                    m_last_alloced_offset; //high 8bit for bucket,low 8bit for item offset
            
        private:
            xslsocket_t*                m_virtual_sockets[256][256]; //total 65536 sockets,and reserved 4096 for client outbound
            std::recursive_mutex        m_mutex;//used for client
        };

    }//end of namespace of base
}; //end of namespace of top
