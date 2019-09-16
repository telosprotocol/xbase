// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <mutex>
#include "xatom.h"
#include "xobject.h"
#include "xpacket.h"
#include "xtimer.h"

namespace  top
{
    namespace base
    {
        //endpoint represent a network or communication related stuff like virtual socket/connection/channel/network port/hub/switch/router..
        //each endpoint must has one XIP address associated
        class xendpoint_t : public xiobject_t
        {
            friend class xsocket_t;
            friend class xendgroup_t;
            friend class xlrouter_t;
            friend class xswitch_t;
        public:
            enum  enum_endpoint_type
            {
                enum_endpoint_type_group        = 0,  //use to queryinterface
                enum_endpoint_type_lrouter      = 1,  //simulate network local router(routable switch) device with routing function
                enum_endpoint_type_switch       = 2,  //simulate network switch device with limited routing ability
                enum_endpoint_type_hub          = 3,  //simulate network hub device (always broadcast)
                enum_endpoint_type_dealer       = 4,  //general/default load-balance endpoint
                enum_endpoint_type_xipconnect   = 5,  //xip layer
                enum_endpoint_type_connection   = 6,  //connection layer
                enum_endpoint_type_xsocket      = 7,  //physical socket endpoint(TCP,SCTP,UDP,RUDP),or customized socket
                enum_endpoint_type_xservice     = 9,  //xservice object
                enum_endpoint_type_xnode        = 10, //xnode
                enum_endpoint_type_xwrouter     = 11, //xwrouter, WAN Router with routing function
                enum_endpoint_type_xgateway     = 12, //xgateway
                enum_endpoint_type_xproxy       = 13, //sit at middle and exchange packet betweeen up and low
                enum_endpoint_type_customized   = 20,
            };
            
            enum  enum_endpoint_status
            {
                enum_endpoint_status_empty      = 0,
                enum_endpoint_status_detaching  = 1,
                enum_endpoint_status_attaching  = 2,
                enum_endpoint_status_open       = 3,
            };
            
            enum enum_endpoint_load
            {
                enum_endpoint_load_min        = 0,    //init load
                enum_endpoint_load_light      = 40,   ///< 40% is light laod
                enum_endpoint_load_heavy      = 80,   //>=80% enter heavy mode
                enum_endpoint_load_max        = 100,  //max usable load
                enum_endpoint_load_unavaiable = 101,  //endpoint not availabe
            };
            
            enum enum_endpoint_quality
            {
                enum_endpoint_quality_unavaiable    = 1,    //can not use
                enum_endpoint_quality_verybad       = 10,   //almost useless
                enum_endpoint_quality_bad           = 30,   //bad quality
                enum_endpoint_quality_low           = 45,   //low quality
                enum_endpoint_quality_average       = 65,   //average quality
                enum_endpoint_quality_good          = 80,   ///< 40% is light laod
                enum_endpoint_quality_excelent      = 90,   //>=80% enter heavy mode
                enum_endpoint_quality_max           = 100,  //100% satification
            };
        protected:
            xendpoint_t(xcontext_t & _context,const int32_t thread_id,xendpoint_t * parent,xendpoint_t* child,enum_endpoint_type endtype);
            virtual ~xendpoint_t();
        private:
            xendpoint_t();
            xendpoint_t(const xendpoint_t &);
            xendpoint_t & operator = (const xendpoint_t &);
        public:
            inline enum_endpoint_type      get_type() const {return m_endpoint_type;}
            inline xendpoint_t*            get_parent() const{return m_parent;}
            inline void                    get_xip_address(uint64_t & low64bit_addr,uint64_t & high64bit_addr) const;
            inline uint64_t                get_xip_address_low64bit() const {return m_xip_address_low;}   //low  64bit xip address
            inline uint64_t                get_xip_address_high64bit() const {return m_xip_address_high;} //high 64bit xip address
            inline uint16_t                get_xip_port() const {return m_xip_port;}
            inline uint32_t                get_access_token() const {return m_access_token;}
            inline uint64_t                get_last_keeplive_time() const {return m_last_keeplive_time;}
            
            inline int32_t                 get_network_id() const {return m_network_id;}
            inline int32_t                 get_network_version() const {return m_network_ver;}
            inline uint32_t                get_interface_id() const {return m_interface_id;}
            inline int32_t                 get_network_type() const {return m_network_type;}
            inline uint32_t                get_server_id() const {return m_server_id;}
            inline int32_t                 get_process_id() const {return m_process_id;}
            inline int32_t                 get_router_id() const {return m_router_id;}
            inline int32_t                 get_switcher_id() const {return m_switcher_id;}
            inline int32_t                 get_local_id()  const {return m_local_id;}
            
            /*refer enum_xpacket_flags
             enum_xpacket_reliable_type_must      = (3 << 14),  //default,not dropable,signal or message
             enum_xpacket_reliable_type_most      = (2 << 14),  //smallest dropable or Video KeyFrame, and retry best(2 time for rudp, use premium route path)
             enum_xpacket_reliable_type_medium    = (1 << 14),  //samll dropable allow(e.g raw voice), retry 1 more time when loss (pick alternative routing)
             enum_xpacket_reliable_type_low       = (0 << 14),  //big dropable allow(e.g. FEC packet), no retry for packet lost and  just rely on ip like udp
             */
            //return highest level of reliable(refer above),as default it is enum_xpacket_reliable_type_must
            inline int                     get_max_packet_reliable_level() const {return m_max_packet_reliable_level;}
            /*
             enum_xpacket_order_type_must         = (1 << 11),  //default
             enum_xpacket_order_type_free         = (0 << 11),  //when set,allow deliver packet as non-order
             */
            //return highest level of order(refer above),as default it is enum_xpacket_order_type_must
            inline int                     get_max_packet_order_level() const {return m_max_packet_order_level;}

            //error code and string management
            virtual int                    get_last_error() const {return m_last_error;}
            virtual void                   set_last_error(const int error_code) { m_last_error = error_code;}
            virtual std::string            get_last_error_string(){return std::string();}
            virtual void                   set_access_token(const uint32_t token);
            //load and quality is dynamic,so using virtual function to let subcalss customize it
            //refer enum_endpoint_load,if load > 100 when not avaiable,return 100 when full address are using
            virtual  uint32_t       get_load() {return m_load;}
            virtual  uint32_t       get_quality() {return m_quality;} //refer enum_endpoint_quality based on ping/echo test
            virtual  void           set_quality(const uint8_t quality){ m_quality = quality;}
            virtual  void           set_load(const uint8_t load){ m_load = load;}
            
            virtual  int            is_alive(uint64_t timenow_ms);//return how many ms left before expire, return <= 0 if expired
            virtual  void*          query_interface(const int32_t type) override;
            
			#ifdef __DEBUG_ENDPOINT_OBJECT_LIFETIME__
            virtual  int32_t        add_ref() override ;
            virtual  int32_t        release_ref() override;
			#endif
        public: //multiple_thread safe
            //as default test whether target_address is equal as m_address
            //subcalss may overwrite this logic when multiple xip address bind to one endpoint
            virtual bool            is_match(const uint64_t target_xip_address_low,const uint64_t target_xip_address_high)
            {
                return (is_xip_address_equal(target_xip_address_low,m_xip_address_low) && (target_xip_address_high == m_xip_address_high));
            }
            
            //if cur_thread_id 0 xsocket_t do query current thread id again. same for timenow_ms.
            //return errorcode -refer  enum_error_code,return enum_code_successful if the packet write to system buffer
            //if packet is just caching at xsocket_t buffer it return enum_code_queue_up;
            //"from_parent_end" is the upper endpoint that fire packet,it might be released since it just be hint, and might be NULL if it is from othes intead of parent node
            //send  search to_xip_addr from top to down and handle it per address match or endpoint type
            //from_xip_addr_high and to_xip_addr_high is 0 for XIP1 Address
            virtual int32_t          send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end);
        
        protected: //recv may be called from io-thread when packets is ready to handle
            //recv search target from lower layer(child) to upper layer(parent) by to_xip_addr.
            //usally it been as callback from childnode'host thread,from_child_end indicate where is the packet from, if from_child_end is NULL which means it is from other instead of it'childs
            //Note:from_child_end might be released since it is just hint.
            //recv search to_xip_addr from down to up and handle it per address match and endpoint type
            //from_xip_addr_high and to_xip_addr_high is 0 for XIP1 Address
            virtual int32_t          recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end);
            
        protected:
            //packet is belong to this endpoint who need process this packet,when on_packet_handle is called
            //on_packet_handle maybe called from multiple-threads, it must be multip-thread safe when implement it
            virtual int32_t         on_packet_handle(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_end);//could be from parent endpoint or child endpoint
            
            //can only be called from host thread, errorcode refer enum_xerror_code,return true when the event is handled
            //packet.size() > 0  when the packet is not completely sent out, 0 means the whole packet has done
            //fail when error_code != 0,error code refer enum_xerror_code
            //Note: dont hold packet object that will destroyed immediately after on_packet_send
            virtual int32_t         on_packet_send(const int32_t error_code,xpacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_end);
            
        protected: //Note: child'host thread maybe is different from parent 'host thread
            /*  Start process of child endpoint
               1. Call attach_endpoint() of parent endpoint ->the child enpoint is called by on_endpoint_attached from parent endpoint when successful
               2. The child enpoint trigger open process(prepare for use),and notify parent endpoint when ready to use
               3. Parent Endpoint is notificed by on_endpoint_open from child endpoint when child endpoint is ready
            */
            /* Close process of child endpoint
                1. Call detach_endpoint() of parent endpoint ->the child enpoint is called by on_endpoint_detached from parent endpoint  when successful
                2. The child enpoint trigger close process(prepare for stop),and notify parent endpoint when finish the close
                3. Parent Endpoint is notificed by on_endpoint_close from child endpoint when child endpoint is completely stopped & close
             
                OR
                1. Child Endpoint directly Close itself -> The child enpoint trigger close process(prepare for stop),and notify parent endpoint when finish the close
                2. Parent Endpoint is notificed by on_endpoint_close from child endpoint when child endpoint is completely stopped & close
             */
            
            //notify this node is attached/detached into parent node
            //current node(this) been attached to parent,errorcode refer enum_error_code ,return true when the event is handled
            virtual bool             on_endpoint_attach(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,const uint64_t end_address_low,const uint64_t end_address_high,const uint32_t auth_access_token,xendpoint_t* from_parent);
            
            //current node(this) been detached from parent
            virtual bool             on_endpoint_detach(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,const uint64_t end_address_low,const uint64_t end_address_high,xendpoint_t* from_parent);
            
            //notify the child endpont is ready to use when receive on_endpoint_open of error_code = enum_xcode_successful
            virtual bool             on_endpoint_open(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child);
            //when associated io-object close happen,post the event to receiver
            //error_code is 0 when it is closed by caller/upper layer
            virtual bool             on_endpoint_close(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child);
            
            //notify upper layer from child endpoint when update keeplive status,note: _payload is customized content by sender,and most time it is empty
            virtual bool             on_endpoint_keepalive(const std::string & _payload,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child);
            
            //notify upper layer from child endpoint when recv ping,note: _payload is customized content by sender
            virtual bool             on_endpoint_ping(const std::string & _payload,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child);
            
            //notify upper layer from child endpoint when recv signal packet,note: _payload is customized content by sender
            virtual bool             on_endpoint_signal(const std::string & _payload,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child);
        protected:
            virtual bool             on_object_close() override; //notify the subclass the object is closed
            virtual void             set_xip_address(const uint64_t low64bit_addr,const uint64_t high64bit_addr);
            virtual void             set_xip_port(const uint16_t port);
            bool                     set_child(xendpoint_t * child_end);//note: can only be called by subclass when construction stage
            //note:set_child(child) may reset the child'address and token,so here provide extend implementation
            bool                     set_child(xendpoint_t * child_end,const uint64_t child_end_address_low,const uint64_t child_end_address_high,const uint32_t child_auth_access_token);
            
            xendpoint_t*             get_child() {return m_child;}
            xendpoint_t*             get_parent_node(int32_t cur_thread_id);
        private: //as default 1-1 mode with 1 public address
            xendpoint_t*            m_parent;           //point to upper layer,private to avoid confilict with juxendpoint_t
            xendpoint_t*            m_child;            //point to next layer,private to avoid confilict with Juendgroup_t
            uint64_t                m_xip_address_low;  //low 64bit address
            uint64_t                m_xip_address_high; //high 64bit address
            uint16_t                m_xip_port;         //xip bind or listen port
            uint16_t                m_reserved16;
            uint32_t                m_access_token;     //allow free access if it is 0.paired with m_xip_address
        protected:  //performance-help property that get from m_xip_address_low
            int32_t                 m_network_id;       //24bit:id,it is 0 as default
            int32_t                 m_network_ver;      //refer XIP2
            uint32_t                m_interface_id;
            uint32_t                m_server_id;        //tell where is for this router
            enum_xnetwork_type      m_network_type;
            int32_t                 m_process_id;
            int32_t                 m_router_id;
            int32_t                 m_switcher_id;
            int32_t                 m_local_id;
        protected:
            enum_endpoint_type      m_endpoint_type;
            uint8_t                 m_load;             //endpoint'current load,refer enum_endpoint_load,default is enum_endpoint_load_min
            uint8_t                 m_quality;          //endpoint'current quality,refer enum_endpoint_quality,default is enum_endpoint_quality_max
            uint16_t                m_max_packet_reliable_level; //how endpoint send packet as reliable,refer enum_xpacket_flag
            uint16_t                m_max_packet_order_level;    //how endpoint send packet as order,refer enum_xpacket_flag
            int16_t                 m_last_error;
            uint64_t                m_last_keeplive_time; //ms,//when receive the last keeplive report from child
        };
    
        class xendgroup_t : public xendpoint_t
        {
        protected:
            xendgroup_t(xcontext_t & context,const int32_t thread_id,xendpoint_t * parent,enum_endpoint_type endtype);
            virtual ~xendgroup_t();
        private:
            xendgroup_t();
            xendgroup_t(const xendgroup_t &);
            xendgroup_t & operator = (const xendgroup_t &);
        public:
            enum
            {
                enum_max_child_nodes             = 256,    //node(0) is reserved
                enum_xip_idle_max_duration       = 10000,  //after 10seconds,reuse the idle address(attaching but not attached any object)
            };
            
            enum enum_addr_property_type
            {
                enum_addr_property_type_unalloc  = 0,  //freed
                enum_addr_property_type_private  = 1,  //internal created and managed
                enum_addr_property_type_public   = 2,  //client attached
            };
            
            struct end_node_t
            {
                end_node_t();
            public:
                //Note at ARM/x86 64bit assign operation might be not 100% atomic,however uint8_t/int32_t is 100% atomic by given constat value
                xendpoint_t*               endpoint;   //point to object
                void*                      cookie;     //depends purpose
                uint64_t                   address;    //for performance, copy low64 bit address here.high64 bit address mostly not changed once set
                uint64_t                   last_time;  //last live time,do keep alive and connect timeout
                uint32_t                   auth_token; //a random authentication token for this address
                uint8_t                    status;     //refer enum_endpoint_status
                uint8_t                    magic;      //use to generate dynamic address
                uint8_t                    flags;      //flags about enum_packet_reliable_type,enum_packet_order_type
                uint8_t                    addrproperty;  //refer enum_addr_property_type
            };
        public:
            virtual int32_t          send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end) override;
 
            virtual int32_t          recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override;
            
        public: //multiple_thread safe
            //return one public XIP address(aka low64bit address of XIP2), OR private address(< 256) OR return 0 if nothing can alloc
            virtual  uint64_t       alloc_address(uint32_t & auth_access_token);
            
            virtual  bool           attach_endpoint(xendpoint_t * child,const uint64_t target_address_low64bit,const uint64_t target_address_high64bit,xfunction_t * post_action);//attached to specified addr
            //Note: once detach,it auto closed to prevent use again and  caller respond to destroy object by release_ref
            virtual  bool           detach_endpoint(xendpoint_t * child,const uint64_t child_address_low64bit,const uint64_t child_address_high64bit,xfunction_t * post_action);
            
        protected: //notify this group self that one child node is attached or detached
            virtual bool            on_endpoint_join(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,const uint64_t child_address_low64bit,const uint64_t child_address_high64bit,xendpoint_t* child);
            //current node(this) been detached from parent,end_address is detached address if successful
            virtual bool            on_endpoint_leave(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,const uint64_t child_address_low64bit,const uint64_t child_address_high64bit,xendpoint_t* child);
            
        protected://notification about close event
            virtual bool             on_object_close() override; //notify the subclass the object is closed
            //when associated child endpoint is close,post the event to receiver(this group),error_code is 0 when it is closed by caller
            virtual bool             on_endpoint_close(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from) override;
 
        protected:
            bool                     has_valid_nodes();     //tell whether has valid nodes now
            int32_t                  get_used_address();    //how many address are opened/how many nodes are avaiable to use
        private:
            //target_address updated by final alocated address once successful
            virtual bool             attach_endpoint_internal_imp(xendpoint_t *child,uint64_t & target_address_low64bit,uint64_t & target_address_high64bit,const int32_t cur_thread_id,const uint64_t timenow_ms,const uint64_t cookie = (uint64_t)-1);
            virtual bool             detach_endpoint_internal_imp(xendpoint_t *child,const uint64_t child_address_low64bit,const uint64_t child_address_high64bit,const int32_t cur_thread_id,const uint64_t timenow_ms);
        protected:
            end_node_t               m_child_nodes[enum_max_child_nodes]; //a fixed array inited at contruction
            
            int32_t                  m_last_node_index;     //for performance optimization
            int32_t                  m_next_alloc_index;    //just hint to alloca from this index
            int32_t                  m_total_alloc_address; //how many address allocated;
            int32_t                  m_total_open_address;  //how many address allocated with open status;
            
            uint8_t                  padding_1[7];
            uint8_t                  m_const_childnodes;  //optimization hint to tell only one child that inited at construction
        };

        //xswitch_t simulate network switch device with limited routing ability
        //total 2^8 - 1 = 255 socket address can alloced
        class xswitch_t : public xendgroup_t
        {
            friend class xlrouter_t;  //only xrouter_t can create it
        private: //not allow create from outside or subclass
            xswitch_t(xcontext_t & context,const int32_t thread_id,const uint64_t switcher_address_low,const uint64_t switcher_address_high,xendpoint_t & parent);
            virtual ~xswitch_t();
        private:
            xswitch_t();
            xswitch_t(const xswitch_t &);
            xswitch_t & operator = (const xswitch_t &);
        private:
            //only allow to be called from xrouter_t
            virtual int32_t     send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end) override;
            
            //only allow to be called through xendpoint_t interface
            virtual int32_t     recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override;
        public:
            virtual bool        is_match(const uint64_t target_xip_address_low,const uint64_t target_xip_address_high) override;
            
            virtual  uint64_t   alloc_xip_address(uint32_t & auth_access_token);//alloc address from any empty slot
            //return the XIP address(aka low64bit address of XIP2);
            virtual  uint64_t   alloc_xip_address(const uint32_t node_index,uint32_t & auth_access_token);//alloc address by specified node index(most for service address)
            
            //target_address MUST be local and valid address,if successful return valid endpoint object pointer
            //Note: the returned pointer that may release soon. need addref if need hold longer
            virtual  xendpoint_t* find_endpoint(const uint64_t target_xip_addr_low64bit,const uint64_t target_xip_addr_high64bit,uint32_t & auth_token);
            
            virtual  bool         attach_endpoint(xendpoint_t * child,const uint64_t target_address_low64bit,const uint64_t target_address_high64bit,xfunction_t * post_action) override;//attached to specified addr
            //Note: once detach,it auto closed to prevent use again and  caller respond to destroy object by release_ref
            virtual  bool         detach_endpoint(xendpoint_t * child,const uint64_t child_address_low64bit,const uint64_t child_address_high64bit,xfunction_t * post_action) override;
        protected: //can only be called from host thread, errorcode refer enum_error_code ,return true when the event is handled
            //on_endpoint_connect is a virtual concept that could be socket connect/connetion connect/attached to router
            virtual bool        on_endpoint_connect(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from){return false;}
            
        protected: //can only be called from host thread, errorcode refer enum_error_code,return true when the event is handled
            //packet.size() > 0  when the packet is not completely sent out, 0 means the whole packet has done
            //write fail when error_code != 0,error code refer enum_error_code
            //Note: dont hold packet object that will destroyed immediately after on_packet_write
            virtual int32_t     on_packet_send(const int32_t error_code,xpacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from) override {return enum_xcode_successful;}
        };
        
        class xnode_t;
        //total 2^8 - 1 = 255 switch can alloced
        //simulate the LAN' router device with routing function,note: only used at LAN network
        class xlrouter_t : public xendgroup_t
        {
            friend class xnode_t;  //only allow xnode_t to create xrouter_t
            friend class xswich_t; //only allow xswitch to call recv
        private: //not allow create from outside or subclass
            xlrouter_t(xcontext_t & context,const int32_t thread_id,const uint64_t router_address_low64bit,const uint64_t router_address_high64bit,xnode_t & parent);
            virtual ~xlrouter_t();
        private:
            xlrouter_t();
            xlrouter_t(const xlrouter_t &);
            xlrouter_t & operator = (const xlrouter_t &);
        private: //multiple_thread safe
            //only allow xnode_t to call send
            virtual int32_t     send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end) override;
            //only allow xswitch_t to call recv
            virtual int32_t     recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override;
            
        public: //multiple_thread safe
            virtual bool        is_match(const uint64_t target_address_low64bit,const uint64_t target_address_high64bit) override;
            
            //service occupy special address that reserved first switch(only 255 address)
            virtual  uint64_t   alloc_xip_address(uint32_t & auth_access_token); //alloc address directly
            virtual  uint64_t   alloc_xip_address(const uint8_t service_addr,uint32_t & auth_access_token); //alloc service address directly
            //if target_address <= 0 router may assign a public address automatically.
            //if target_address > 0, target_address must be a public and valid address.
            virtual  bool        attach_endpoint(xendpoint_t * child,uint64_t target_address_low64bit,const uint64_t target_address_high64bit,xfunction_t * post_action) override;
            //target_address must be a public and valid address.
            virtual  bool        detach_endpoint(xendpoint_t* child,const uint64_t child_address_low64bit,const uint64_t child_address_high64bit,xfunction_t * post_action) override;
            //target_address must be a public and valid address.if successful return valid endpoint object pointer
            //Note: the returned pointer might be release very soon. please add reference to hold longer
            virtual  xendpoint_t* find_endpoint(const uint64_t target_xip_addr_low64bit,const uint64_t target_xip_addr_high64bit,uint32_t & auth_token);
            
        protected://router can not be attached to another node
            //current router(this) been attached to parent
            virtual bool             on_endpoint_attach(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,const uint64_t end_address_low,const uint64_t end_address_high,const uint32_t auth_access_token,xendpoint_t* from_parent) override {return false;}
            //current router(this) been detached from parent
            virtual bool             on_endpoint_detach(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,const uint64_t end_address_low,const uint64_t end_address_high,xendpoint_t* from_parent) override {return false;}

        protected: //can only be called from host thread, errorcode refer enum_error_code ,return true when the event is handled
            //on_endpoint_connect is a virtual concept that could be socket connect/connetion connect/attached to router
            virtual bool             on_endpoint_open(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from) override {return false;}
 
            //when associated io-object close happen,post the event to receiver
            //error_code is 0 when it is closed by caller/upper layer
            virtual bool             on_endpoint_close(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from) override {return false;}
            
        protected: //can only be called from host thread, errorcode refer enum_error_code,return true when the event is handled
            //packet.size() > 0  when the packet is not completely sent out, 0 means the whole packet has done
            //write fail when error_code != 0,error code refer enum_error_code
            //Note: dont hold packet object that will destroyed immediately after on_packet_write
            virtual int32_t           on_packet_send(const int32_t error_code,xpacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from) override {return enum_xcode_successful;}
        protected:
            virtual bool  on_object_close() override;//notify the subclass the object is closed
        };
        
        //xnode_t represent a host,or a process of the host
        class xnode_t : public xendpoint_t
        {
            friend class xcontext_t;
            friend class xlrouter_t;
        protected:
            //node_addr = [zone-id:8bit|cluster-id:8bit|group-id:8bit|node-id:8bit]
            //the valid process_id is [0,15],0 means not specified process_id
            //xnode_t may generate the node_xip_address_low64bit based on server_id and process_id
            xnode_t(xcontext_t & context,const int32_t thread_id,const uint64_t node_xip_address_high64bit,const uint32_t node_addr,const uint32_t process_id);
            virtual ~xnode_t();
        private:
            xnode_t();
            xnode_t(const xnode_t &);
            xnode_t & operator = (const xnode_t &);
        public:
            xlrouter_t*    create_router(enum_xnetwork_type net_type,enum_xip_type addr_type, int32_t target_thread_id);
            xlrouter_t*    find_router(enum_xnetwork_type nettype,const int32_t router_id);
            xendpoint_t*   find_endpoint(const uint64_t target_xip_addr_low64bit,const uint64_t target_xip_addr_high64bit,uint32_t & auth_token);
            
            virtual void*  query_interface(const int32_t type) override; //caller respond to cast (void*) to related  interface ptr
            virtual bool   is_match(const uint64_t target_address_low64bit,const uint64_t target_address_high64bit) override;
        protected:
            virtual bool   on_object_close() override; //notify the subclass the object is closed
            virtual bool   on_lrouter_up(xlrouter_t * local_router);    //router is going to work
            virtual bool   on_lrouter_down(xlrouter_t * local_router);  //rtouer is going to close
        private:
            //just allow xcontext_t call send
            virtual int32_t   send(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_parent_end) override;
            
            //just allow xrouter call recv
            virtual int32_t   recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_end) override;
        private:
            std::recursive_mutex  m_mutex;
            xlrouter_t*      m_routers[const_max_xnetwork_types_count][const_max_routers_count];
        };
    }; //end of namespace of base
}; //end of namesapce of top

