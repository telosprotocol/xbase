// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xcontext.h"
#include "xpacket.h"

namespace top
{
    namespace base
    {
        class xlinkpdu_t : public xpdu_t<xlinkhead_t>
        {
            typedef xpdu_t<xlinkhead_t> base;
        protected:
            xlinkpdu_t(xcontext_t & _context,enum_xprotocol_type protocol,int version);
            virtual ~xlinkpdu_t();
        private:
            xlinkpdu_t();
            xlinkpdu_t(const xlinkpdu_t &);
            xlinkpdu_t & operator = (const xlinkpdu_t &);
        protected:
            bool  generate_random_content(std::string & content,uint32_t max_random_bytes = 0);
        };
        
        //xlink_ping_pdu used for p2p ping based UPD or XUDP
        class xlink_ping_pdu : public xlinkpdu_t
        {
        public:
            xlink_ping_pdu(xcontext_t & _context,int version, int _anti_dpi_level = 0);//valid _anti_dpi_level is at range of [0,8]
            virtual ~xlink_ping_pdu();
        private:
            xlink_ping_pdu();
            xlink_ping_pdu(const xlink_ping_pdu &);
            xlink_ping_pdu & operator = (const xlink_ping_pdu &);
        protected:
            virtual int32_t     do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t     do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            std::string   _pdu_random_content;
            uint64_t      _pdu_fire_timestamp;  //local gmt timestamp
            int16_t       _pdu_fire_TTL;        //set max TTL
            uint16_t      _socket_avg_RTT;      //ms,carry RTT from peer where fire keepalive and recv adk then calcuate RTT
            uint16_t      _from_ip_port;        //real socket port of source
            uint16_t      _to_ip_port;          //real socket port of target
            std::string   _from_ip_address;     //IP address of source
            std::string   _to_ip_address;       //IP address of target
            std::string   _pdu_payload;         //customized payload
        };
        
        //xlink_signal_pdu wrap control/signal command
        class xlink_signal_pdu : public xlinkpdu_t
        {
        public:
            xlink_signal_pdu(xcontext_t & _context,int version,int _anti_dpi_level = 0);
            virtual ~xlink_signal_pdu();
        private:
            xlink_signal_pdu();
            xlink_signal_pdu(const xlink_signal_pdu &);
            xlink_signal_pdu & operator = (const xlink_signal_pdu &);
        protected:
            virtual int32_t     do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t     do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            std::string   _pdu_random_content;
            uint64_t      _pdu_fire_timestamp;
            int16_t       _pdu_fire_TTL;        //set max TTL
            uint16_t      _socket_avg_RTT;      //ms,carry RTT from peer where fire keepalive and recv adk then calcuate RTT
            std::string   _pdu_payload;         //customized payload
        };
        
        //xlink_keepalive_pdu wrap keep alive
        class xlink_keepalive_pdu : public xlinkpdu_t
        {
        public:
            xlink_keepalive_pdu(xcontext_t & _context,int version,int _anti_dpi_level = 0);
            virtual ~xlink_keepalive_pdu();
        private:
            xlink_keepalive_pdu();
            xlink_keepalive_pdu(const xlink_keepalive_pdu &);
            xlink_keepalive_pdu & operator = (const xlink_keepalive_pdu &);
        protected:
            virtual int32_t     do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t     do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            std::string   _pdu_random_content;
            uint64_t      _pdu_fire_timestamp;
            int16_t       _pdu_fire_TTL;        //set max TTL
            uint16_t      _socket_avg_RTT;      //ms,carry RTT of packet
            uint8_t       _socket_avg_lossrate; //avg packet' drop rate,0- 100%
            uint8_t       _socket_avg_jitter;   //avg packet' jitter,each unit present as 16ms,and max jitter is 255 * 16ms = 4080ms
            std::string   _pdu_payload;         //customized payload
        };
        
        //xlink_null_pdu just generate random data,peer just drop it when receive
        class xlink_null_pdu : public xlinkpdu_t
        {
        public:
            xlink_null_pdu(xcontext_t & _context,int version);
            virtual ~xlink_null_pdu();
        private:
            xlink_null_pdu();
            xlink_null_pdu(const xlink_null_pdu &);
            xlink_null_pdu & operator = (const xlink_null_pdu &);
        protected:
            virtual int32_t     do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t     do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            std::string   _pdu_random_content;
        };

        class xlink_handshake_pdu : public xlinkpdu_t
        {
        protected:
            xlink_handshake_pdu(xcontext_t & _context,enum_xlink_handshake_version sub_type);
            virtual ~xlink_handshake_pdu();
        private:
            xlink_handshake_pdu();
            xlink_handshake_pdu(const xlink_handshake_pdu &);
            xlink_handshake_pdu & operator = (const xlink_handshake_pdu &);
        public:
            uint32_t    get_xbase_version_code() const {return _xbase_bin_version;}
            uint8_t     get_socket_type() const {return _socket_type;}
            void        set_socket_type(const uint8_t type){_socket_type = type;}//refer enum_socket_type
            
            uint8_t     get_buildin_keyid() const {return _buildin_key_id;}
            void        set_buildin_keyid(const uint8_t keyid){_buildin_key_id = keyid;}//refer enum_socket_type
        public: //override serialize function by support buildin aes key
            virtual int32_t  serialize_from(xpacket_t & _packet) override;
            virtual int32_t  serialize_to(xpacket_t & _packet) override;
        protected:
            virtual int32_t  do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t  do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        private:
            std::string _pdu_random_content;                //random bytes to let pdu length vary
            uint64_t    _pdu_fire_timestamp;                //the timestamp(ms) when this pdu sendout
            uint32_t    _xbase_bin_version;                 //Core SDK 'version code at local
            uint8_t     _socket_type;                       //request from socket type(refer enum_socket_type)
            uint8_t     _buildin_key_id;                    //buildin key id for encyprtion
        public:
            uint16_t    _ack_peer_seq_id;                   //ATPN,Acked Transmission Packet Number
            uint64_t    _ack_peer_fire_timestamp;           //the timestamp(ms) of last packet from peer, used to calculate RTT
            std::string _pdu_reserved_data;                 //reserver string
        };
        
        //init, init_ack should not store any data,
        class xlink_handshake_init_pdu : public xlink_handshake_pdu
        {
        public:
            xlink_handshake_init_pdu(xcontext_t & _context);
            virtual ~xlink_handshake_init_pdu();
        private:
            xlink_handshake_init_pdu();
            xlink_handshake_init_pdu(const xlink_handshake_init_pdu &);
            xlink_handshake_init_pdu & operator = (const xlink_handshake_init_pdu &);
        protected:
            virtual int32_t  do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t  do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            uint64_t        _client_socket_options;            //present socket 'reliable,order etc information
            uint32_t        _client_cookie_hash;               //cookie hash generated from client
            uint32_t        _client_recv_window;               //receive window like TCP
            uint32_t        _server_bin_version;               //what is minimal 'Core' bin version of server
            
            std::string     _client_account_id;                //account address,verify by _client_signature
            std::string     _client_signature;                 //ecc signature(_client_public_key),and may decode out public-key from signature
            std::string     _client_payload;                   //carry on any addtional information
        };
        
        class xlink_handshake_initack_pdu : public xlink_handshake_pdu
        {
        public:
            xlink_handshake_initack_pdu(xcontext_t & _context);
            virtual ~xlink_handshake_initack_pdu();
        private:
            xlink_handshake_initack_pdu();
            xlink_handshake_initack_pdu(const xlink_handshake_initack_pdu &);
            xlink_handshake_initack_pdu & operator = (const xlink_handshake_initack_pdu &);
       
        protected:
            virtual int32_t  do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t  do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            uint32_t        _server_recv_window;                //receive window like TCP
            int32_t         _server_response_error_code;        //error code for xlinkhandshake_syn_pdu pdu
            uint32_t        _server_cookie_hash;                //cookie hash at server side
            
            uint32_t        _client_cookie_hash;                //send back the cookie from init packet
            
            uint8_t         _session_pow_hash_difficulty;       //session pow-hash difficult
            int8_t          _session_seconds_before_expire;     //total 127 seconds to allow
            
            uint16_t        _client_external_public_port;       //client ' external port saw by server
            std::string     _client_external_public_ip;         //client ' external public ip saw by server
            std::string     _server_response_error_description; //detail error reason
        };
        
        class xlinkhandshake_syn_pdu : public xlink_handshake_pdu
        {
        public:
            xlinkhandshake_syn_pdu(xcontext_t & _context);
            virtual ~xlinkhandshake_syn_pdu();
        private:
            xlinkhandshake_syn_pdu();
            xlinkhandshake_syn_pdu(const xlinkhandshake_syn_pdu &);
            xlinkhandshake_syn_pdu & operator = (const xlinkhandshake_syn_pdu &);
        protected:
            virtual int32_t  do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t  do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            //exchange keepalive information
            uint8_t     _client_keepalive_interval_range;   //keepalive interval as seconds
            
            //exchange obufucation information
            uint8_t     _client_obufucation_method;         //obufucation method from client
            uint8_t     _client_obufucation_seed;           //obufucation seed from client
            uint8_t     _client_obufucation_random_range;   //obufucation range from client
            
            //exchange  encode information
            uint8_t     _client_encode_method;              //reserved for future
            uint8_t     _client_encode_version;             //reserved for future
            
            uint8_t     _client_country_code;               //CC code at client if have or know
            uint8_t     _client_public_key_type;            //indicate what type of m_public_key, 0 for standard XECDH(25519)
            uint8_t     _client_public_key[32];             //ECDH public key
            
            uint32_t    _client_random_seed;                //random seed generated from client
            uint32_t    _client_pow_hash;                   //proof of work of hash = hash32(_session_cookie_hash + _random_nounce)
            uint32_t    _client_cookie_hash;                //cookie hash generated from client
            uint32_t    _server_cookie_hash;                //server' cookie hash,it is 0 for TCP
            uint32_t    _server_bin_version;                //ask the minimal version code of Server Core SDK '
            
            std::string _client_account_id;                //account address,verify by _client_signature
            std::string _client_signature;                 //ecc signature(_client_public_key),and may decode out public-key from signature
            std::string _client_payload;                   //carry on any addtional information
            
            std::string _xip_request_payload;               //for performance, linklayer may carry the xip layer' request pdu directly
            //将STATE COOKIE 中的TCB 部分和本端密钥根据RFC2401 的MAC 算法进行计算，得出的MAC 和STATE COOKIE 中携带的MAC 进行比较。如果不同则丢弃这个消息；如 果相同，则取出TCB 部分的时间戳，和当前时间比较，看时间是否已经超过 了COOKIE 的生命期。如果是，同样丢弃。否则根据TCB 中的信息建立一个 和端A 的偶联。端点B 将状态迁入ESTABLISHED，并发出COOKIE ACK 数 据块
        };
        
        class xlinkhandshake_synack_pdu : public xlink_handshake_pdu
        {
        public:
            xlinkhandshake_synack_pdu(xcontext_t & _context);
            virtual ~xlinkhandshake_synack_pdu();
        private:
            xlinkhandshake_synack_pdu();
            xlinkhandshake_synack_pdu(const xlinkhandshake_synack_pdu &);
            xlinkhandshake_synack_pdu & operator = (const xlinkhandshake_synack_pdu &);
        protected:
            virtual int32_t  do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t  do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            //exchange keepalive information
            uint8_t     _server_keepalive_interval_range;   //keepalive interval as seconds
            
            //exchange obufucation information
            uint8_t     _server_obufucation_method;         //obufucation method from server
            uint8_t     _server_obufucation_seed;           //obufucation seed from server
            uint8_t     _server_obufucation_random_range;   //obufucation range from server
            
            //exchange  encode information
            uint8_t     _server_encode_method;              //reserved for future
            uint8_t     _server_encode_version;             //reserved for future
            
            uint8_t     _server_country_code;               //CC code at server if have or know
            uint8_t     _server_public_key_type;            //must match _client_public_key_type,refer enum_public_private_key_type
            uint8_t     _server_public_key[32];             //ECDH public key
            uint32_t    _server_random_seed;                //random nounce generated from server
            uint32_t    _server_cookie_hash;                //secret cookie hash of server
            uint32_t    _client_cookie_hash;                //secret cookie hash of client
            
            int32_t     _server_response_error_code;        //error code for xlinkhandshake_syn_pdu pdu
            std::string _server_response_error_description; //detail error reason

            std::string _xip_response_payload;              //for performance, linklayer may carry the xip layer' response pdu directly
            
            //std::string _server_account_id;                 //account address,verify by _server_signature
            //std::string _server_signature;                  //sign(_server_public_key),and may decode out one public-key of signature
            //std::string _server_payload;                    //carry on any addtional information
        };
        
        class xlinkhandshake_ack_pdu : public xlink_handshake_pdu
        {
        public:
            xlinkhandshake_ack_pdu(xcontext_t & _context);
            virtual ~xlinkhandshake_ack_pdu();
        private:
            xlinkhandshake_ack_pdu();
            xlinkhandshake_ack_pdu(const xlinkhandshake_ack_pdu &);
            xlinkhandshake_ack_pdu & operator = (const xlinkhandshake_ack_pdu &);
        protected:
            virtual int32_t  do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t  do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            uint32_t    _peer_cookie_hash;   //secret cookie hash,it is 0 for TCP
        };
        
        //close peer connection
        class xlinkhandshake_fin_pdu : public xlink_handshake_pdu
        {
        public:
            xlinkhandshake_fin_pdu(xcontext_t & _context);
            virtual ~xlinkhandshake_fin_pdu();
        private:
            xlinkhandshake_fin_pdu();
            xlinkhandshake_fin_pdu(const xlinkhandshake_fin_pdu &);
            xlinkhandshake_fin_pdu & operator = (const xlinkhandshake_fin_pdu &);
        protected:
            virtual int32_t  do_read(xmemh_t & archive,const int32_t pdu_body_size) override;
            virtual int32_t  do_write(xmemh_t & archive,int32_t & packet_processs_flags) override;
        public:
            uint32_t    _peer_cookie_hash;   //secret cookie hash,it is 0 for TCP
        };
    }
}
