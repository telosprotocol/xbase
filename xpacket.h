// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xmem.h"
#include "xaes.h"
#include "xhash.h"
#include "xcompress.h"

namespace top
{
    namespace base
    {
        //process flag to indicate how to wrap local packet before send to peer
        //note:process flag not persisting into the content of packet,just used to manage how to send/receive packet at local
        //total 16 bit
        enum enum_xpacket_process_flag
        {
            enum_xpacket_process_flag_has_basehead     = 0x0001, //packet already tag the XBASE header
            enum_xpacket_process_flag_encrypt          = 0x0002, //defaut,ask do encryption between two peer nodes by using DHF exchange key
            enum_xpacket_process_flag_checksum         = 0x0004, //defaut,ask do CRC32/16 or hash32 verification for every packet
            enum_xpacket_process_flag_compress         = 0x0008, //defaut turn off,recommend compress for packet'content
            enum_xpacket_process_flag_fec_correct      = 0x0010, //FEC(forward error correct) deliver
            enum_xpacket_process_flag_redundancy       = 0x0020, //RED(redundancy）deliver(e.g. pick TCP/UDP send both)
            enum_xpacket_process_flag_obfuscation      = 0x0040, //ask add mix with non-fix header,obfuscation feature
            enum_xpacket_process_flag_write_confirm    = 0x0080, //ask report this packet is completely writed into native handle(Socket)
            enum_xpacket_process_flag_raw_src_address  = 0x0100, //xpacket'from_ip_addr is raw address,to get readable address by calling xsocket_utl::get_ipaddress_from_raw()
            enum_xpacket_process_flag_following_packet = 0x0200, //indicate has next packet is comming & following,as optimization purpose
        };

        class xpacket_t
        {
        protected:
            enum { enum_object_size       = 256};    //64bytes is equal 1 cpu cache line at x86 cpu
            enum { enum_property_size     = 64};     //exclude from/to/proxy/packet_id/timems/cookie + _type,flags...
            enum { enum_local_header_size = 64};     //local header buffer
            enum { enum_local_body_size   = 128};    //local body buffer
        public:
            xpacket_t();//using xcontext_t::instance() to initialize
            xpacket_t(xcontext_t & _context);
            xpacket_t(xcontext_t & _context,xmemh_t & body);
            xpacket_t(xcontext_t & _context,const xpacket_t & obj,bool just_copy_property = false);  //equal as copy_from
            //this contruction allow xpacket_t use outside buffer to hold data,use case to improve performance for memory alloc by resuing outside memory
            //note: front_offset and back_offset present the valid data,e.g. xpacket is empty when (front_offset == back_offset)
            xpacket_t(xcontext_t & _context,const uint8_t* source_buffer, uint32_t buff_capacity,uint32_t front_offset,uint32_t back_offset,bool b_lock_write = false);
            
            xpacket_t & operator = (xmemh_t & body); //
            xpacket_t & operator = (xpacket_t & obj);  //equal as copy_from
            
            ~xpacket_t();
        public: //basic use case
            //XIP<->XLINK<->System Socket<->standard IP layer
            //IP Layer ' ip address and port
            std::string        get_from_ip_addr() const; //guarentee return a readable string of IP Address
            inline std::string get_to_ip_addr() const {return to_ip_addr;}
            inline int         get_from_ip_port() const {return from_ip_port;}
            inline int         get_to_ip_port() const {return to_ip_port;}
            //XLINK 'channel id/port
            inline int         get_from_xlink_port() const {return from_xlink_port;}
            inline int         get_to_xlink_port() const {return to_xlink_port;}
            //XIP/XIP2 Port
            inline uint64_t    get_from_xip_lowaddr() const {return from_xip_lowaddr;}
            inline uint64_t    get_from_xip_highaddr() const {return from_xip_highaddr;}
            inline uint64_t    get_to_xip_lowaddr() const {return  to_xip_lowaddr;}
            inline uint64_t    get_to_xip_highaddr() const {return to_xip_highaddr;}
            inline int         get_from_xip_port() const  {return from_xip_port;}
            inline int         get_to_xip_port() const {return to_xip_port;}
            
            //IP Layer ' ip address and port
            inline void        set_from_ip_addr(const std::string & _from){from_ip_addr = _from;}
            inline void        set_to_ip_addr(const std::string & _to){to_ip_addr= _to;}
            inline void        set_from_ip_port(const uint16_t _from){from_ip_port = _from;}
            inline void        set_to_ip_port(const uint16_t _to){to_ip_port = _to;}
            void               reset_ip_address(); //reset and clean those information if ip address 
            //XLINK 'channel id/port
            inline void        set_from_xlink_port(const uint16_t _from)  {from_xlink_port = _from;}
            inline void        set_to_xlink_port(const uint16_t _to)  {to_xlink_port = _to;}
            //XIP/XIP2 Port
            inline void        set_from_xip_lowaddr(uint64_t addr)  { from_xip_lowaddr = addr;}
            inline void        set_from_xip_highaddr(uint64_t addr) { from_xip_highaddr = addr;}
            inline void        set_to_xip_lowaddr(uint64_t addr)    { to_xip_lowaddr = addr;}
            inline void        set_to_xip_highaddr(uint64_t addr)   { to_xip_highaddr = addr;}
            void               reset_xip_address();
            
            inline uint16_t    get_toxaddr_token() const {return to_xaddr_token;}
            inline void        set_toxaddr_token(const uint16_t token){to_xaddr_token = token;}
            
            inline void        set_from_xip_port(const uint16_t _from)  {from_xip_port = _from;}
            inline void        set_to_xip_port(const uint16_t _to)  {to_xip_port = _to;}
            
            inline int64_t     get_cookie() const {return cookie;}
            inline void        set_cookie(const int64_t _cookie) {cookie = _cookie;}
  
            inline  uint8_t    get_TTL() const {return _TTL;}
            inline  uint8_t    get_MTU() const {return _MTU;}
            inline  void       set_TTL(const uint8_t ttl){_TTL = ttl;}
            inline  void       set_MTUL(const uint8_t mtu){_MTU = mtu;}
            
            #ifdef _DEBUG_PACKET_ID_TIME_
            inline uint64_t    get_id() const {return packet_id;}
            inline void        set_id(const uint64_t _packet_id) {packet_id = _packet_id;}
            inline uint32_t    get_time() const {return timems;}  //note: timems is uint32
            inline void        set_time(const uint32_t _timems) {timems = _timems;}
            inline int32_t     get_duration(const uint32_t timenow_ms) { return (timenow_ms > timems) ? (timenow_ms - timems) : 0; }
            #else
            inline uint64_t    get_id() const {return 0;}
            inline void        set_id(const uint64_t _packet_id){};
            inline uint32_t    get_time() const {return 0;}
            inline void        set_time(const uint32_t _timems) {};
            inline int32_t     get_duration(const uint32_t timenow_ms) { return 0; }
            #endif
            
            inline int32_t              get_packet_flags() const {return _packet_flags;}
            inline int32_t              get_process_flags() const {return _process_flags;}
            
            //[enum_packet_reliable_type(2bit),enum_packet_priority_type(2bit),enum_packet_order_type[1bit],enum_packet_deliver_flag(3bit)]
            //those flags may insert to packet'header when pass the Juclientconnect layer
            inline void                 set_packet_flag(enum_xpacket_flag flag){_packet_flags |= flag;}
            //remove this flag
            inline void                 remove_packet_flag(enum_xpacket_flag flag){_packet_flags &= (~flag);}
            inline void                 reset_packet_flags(const int32_t flags){_packet_flags = flags;}//completely overwrite the flags
            
            inline void                 set_process_flag(enum_xpacket_process_flag flag){_process_flags |= flag;}
            inline void                 remove_process_flag(enum_xpacket_process_flag flag){ _process_flags &= (~flag);}
            inline void                 reset_process_flags(const int32_t flags){_process_flags = flags;}
        public:
            /*
             header:
                [--]
                front:(recent)
                back :(old)
                [--]
             -----------
             body:
                [--]
                front:(old)
                back :(recent)
                [--]
             */
            //using header if header has buffer to hold the size,note: here automically manage _header and _body
            //using get_header() & get_body() if want control it competely
            int32_t             push_front(uint8_t* pPtr, const uint32_t nSize);
            int32_t             push_back(uint8_t* pPtr, const uint32_t nSize);
            
            //pop the data at front direction(first check header ,then body),return the actual size popped
            int32_t             pop_front(const uint32_t pop_size); //pop front header than body
            //pop the data at back direction(first check body ,then header),return the actual size popped
            int32_t             pop_back(const uint32_t nSize);     //pop the data at back end
            
            //advanced use case,operate header and body directly
            xstream_t&          get_header(){ return _header;} //header is optional and reserved for protocol header append at begin of body
            xmemh_t  &          get_body()  { return _body;}      //packet content
            int32_t             get_size()  {return (_header.size() + _body.size());}   //packet 'header + body'
  
        public: //advance use case
            bool   close();  //release the memory if have
            
            bool   copy_from(const xpacket_t & packet,bool just_copy_property = false);//copy whole data and property or just property
            bool   move_from(xpacket_t & packet);
            
        public: //internal use only
            bool   init(); //init as empty packet,and release alloced memory
            bool   reset();//init packet but keep original memory handle untouch,so packet can be use again without memory reallocation
            inline std::string  get_from_ip_rawaddr(){return from_ip_addr;} //from_ip_addr might be a raw value of IP or readable string depends by enum_xpacket_process_flag_raw_src_address flag
        private:
            void   init_construct(); //init for construct function
        private://Note: from/to/packet_id/timems just use to identify and route local, those value not send to network
            //XIP->XLINK->System Socket->standard IP layer
            
            uint64_t      from_xip_lowaddr; //carry source xip address(low64bit) when cache packet
            uint64_t      from_xip_highaddr;//carry source xip address(high64bit)
            uint64_t      to_xip_lowaddr;   //carry target xip address(low64bit)
            uint64_t      to_xip_highaddr;  //carry target xip address(high64bit)
            //XIP/XIP2 Port
            uint16_t      from_xip_port;    //port at XIP layer,0 means invalid/unused
            uint16_t      to_xip_port;      //port at XIP layer,0 means invalid/unused
            uint16_t      to_xaddr_token;   //access token to allow send packet to
            
            //XLink Port
            uint16_t      from_xlink_port;  //logic port/id of at xlink layer,used to demulplex link-channels within UDP/TCP,0 means invalid/unused
            uint16_t      to_xlink_port;    //logic port/id of at xlink layer,used to demulplex link-channels within UDP/TCP,0 means invalid/unused
            
            //default value is enum_xpacket_reliable_type_must | enum_xpacket_order_type_must | enum_xpacket_priority_type_flash
            uint16_t      _packet_flags;    //refer enum_xpacket_flag
            
            //IP Layer 'IP address and Port
            uint16_t      from_ip_port;     //system'socket port(ip layer), 0 means invalid/unused
            uint16_t      to_ip_port;       //system'socket port(ip layer), 0 means invalid/unuse
            
            uint8_t       _MTU;             //max transfer unit size by multiple 256bytes,max value = (255) * 256 = 64KB,0 = no fragment
            uint8_t       _TTL;             //total how many hop allowed,using default size if 0
            uint16_t      _process_flags;   //process flags,refer enum_xpacket_process_flag
            int64_t        cookie;          //application set specific data when get callback of packet is sentout
            
            std::string   from_ip_addr;     //source address(non-readable address or raw address) by depending flag
            std::string   to_ip_addr;       //target address(e.g. IPv4: 200.168.1.1, IPv6:2001:0D12::0987:FE29:9871 )

            #if defined(_DEBUG_PACKET_ID_TIME_)
            uint64_t      packet_id;    //unique packet id under one process
            uint32_t      timems;       //when the packet is generated,0 means not tracking
            #endif
            
            xautomemh_t<enum_local_body_size>     _body;
            xautostream_t<enum_local_header_size> _header; //buffer
        };
 
        //xbasehead_t wrap the head of base protocol
        class xbasehead_t
        {
        public:
            static  void   add_head_flag(_xbase_header * _header,enum_xpacket_flag flag);
            static  void   remove_head_flag(_xbase_header * _header,enum_xpacket_flag flag);
        protected:
            xbasehead_t(_xbase_header * _raw_base_header); //_raw_base_header must be valid
            virtual ~xbasehead_t();
        private:
            xbasehead_t();
            xbasehead_t(const xbasehead_t & obj);
            xbasehead_t & operator = (const  xbasehead_t & obj);
        public:
            inline int      get_protocol() const{ return (m_base_header->ver_protocol & 0xFFF);}
            inline int      get_protocol_version() const{ return (m_base_header->ver_protocol >> 12);}
            inline int      get_extlength() const {return m_base_header->extlength;}
            inline int      get_head_len() const {return m_base_header->header_len;}
            inline int      get_packet_len() const {return m_base_header->packet_len;}
            inline int      get_flags() const {return m_base_header->flags;}
            
            inline void     set_packet_length(const uint16_t packet_len){ m_base_header->packet_len = packet_len;}
            inline void     set_extlength(const uint8_t extend){m_base_header->extlength = extend;}
            void            set_protocol(enum_xprotocol_type protocol);
            void            set_protocol_version(int version); //valid version is range[0,15];
            
            
            void            add_flag(enum_xpacket_flag flag);
            void            remove_flag(enum_xpacket_flag flag);
            inline void     reset_flags(const uint16_t full_flags){m_base_header->flags = full_flags;}
        public:
            virtual int32_t serialize_from(xmemh_t & packet,uint32_t & checksum,uint8_t & compressrate);
            virtual int32_t serialize_to(xmemh_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate);
            
            virtual int32_t serialize_from(xstream_t & packet,uint32_t & checksum,uint8_t & compressrate);
            virtual int32_t serialize_to(xstream_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate);
            
            virtual uint8_t get_serialize_size() { return sizeof(_xbase_header);}
        private:
            _xbase_header*  m_base_header;
        };
        
        class xlinkhead_t : public xbasehead_t
        {
        public:
            xlinkhead_t(enum_xprotocol_type protocol,int version);
            virtual ~xlinkhead_t();
        private:
            xlinkhead_t();
            xlinkhead_t(const xlinkhead_t & obj);
            xlinkhead_t & operator = (const  xlinkhead_t & obj);
        public:
            inline int      get_seq_id() {return m_header.sequnceid;}
            inline void     set_seq_id(const int seq_id){m_header.sequnceid = seq_id;}
            inline int      get_checksum() {return m_header.checksum;}
            inline void     set_checksum(const int cheksum){ m_header.checksum = cheksum;}
            
            inline int      get_to_logic_port() const {return m_header.to_logic_port;}
            inline int      get_to_logic_port_token() const {return m_header.to_logic_port_token;}
            inline void     set_to_logic_port(const uint16_t port) { m_header.to_logic_port = port;}
            inline void     set_to_logic_port_token(const uint16_t port_token) { m_header.to_logic_port_token = port_token;}
            
            inline int      get_from_logic_port() const {return m_header.from_logic_port;}
            inline int      get_from_logic_port_token() const {return m_header.from_logic_port_token;}
            inline void     set_from_logic_port(const uint16_t port) { m_header.from_logic_port = port;}
            inline void     set_from_logic_port_token(const uint16_t port_token) { m_header.from_logic_port_token = port_token;}
            
            inline int      get_fragment_id() const {return m_header.fragment_id;}
            inline int      get_fragment_count() const {return m_header.fragments_count;}
            inline void     set_fragment_id(const uint8_t _id){m_header.fragment_id = _id;}
            inline void     set_fragment_count(const uint8_t _count){m_header.fragments_count = _count;}
            
            inline int      get_compresss_rate() const {return m_header.compressrate;}
        public:
            virtual int32_t serialize_from(xmemh_t & packet,uint32_t & checksum,uint8_t & compressrate) override;
            virtual int32_t serialize_to(xmemh_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate) override;
            
            virtual int32_t serialize_from(xstream_t & packet,uint32_t & checksum,uint8_t & compressrate) override;
            virtual int32_t serialize_to(xstream_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate) override;
            
            virtual uint8_t get_serialize_size() override { return sizeof(_xlink_header);}
            _xlink_header*  get_raw_header() {return &m_header;}
        protected:
            _xlink_header   m_header;
        };
 
        class xiphead_t : public xbasehead_t
        {
        public:
            xiphead_t(enum_xprotocol_type protocol,int version);
            virtual ~xiphead_t();
        private:
            xiphead_t();
            xiphead_t(const xiphead_t & obj);
            xiphead_t & operator = (const  xiphead_t & obj);
        public:
            inline  int         get_ttl()            const {return m_header.TTL;}
            inline  int         get_to_xport()       const {return m_header.to_xport;}
            inline  int         get_from_xport()     const {return m_header.from_xport;}
            inline  uint32_t    get_checksum()       const {return m_header.checksum;}
            inline  uint32_t    get_session_id()     const {return m_header.sesssion_id;}
            inline  int         get_sequence_id()    const {return m_header.sequence_id;}
            inline  int         get_to_xaddr_token() const {return m_header.to_xaddr_token;}
            inline  uint64_t    get_to_xaddr()       const {return m_header.to_xaddr;}
            inline  uint64_t    get_from_xaddr()     const {return m_header.from_xaddr;}
            inline int          get_compresss_rate() const {return m_header.compressrate;}
            
            inline  void        set_ttl(const uint8_t _ttl)                         { m_header.TTL = _ttl;}
            inline  void        set_to_xport(const uint8_t _to_xport)               { m_header.to_xport = _to_xport;}
            inline  void        set_from_xport(const uint8_t _from_xport)           { m_header.from_xport = _from_xport;}
            inline  void        set_checksum(const uint32_t _checksum)              { m_header.checksum = _checksum;}
            inline  void        set_session_id(const uint32_t _session_id)          { m_header.sesssion_id = _session_id;}
            inline  void        set_sequence_id(const uint16_t _sequence_id)        { m_header.sequence_id = _sequence_id;}
            inline  void        set_to_xaddr_token(const uint16_t _to_xaddr_token)  { m_header.to_xaddr_token = _to_xaddr_token;}
            inline  void        set_to_xaddr(const uint64_t _addr)                  { m_header.to_xaddr = _addr;}
            inline  void        set_from_xaddr(const uint64_t _addr)                { m_header.from_xaddr = _addr;}
        public:
            virtual int32_t     serialize_from(xmemh_t & packet,uint32_t & checksum,uint8_t & compressrate) override;
            virtual int32_t     serialize_to(xmemh_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate) override;
            
            virtual int32_t     serialize_from(xstream_t & packet,uint32_t & checksum,uint8_t & compressrate) override;
            virtual int32_t     serialize_to(xstream_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate) override;
            
            virtual uint8_t     get_serialize_size() override { return sizeof(_xip_header);}
            _xip_header*        get_raw_header() {return &m_header;}
        protected:
            _xip_header    m_header;
        };
        
        class xip2head_t : public xbasehead_t
        {
        public:
            xip2head_t(enum_xprotocol_type protocol,int version);
            virtual ~xip2head_t();
        private:
            xip2head_t();
            xip2head_t(const xip2head_t & obj);
            xip2head_t & operator = (const  xip2head_t & obj);
        public:
            inline  int         get_ttl()            const {return m_header.TTL;}
            inline  int         get_to_xport()       const {return m_header.to_xport;}
            inline  int         get_from_xport()     const {return m_header.from_xport;}
            inline  uint32_t    get_checksum()       const {return m_header.checksum;}
            inline  uint32_t    get_session_id()     const {return m_header.sesssion_id;}
            inline  int         get_sequence_id()    const {return m_header.sequence_id;}
            inline  int         get_to_xaddr_token() const {return m_header.to_xaddr_token;}
        
            inline  uint64_t    get_to_xaddr_low()   const {return m_header.to_xaddr_low;}
            inline  uint64_t    get_to_xaddr_high()  const {return m_header.to_xaddr_high;}
            inline  uint64_t    get_from_xaddr_low() const {return m_header.from_xaddr_low;}
            inline  uint64_t    get_from_xaddr_high()const {return m_header.from_xaddr_high;}
            inline int          get_compresss_rate() const {return m_header.compressrate;}
            
            inline  void        set_ttl(const uint8_t _ttl)                         { m_header.TTL = _ttl;}
            inline  void        set_to_xport(const uint8_t _to_xport)               { m_header.to_xport = _to_xport;}
            inline  void        set_from_xport(const uint8_t _from_xport)           { m_header.from_xport = _from_xport;}
            inline  void        set_checksum(const uint32_t _checksum)              { m_header.checksum = _checksum;}
            inline  void        set_session_id(const uint32_t _session_id)          { m_header.sesssion_id = _session_id;}
            inline  void        set_sequence_id(const uint16_t _sequence_id)        { m_header.sequence_id = _sequence_id;}
            inline  void        set_to_xaddr_token(const uint16_t _to_xaddr_token)  { m_header.to_xaddr_token = _to_xaddr_token;}
            
            inline  void        set_to_xaddr_low(const uint64_t _addr)              { m_header.to_xaddr_low = _addr;}
            inline  void        set_to_xaddr_high(const uint64_t _addr)             { m_header.to_xaddr_high = _addr;}
            inline  void        set_from_xaddr_low(const uint64_t _addr)            { m_header.from_xaddr_low = _addr;}
            inline  void        set_from_xaddr_high(const uint64_t _addr)           { m_header.from_xaddr_high = _addr;}
        public:
            virtual int32_t     serialize_from(xmemh_t & packet,uint32_t & checksum,uint8_t & compressrate) override;
            virtual int32_t     serialize_to(xmemh_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate) override;
            
            virtual int32_t     serialize_from(xstream_t & packet,uint32_t & checksum,uint8_t & compressrate) override;
            virtual int32_t     serialize_to(xstream_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate) override;
            
            virtual uint8_t     get_serialize_size() override { return sizeof(_xip2_header);}
            _xip2_header*       get_raw_header() {return &m_header;}
        protected:
            _xip2_header    m_header;
        };
        
        //application layer pdu'header
        class xapphead_t : public xbasehead_t
        {
        public:
            xapphead_t(enum_xprotocol_type protocol,int version);
            virtual ~xapphead_t();
        private:
            xapphead_t();
            xapphead_t(const xapphead_t & obj);
            xapphead_t & operator = (const  xapphead_t & obj);
        public:
            inline int          get_compresss_rate() const {return m_header.compressrate;}
        public:
            virtual int32_t     serialize_from(xmemh_t & packet,uint32_t & checksum,uint8_t & compressrate) override;
            virtual int32_t     serialize_to(xmemh_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate) override;
            
            virtual int32_t     serialize_from(xstream_t & packet,uint32_t & checksum,uint8_t & compressrate) override;
            virtual int32_t     serialize_to(xstream_t & packet,const uint32_t body_size,const uint32_t checksum,const uint8_t compressrate) override;
            
            virtual uint8_t     get_serialize_size() override { return sizeof(xapp_header);}
            xapp_header*        get_raw_header() {return &m_header;}
        protected:
            xapp_header         m_header;
        };
        
        //xpdu_t is general implementation for pdu,can combine different header
        //how to use serialize_from: 1.)create subclass of xpdu_t<with specified header> 2.) overwrite do_read and do_write
        //note: enum_xpacket_process_flag_obfuscation will be handled by xsocket intead of xpdu_t
        template<typename __T_PDU_HEADER__>
        class xpdu_t
        {
        public:
            xpdu_t(xcontext_t & _context,enum_xprotocol_type protocol,int version)
                :m_pdu_header(protocol,version),
                 m_context(_context)
            {
                memset(m_aes_key,0,16);
                memset(m_aes_iv,0,16);
            }
            virtual ~xpdu_t()
            {
            }
        private:
            xpdu_t();
            xpdu_t(const xpdu_t & obj);
            xpdu_t & operator = (const  xpdu_t & obj);
        public:
            __T_PDU_HEADER__ & get_header() {return m_pdu_header;}
            //common header property
            inline int      get_protocol() const{ return m_pdu_header.get_protocol();}
            inline int      get_protocol_version() const{ return m_pdu_header.get_protocol_version();}
            inline int      get_extlength() const {return m_pdu_header.get_extlength();}
            inline int      get_head_len() const {return m_pdu_header.get_head_len();}
            inline int      get_packet_len() const {return m_pdu_header.get_packet_len();}
            inline int      get_flags() const {return m_pdu_header->get_flags();}
            
            void    set_aes_128bit_key(uint8_t aes_128bit_key[16],uint16_t aes_iv[16])
            {
                memcpy(m_aes_key,aes_128bit_key,16);
                memcpy(m_aes_iv,aes_iv,16);
            }
            
            //paired with do_read(xmem_h)
            virtual int32_t serialize_from(xpacket_t & _packet) //packet must be a full one that already finish unfragment
            {
                try
                {
                    xmemh_t & pdu = _packet.get_body();
                    if(pdu.size() < sizeof(_xbase_header))
                    {
                        xwarn_err("xpdu_t::serialize_from,invalid packet(size:%d)",pdu.size());
                        return enum_xerror_code_bad_packet; //indicate error
                    }
                    
                    _xbase_header * base_header = (_xbase_header*)pdu.data();
                    //const int _protocol =  get_xpacket_protocol(base_header);
                    const int total_head_len = base_header->header_len;
                    int       total_packet_len = base_header->packet_len;
                    if(get_xpacket_exflags_value(base_header->flags) != enum_xpacket_extlength_as_flags) //has extend length
                    {
                        total_packet_len |= (((uint32_t)base_header->extlength) << 16);
                    }
                    const int total_body_len = total_packet_len - total_head_len;
                    if( (pdu.size() < total_packet_len) || (total_body_len < 0) )
                    {
                        xwarn_err("xpdu_t::serialize_from,invalid packet(size:%d) < required size(%d),and total_body_len=%d",pdu.size(),total_packet_len,total_body_len);
                        return enum_xerror_code_bad_packet; //indicate error
                    }
                    
                    uint8_t  compress_rate = 1;
                    uint32_t checksum = 0;
                    const int32_t header_readed_size = m_pdu_header.serialize_from(pdu,checksum,compress_rate);
                    if(header_readed_size != total_head_len)
                    {
                        if(header_readed_size < total_head_len)//recoveable error,a newer version of head coming to old client
                        {
                            pdu.pop_front(total_head_len - header_readed_size);
                        }
                        else
                        {
                            xwarn_err("xpdu_t::serialize_from,invalid header(size:%d) > header.pduLen(%d)",header_readed_size,total_head_len);
                            pdu.pop_front(NULL,total_packet_len - header_readed_size); //pop out remaining data
                            return enum_xerror_code_bad_packet;
                        }
                    }
                    if(total_body_len > 0)
                    {
                        //deencrypt if enable
                        if( (m_pdu_header.get_flags() & enum_xpacket_encrypted_flag) != 0) //packet has been encrypted by aes
                        {
                            uint32_t  aes_nounce[8]; //32byte
                            for(int i = 0; i < 4; ++i)//16bytes IV -> 4 * 4byte(uint32_t)
                            {
                                aes_nounce[i] = ((uint32_t*)m_aes_iv)[i] + checksum;//mix the checksum
                            }
                            xaes_t::aes_ctr_decrypt_128bit((const uint8_t*)pdu.data(),total_body_len,m_aes_key,(uint8_t*)aes_nounce); //CTR not required alignment,so CTR not change data size
                            m_pdu_header.remove_flag(enum_xpacket_encrypted_flag);//remove flag after deencrypt
                        }
                        //decompress if enable
                        if( (m_pdu_header.get_flags() & enum_xpacket_compressed_flag) != 0)
                        {
                            xautomemh_t<4096> body_block(m_context,0); //4K is big enough for most packet
                            //reserve enough memory to hold body
                            int32_t reserve_size = std::max((int)(compress_rate * total_packet_len),total_packet_len) + 8;
                            if(reserve_size > enum_max_xpacket_len)//should not over it
                                reserve_size = enum_max_xpacket_len;
                            
                            reserve_size += 256;
                            body_block.reserve_back(reserve_size);//xmem_h::reserved(total_capacity),it is differ than xstream_t::reserver(addtional_size)
                            
                            const int decompressed_size = xcompress_t::lz4_decompress((const char*)pdu.data(),(char*)body_block.back(), total_body_len, body_block.spare_size());
                            if(decompressed_size > 0)
                            {
                                body_block.push_back(NULL,decompressed_size); //refill the decompress data
                                m_pdu_header.remove_flag(enum_xpacket_compressed_flag);
                                pdu.pop_front(NULL,total_body_len); //pop out pdu after successful decompress
                            }
                            else
                            {
                                xwarn_err("xpdu_t::serialize_from,decompress fail,decompressed_size=%d vs org_total_body_len=%d vs body_block(total:%d,front:%d,back:%d),compress_rate=%d,total_packet_len(%d)",decompressed_size,total_body_len,body_block.capacity(),body_block.front_offset(),body_block.back_offset(),(int)compress_rate,total_packet_len);
                                
                                pdu.pop_front(NULL,total_body_len); //pop out the remaining data
                                return enum_xerror_code_bad_packet;
                            }
                            
                            //then do checksum verifiy for raw data now
                            if(checksum != 0)
                            {
                                const uint32_t org_checksum = (uint32_t)xhash64_t::digest(body_block.data(), body_block.size());
                                if(checksum != org_checksum) //if checksum fail, return enum_xerror_code_bad_packet
                                {
                                    xwarn_err("xpdu_t::serialize_from,checksum verify fail,checksum=%d vs org_checksum=%d;total_body_len(%d) vs raw_body_len(%d),compress_rate=%d,total_packet_len(%d)",checksum,org_checksum,total_body_len,decompressed_size,(int)compress_rate,total_packet_len);
                                    return enum_xerror_code_bad_packet;
                                }
                            }
                            
                            const int readed_size = do_read(body_block,body_block.size());
                            if( (readed_size <= 0) || (readed_size > decompressed_size) )
                            {
                                xwarn_err("xpdu_t::serialize_from,do_read return error result(%d) vs total_body_len(%d)",readed_size,total_body_len);
                                return enum_xerror_code_bad_packet;
                            }
                        }
                        else
                        {
                            if(checksum != 0)
                            {
                                const uint32_t org_checksum = (uint32_t)xhash64_t::digest(pdu.data(), total_body_len);
                                if(checksum != org_checksum) //if checksum fail, return enum_xerror_code_bad_packet
                                {
                                    xwarn_err("xpdu_t::serialize_from,checksum verify fail,checksum=%d vs org_checksum=%d,total_body_len(%d)",checksum,org_checksum,total_body_len);
                                    return enum_xerror_code_bad_packet;
                                }
                            }
                            const int readed_size = do_read(pdu,total_body_len); //directly read from packet
                            if( (readed_size <= 0) || (readed_size > total_body_len) )
                            {
                                xwarn_err("xpdu_t::serialize_from,do_read return error result(%d) vs total_body_len(%d)",readed_size,total_body_len);
                                return enum_xerror_code_bad_packet;
                            }
                            
                            if(readed_size < total_body_len) //exception handle
                            {
                                pdu.pop_front(NULL,(total_body_len - readed_size)); //pop the unrecognized data
                            }
                        }
                    }
                    else//empty body is allow
                    {
                        m_pdu_header.remove_flag(enum_xpacket_encrypted_flag);
                        m_pdu_header.remove_flag(enum_xpacket_compressed_flag);
                    }
                    return total_packet_len;
                }catch(int errorcode)
                {
                    xerror("xpdu_t::serialize_from,throw exception with errocode",errorcode);
                    return enum_xerror_code_bad_packet;
                }
                xerror("xpdu_t::serialize_from,throw unknow exception");
                return enum_xerror_code_bad_packet;
            }
            
            //paired with do_write(xmem_h)
            virtual int32_t serialize_to(xpacket_t & _packet)
            {
                try
                {   
                    xmemh_t & pdu = _packet.get_body();
                    int32_t packet_processs_flags = _packet.get_process_flags();
                    if( ((_packet.get_packet_flags() & enum_xpacket_compressed_flag) == 0) && ((packet_processs_flags & enum_xpacket_process_flag_compress) == 0) ) //not compressed yet and will not ask compress 
                    {
                        if(pdu.size() > 1400) //force to compress if over 1.4K(regular MTU)
                            packet_processs_flags |= enum_xpacket_process_flag_compress;
                    }
                    if(packet_processs_flags & enum_xpacket_process_flag_compress) //check compress first
                    {
                        //first direct write or reserved memory at target packet
                        #ifdef DEBUG
                        const int32_t pdu_head_size = m_pdu_header.serialize_to(pdu,0,0,1); //writed first by dirty data
                        xassert(pdu_head_size == m_pdu_header.get_serialize_size());
                        if(pdu_head_size != m_pdu_header.get_serialize_size())
                            return enum_xerror_code_fail;
                        #else
                        const int32_t pdu_head_size = m_pdu_header.get_serialize_size();
                        pdu.push_back(NULL,pdu_head_size); //reserve the space of head for perormance
                        #endif
                        
                        //then write body data to local memory
                        xautomemh_t<4096> body_block(m_context); //4K is big enough for most packet
                        const int32_t raw_body_size = do_write(body_block,packet_processs_flags);
                        if( (pdu_head_size <= 0) || (raw_body_size < 0) )
                        {
                            xerror("xpdu_t::serialize_to,invalid pdu_head_size(%d) or pdu_body_size(%d)",pdu_head_size,raw_body_size);
                            return enum_xerror_code_fail;
                        }
                        if(raw_body_size != body_block.size())
                        {
                            xerror("xpdu_t::serialize_to,do_write return invalid raw_body_size(%d) != body_block.size(%d)",raw_body_size,body_block.size());
                            return enum_xerror_code_fail;
                        }
                        
                        //now ready to calculate the checksum of original data(at local memory)
                        uint32_t checksum = 0;
                        if(packet_processs_flags & enum_xpacket_process_flag_checksum)
                            checksum = (uint32_t)xhash64_t::digest(body_block.data(), raw_body_size);
                        
                        //reserved enough space  at target packet
                        const int max_body_bound_size = xcompress_t::lz4_get_compressed_bound_size(raw_body_size) + 8;
                        pdu.reserve_back(max_body_bound_size);
                        //xmem_h::reserved(total_capacity),it is differ than xstream_t::reserver(addtional_size)
                        
                        uint8_t compress_rate = 1;
                        //compress data of local and write to target packet
                        int pdu_body_size = xcompress_t::lz4_compress((const char*)body_block.data(), (char*)pdu.back(), raw_body_size, max_body_bound_size);
                        if(pdu_body_size <= 0)
                        {
                            xwarn("xpdu_t::serialize_to,fail to compress raw_body_size(%d) to max_body_bound_size(%d) as result(%d)",raw_body_size,max_body_bound_size,pdu_body_size);
                            
                            pdu_body_size = raw_body_size;
                            memcpy(pdu.data() + pdu.size(),(const uint8_t*)body_block.data(),raw_body_size);
                            m_pdu_header.remove_flag(enum_xpacket_compressed_flag);
                            
                            _packet.set_packet_flag(enum_xpacket_compressed_flag);//add done_compressed flag
                            _packet.remove_process_flag(enum_xpacket_process_flag_compress); //remove compresss command
                        }
                        else
                        {
                            _packet.remove_process_flag(enum_xpacket_process_flag_compress); //remove compresss command
                            
                            const int full_compress_rate = raw_body_size / pdu_body_size + 2;
                            if(full_compress_rate > 255)//compress_rate is just only 8bit
                            {
                                xerror("xpdu_t::serialize_to,over the max compress rate = raw_body_size(%d) / pdu_body_size(%d)",raw_body_size,pdu_body_size);
                                return enum_xerror_code_bad_size; //too big size
                            }
                            m_pdu_header.add_flag(enum_xpacket_compressed_flag);
                            _packet.set_packet_flag(enum_xpacket_compressed_flag);//add done_compressed flag
                            compress_rate = (uint8_t)full_compress_rate;
                        }
                        if(packet_processs_flags & enum_xpacket_process_flag_encrypt) //do CTR encryption
                        {
                            uint32_t  aes_nounce[8]; //32byte
                            for(int i = 0; i < 4; ++i)//16bytes IV -> 4 * 4byte(uint32_t)
                            {
                                aes_nounce[i] = ((uint32_t*)m_aes_iv)[i] + checksum;//mix the checksum
                            }
                            xaes_t::aes_ctr_encrypt_128bit((const uint8_t*)pdu.back(),pdu_body_size,m_aes_key,(uint8_t*)aes_nounce); //CTR not required alignment,so CTR not change data size
                            m_pdu_header.add_flag(enum_xpacket_encrypted_flag);
                        }
                        else
                        {
                            m_pdu_header.remove_flag(enum_xpacket_encrypted_flag);
                        }
                        pdu.push_back(NULL, pdu_body_size); //present the data of body to size
                        
                        //skip offset to original,NOTE:data are still there
                        pdu.pop_back(NULL,pdu_head_size + pdu_body_size);
                        //then rewrite header
                        const int head_writed = m_pdu_header.serialize_to(pdu,pdu_body_size,checksum,compress_rate);
                        if(head_writed != pdu_head_size)
                        {
                            xerror("xpdu_t::serialize_to,fatal error as head_writed(%d) != pdu_head_size(%d)",head_writed,pdu_head_size);
                            return enum_xerror_code_fail;
                        }
                        //finally move to end
                        pdu.push_back(NULL, pdu_body_size);
                        return (pdu_body_size + pdu_head_size);
                    }
                    else
                    {
                        m_pdu_header.remove_flag(enum_xpacket_compressed_flag); //no compression
                        
                        #ifdef DEBUG
                        const int32_t pdu_head_size = m_pdu_header.serialize_to(pdu,0,0,1); //writed first by dirty data
                        xassert(pdu_head_size == m_pdu_header.get_serialize_size());
                        if(pdu_head_size != m_pdu_header.get_serialize_size())
                            return enum_xerror_code_fail;
                        #else
                        const int32_t pdu_head_size = m_pdu_header.get_serialize_size();
                        pdu.push_back(NULL,pdu_head_size); //reserve the space of head for perormance
                        #endif
                        
                        const int32_t pdu_body_size = do_write(pdu,packet_processs_flags);//direct write body to packet
                        if( (pdu_head_size <= 0) || (pdu_body_size < 0) )
                        {
                            xerror("xpdu_t::serialize_to,invalid pdu_head_size(%d) or pdu_body_size(%d)",pdu_head_size,pdu_body_size);
                            return enum_xerror_code_fail;
                        }
                        
                        uint32_t checksum = 0;
                        if(packet_processs_flags & enum_xpacket_process_flag_checksum)
                            checksum = (uint32_t)xhash64_t::digest(((const uint8_t*)pdu.data()) + pdu.size() - pdu_body_size,pdu_body_size);
                        
                        if(packet_processs_flags & enum_xpacket_process_flag_encrypt) //do CTR encryption
                        {
                            uint32_t  aes_nounce[8]; //32byte
                            for(int i = 0; i < 4; ++i)//16bytes IV -> 4 * 4byte(uint32_t)
                            {
                                aes_nounce[i] = ((uint32_t*)m_aes_iv)[i] + checksum;//mix the checksum
                            }
                            xaes_t::aes_ctr_encrypt_128bit(((const uint8_t*)pdu.data()) + pdu.size() - pdu_body_size,pdu_body_size,m_aes_key,(uint8_t*)aes_nounce); //CTR not required alignment,so CTR not change data size
                            m_pdu_header.add_flag(enum_xpacket_encrypted_flag);
                        }
                        else
                        {
                            m_pdu_header.remove_flag(enum_xpacket_encrypted_flag);
                        }
                        //skip offset to original,NOTE:data are still there
                        pdu.pop_back(NULL,pdu_head_size + pdu_body_size);
                        //then rewrite header
                        const int head_writed = m_pdu_header.serialize_to(pdu,pdu_body_size,checksum,1);
                        if(head_writed != pdu_head_size)
                        {
                            xerror("xpdu_t::serialize_to,fatal error as head_writed(%d) != pdu_head_size(%d)",head_writed,pdu_head_size);
                            return enum_xerror_code_fail;
                        }
                        //finally move to end
                        pdu.push_back(NULL, pdu_body_size);
                        return (pdu_body_size + pdu_head_size);
                    }
                }catch(int errorcode)
                {
                    xerror("xpdu_t::serialize_to,throw exception with errocode",errorcode);
                    return enum_xerror_code_fail;
                }
                xerror("xpdu_t::serialize_to,throw unknow exception");
                return enum_xerror_code_fail;
            }
            
            //paired with do_read(xstream_t)
            virtual int32_t serialize_from(xstream_t & pdu) //packet must be a full one that already finish unfragment
            {
                try
                {
                    if(pdu.size() < sizeof(_xbase_header))
                    {
                        xwarn_err("xpdu_t::serialize_from,invalid packet(size:%d)",pdu.size());
                        return enum_xerror_code_bad_packet; //indicate error
                    }
                    
                    _xbase_header * base_header = (_xbase_header*)pdu.data();
                    //const int _protocol =  get_xpacket_protocol(base_header);
                    const int total_head_len = base_header->header_len;
                    int       total_packet_len = base_header->packet_len;
                    if(get_xpacket_exflags_value(base_header->flags) != enum_xpacket_extlength_as_flags) //has extend length
                    {
                        total_packet_len |= (((uint32_t)base_header->extlength) << 16);
                    }
                    const int total_body_len = total_packet_len - total_head_len;
                    if( (pdu.size() < total_packet_len) || (total_body_len < 0) )
                    {
                        xwarn_err("xpdu_t::serialize_from,invalid packet(size:%d) < required size(%d),and total_body_len=%d",pdu.size(),total_packet_len,total_body_len);
                        return enum_xerror_code_bad_packet; //indicate error
                    }
                    
                    uint8_t  compress_rate = 1;
                    uint32_t checksum = 0;
                    const int32_t header_readed_size = m_pdu_header.serialize_from(pdu,checksum,compress_rate);
                    if(header_readed_size != total_head_len)
                    {
                        if(header_readed_size < total_head_len)//recoveable error,a newer version of head coming to old client
                        {
                            pdu.pop_front(total_head_len - header_readed_size);
                            xdbg("xpdu_t::serialize_from,hit extend header,total_head_len(%d) vs header_readed_size(%d)",total_head_len,header_readed_size);
                        }
                        else
                        {
                            xwarn_err("xpdu_t::serialize_from,invalid header(size:%d) > header.pduLen(%d)",header_readed_size,total_head_len);
                            pdu.pop_front(total_packet_len - header_readed_size); //pop out remaining data
                            return enum_xerror_code_bad_packet;
                        }
                    }
                    if(total_body_len > 0)
                    {
                        //deencrypt if enable
                        if( (m_pdu_header.get_flags() & enum_xpacket_encrypted_flag) != 0) //packet has been encrypted by aes
                        {
                            uint32_t  aes_nounce[8]; //32byte
                            for(int i = 0; i < 4; ++i)//16bytes IV -> 4 * 4byte(uint32_t)
                            {
                                aes_nounce[i] = ((uint32_t*)m_aes_iv)[i] + checksum;//mix the checksum
                            }
                            xaes_t::aes_ctr_decrypt_128bit((const uint8_t*)pdu.data(),total_body_len,m_aes_key,(uint8_t*)aes_nounce); //CTR not required alignment,so CTR not change data size
                            m_pdu_header.remove_flag(enum_xpacket_encrypted_flag);//remove flag after deencrypt
                        }
                        //decompress if enable
                        if( (m_pdu_header.get_flags() & enum_xpacket_compressed_flag) != 0)
                        {
                             //reserve enough memory to hold body
                            xautostream_t<8192> body_block(m_context); //8K is big enough for most packet
                            int32_t reserve_size = std::max((int)(compress_rate * total_packet_len),total_packet_len) + 8;
                            if(reserve_size > enum_max_xpacket_len)//should not over it
                            {
                                xerror("xpdu_t::serialize_from,get attack packet,total_packet_len(%d) * compress_rate(%d) > enum_max_xpacket_len",total_packet_len,(int)compress_rate);
                                reserve_size = enum_max_xpacket_len;
                            }
                            reserve_size += 256;
                            //xstream_t::reserver(addtional_size) ,it is differ than xmem_h::reserved(total_capacity)
                            body_block.reserve_back(reserve_size); //how much more need to reserve, refer xbuffer_t::reserver(addtional_size)
                            
                            const int decompressed_size = xcompress_t::lz4_decompress((const char*)pdu.data(),(char*)body_block.back(), total_body_len, body_block.get_spare_size());
                            if(decompressed_size > 0)
                            {
                                body_block.push_back(NULL,decompressed_size); //refill the decompress data
                                m_pdu_header.remove_flag(enum_xpacket_compressed_flag);
                                pdu.pop_front(total_body_len); //pop out pdu after successful decompress
                            }
                            else
                            {
                                xwarn_err("xpdu_t::serialize_from,decompress fail,decompressed_size=%d vs org_total_body_len=%d vs body_block(total:%d,front:%d,back:%d),compress_rate=%d,total_packet_len(%d)",decompressed_size,total_body_len,body_block.get_capacity(),body_block.get_front_offset(),body_block.get_back_offset(),(int)compress_rate,total_packet_len);
                                
                                pdu.pop_front(total_body_len); //pop out the remaining data
                                return enum_xerror_code_bad_packet;
                            }
                            
                            //then do checksum verifiy for raw data now
                            if(checksum != 0)
                            {
                                const uint32_t org_checksum = (uint32_t)xhash64_t::digest(body_block.data(), body_block.size());
                                if(checksum != org_checksum) //if checksum fail, return enum_xerror_code_bad_packet
                                {
                                    xwarn_err("xpdu_t::serialize_from,checksum verify fail,checksum=%d vs org_checksum=%d;total_body_len(%d) vs raw_body_len(%d),compress_rate=%d,total_packet_len(%d)",checksum,org_checksum,total_body_len,decompressed_size,(int)compress_rate,total_packet_len);
                                    return enum_xerror_code_bad_packet;
                                }
                            }
                            
                            const int readed_size = do_read(body_block,body_block.size());
                            if( (readed_size <= 0) || (readed_size > decompressed_size) )
                            {
                                xwarn_err("xpdu_t::serialize_from,do_read return error result(%d) vs total_body_len(%d)",readed_size,total_body_len);
                                return enum_xerror_code_bad_packet;
                            }
                        }
                        else
                        {
                            if(checksum != 0)
                            {
                                const uint32_t org_checksum = (uint32_t)xhash64_t::digest(pdu.data(), total_body_len);
                                if(checksum != org_checksum) //if checksum fail, return enum_xerror_code_bad_packet
                                {
                                    xwarn_err("xpdu_t::serialize_from,checksum verify fail,checksum=%d vs org_checksum=%d,total_body_len(%d)",checksum,org_checksum,total_body_len);
                                    return enum_xerror_code_bad_packet;
                                }
                            }
                            xstream_t body_block(m_context,pdu.data(),total_body_len,0,total_body_len);
                            const int readed_size = do_read(body_block,total_body_len);
                            pdu.pop_front(total_body_len);
                            if( (readed_size <= 0) || (readed_size > total_body_len) )
                            {
                                xwarn_err("xpdu_t::serialize_from,do_read return error result(%d) vs total_body_len(%d)",readed_size,total_body_len);
                                return enum_xerror_code_bad_packet;
                            }
                        }
                    }
                    else//empty body is allow
                    {
                        m_pdu_header.remove_flag(enum_xpacket_encrypted_flag);
                        m_pdu_header.remove_flag(enum_xpacket_compressed_flag);
                    }
                    return total_packet_len;
                }catch(int errorcode)
                {
                    xerror("xpdu_t::serialize_from,throw exception with errocode",errorcode);
                    return enum_xerror_code_bad_packet;
                }
                xerror("xpdu_t::serialize_from,throw unknow exception");
                return enum_xerror_code_bad_packet;
            }
            
            //paired with do_write(xstream_t)
            virtual int32_t serialize_to(xstream_t & pdu,int32_t packet_processs_flags = (enum_xpacket_process_flag_compress | enum_xpacket_process_flag_checksum) )
            {
                try
                {
                    if(packet_processs_flags & enum_xpacket_process_flag_compress) //check compress first
                    {
                        //first direct write or reserved memory at target packet
                        #ifdef DEBUG
                        const int32_t pdu_head_size = m_pdu_header.serialize_to(pdu,0,0,1); //writed first by dirty data
                        xassert(pdu_head_size == m_pdu_header.get_serialize_size());
                        if(pdu_head_size != m_pdu_header.get_serialize_size())
                            return enum_xerror_code_fail;
                        #else
                        const int32_t pdu_head_size = m_pdu_header.get_serialize_size();
                        pdu.push_back(NULL,pdu_head_size); //reserve the space of head for perormance
                        #endif
                        
                        //then write body data to local memory
                        xautostream_t<8192> body_block(m_context); //8K is big enough for most packet
                        const int32_t raw_body_size = do_write(body_block,packet_processs_flags);
                        if( (pdu_head_size <= 0) || (raw_body_size < 0) )
                        {
                            xerror("xpdu_t::serialize_to(stream),invalid pdu_head_size(%d) or pdu_body_size(%d)",pdu_head_size,raw_body_size);
                            return enum_xerror_code_fail;
                        }
                        if(raw_body_size != body_block.size())
                        {
                            xerror("xpdu_t::serialize_to(stream),do_write return invalid raw_body_size(%d) != body_block.size(%d)",raw_body_size,body_block.size());
                            return enum_xerror_code_fail;
                        }
                        
                        //now ready to calculate the checksum of original data(at local memory)
                        uint32_t checksum = 0;
                        if(packet_processs_flags & enum_xpacket_process_flag_checksum)
                            checksum = (uint32_t)xhash64_t::digest(body_block.data(), raw_body_size);
                        
                        //reserved enough space  at target packet
                        const int max_body_bound_size = xcompress_t::lz4_get_compressed_bound_size(raw_body_size) + 8;
                        const int revered_result = pdu.reserve_back(max_body_bound_size + 56); //how much more need expend
                        xassert(revered_result > max_body_bound_size);
                        
                        uint8_t compress_rate = 1;
                        //compress data of local and write to target packet
                        int pdu_body_size = xcompress_t::lz4_compress((const char*)body_block.data(), (char*)pdu.back(), raw_body_size, max_body_bound_size);
                        if(pdu_body_size <= 0)
                        {
                            xwarn_err("xpdu_t::serialize_to(stream),fail to compress raw_body_size(%d) to max_body_bound_size(%d)",raw_body_size,max_body_bound_size);
                            
                            pdu_body_size = raw_body_size;
                            memcpy(pdu.back(),(const uint8_t*)body_block.data(),raw_body_size);
                            m_pdu_header.remove_flag(enum_xpacket_compressed_flag);
                        }
                        else
                        {
                            const int full_compress_rate = raw_body_size / pdu_body_size + 2;
                            if(full_compress_rate > 255)//compress_rate is just only 8bit
                            {
                                xerror("xpdu_t::serialize_to(stream),over the max compress rate = raw_body_size(%d) / pdu_body_size(%d)",raw_body_size,pdu_body_size);
                                return enum_xerror_code_bad_size; //too big size
                            }
                            m_pdu_header.add_flag(enum_xpacket_compressed_flag);
                            compress_rate = (uint8_t)full_compress_rate;
                        }
                        if(packet_processs_flags & enum_xpacket_process_flag_encrypt) //do CTR encryption
                        {
                            uint32_t  aes_nounce[8]; //32byte
                            for(int i = 0; i < 4; ++i)//16bytes IV -> 4 * 4byte(uint32_t)
                            {
                                aes_nounce[i] = ((uint32_t*)m_aes_iv)[i] + checksum;//mix the checksum
                            }
                            xaes_t::aes_ctr_encrypt_128bit((const uint8_t*)pdu.back(),pdu_body_size,m_aes_key,(uint8_t*)aes_nounce); //CTR not required alignment,so CTR not change data size
                            m_pdu_header.add_flag(enum_xpacket_encrypted_flag);
                        }
                        else
                        {
                            m_pdu_header.remove_flag(enum_xpacket_encrypted_flag);
                        }
                        pdu.push_back(NULL, pdu_body_size); //present the data of body to size
                        
                        //skip offset to original,NOTE:data are still there
                        pdu.pop_back(pdu_head_size + pdu_body_size);
                        //then rewrite header
                        const int head_writed = m_pdu_header.serialize_to(pdu,pdu_body_size,checksum,compress_rate);
                        if(head_writed != pdu_head_size)
                        {
                            xerror("xpdu_t::serialize_to(stream),fatal error as head_writed(%d) != pdu_head_size(%d)",head_writed,pdu_head_size);
                            return enum_xerror_code_fail;
                        }
                        //finally move to end
                        pdu.push_back(NULL, pdu_body_size);
                        return (pdu_body_size + pdu_head_size);
                    }
                    else
                    {
                        m_pdu_header.remove_flag(enum_xpacket_compressed_flag); //no compression
                        #ifdef DEBUG
                        const int32_t pdu_head_size = m_pdu_header.serialize_to(pdu,0,0,1); //writed first by dirty data
                        xassert(pdu_head_size == m_pdu_header.get_serialize_size());
                        if(pdu_head_size != m_pdu_header.get_serialize_size())
                            return enum_xerror_code_fail;
                        #else
                        const int32_t pdu_head_size = m_pdu_header.get_serialize_size();
                        pdu.push_back(NULL,pdu_head_size); //reserve the space of head for perormance
                        #endif
                        
                        const int32_t pdu_body_size = do_write(pdu,packet_processs_flags);//direct write body to packet
                        if( (pdu_head_size <= 0) || (pdu_body_size < 0) )
                        {
                            xerror("xpdu_t::serialize_to(stream),invalid pdu_head_size(%d) or pdu_body_size(%d)",pdu_head_size,pdu_body_size);
                            return enum_xerror_code_fail;
                        }
                        
                        uint32_t checksum = 0;
                        if(packet_processs_flags & enum_xpacket_process_flag_checksum)
                            checksum = (uint32_t)xhash64_t::digest(((const uint8_t*)pdu.data()) + pdu.size() - pdu_body_size,pdu_body_size);
                        
                        if(packet_processs_flags & enum_xpacket_process_flag_encrypt) //do CTR encryption
                        {
                            uint32_t  aes_nounce[8]; //32byte
                            for(int i = 0; i < 4; ++i)//16bytes IV -> 4 * 4byte(uint32_t)
                            {
                                aes_nounce[i] = ((uint32_t*)m_aes_iv)[i] + checksum;//mix the checksum
                            }
                            xaes_t::aes_ctr_encrypt_128bit(((const uint8_t*)pdu.data()) + pdu.size() - pdu_body_size,pdu_body_size,m_aes_key,(uint8_t*)aes_nounce); //CTR not required alignment,so CTR not change data size
                            m_pdu_header.add_flag(enum_xpacket_encrypted_flag);
                        }
                        else
                        {
                            m_pdu_header.remove_flag(enum_xpacket_encrypted_flag);
                        }
                        //skip offset to original,NOTE:data are still there
                        pdu.pop_back(pdu_head_size + pdu_body_size);
                        //then rewrite header
                        const int head_writed = m_pdu_header.serialize_to(pdu,pdu_body_size,checksum,1);
                        if(head_writed != pdu_head_size)
                        {
                            xerror("xpdu_t::serialize_to(stream),fatal error as head_writed(%d) != pdu_head_size(%d)",head_writed,pdu_head_size);
                            return enum_xerror_code_fail;
                        }
                        //finally move to end
                        pdu.push_back(NULL, pdu_body_size);
                        return (pdu_body_size + pdu_head_size);
                    }
                }catch(int errorcode)
                {
                    xerror("xpdu_t::serialize_to(stream),throw exception with errocode",errorcode);
                    return enum_xerror_code_fail;
                }
                xerror("xpdu_t::serialize_to(stream),throw unknow exception");
                return enum_xerror_code_fail;
            }

        protected: //note:subclass must override the related do_read and do_write function
            
            //paired with seriliaze_from(xpacket_t) and seriliaze_to(xpacket_t)
            virtual int32_t     do_read(xmemh_t & archive,const int32_t pdu_body_size) { xassert(0); return 0;}
            virtual int32_t     do_write(xmemh_t & archive,int32_t & packet_processs_flags){ xassert(0); return 0;}
            
            //paired with seriliaze_from(xstream_t) and seriliaze_to(xstream_t)
            virtual int32_t     do_read(xstream_t & archive,const int32_t pdu_body_size){ xassert(0); return 0;}
            virtual int32_t     do_write(xstream_t & archive,int32_t & packet_processs_flags){ xassert(0); return 0;}
            
        protected:
            __T_PDU_HEADER__  m_pdu_header;
            uint8_t           m_aes_key[16]; //128bit AES Key for CTR Mode
            uint8_t           m_aes_iv[16];  //128bit AES IV  for CTR Mode
            xcontext_t &      m_context;
        };
    };//end of namespace of base
};//end of namespace of top
