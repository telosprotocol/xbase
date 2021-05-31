// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <memory>
#include "xrefcount.h"

namespace top
{
    namespace base
    {
        enum enum_xevent_route_path
        {
            enum_xevent_route_path_down     = 0x01,  //event go down from higher layer/object to lower layer
            enum_xevent_route_path_up       = 0x02,  //event go up   from lower  layer/object to upper layer
            enum_xevent_route_path_by_mbus  = 0x04,  //event go through mbus channel
            enum_xevent_route_path_any      = 0x07,  //to anyone that may handle it
        };
        
        enum enum_xevent_type
        {
            enum_xevent_app_type_max      = 32765,
            enum_xevent_app_type_min      =  1,
            
            enum_xevent_type_invalid      =  0, //below event types are reserved by xbase
            enum_xevent_core_type_mbus    = -1, //mbus event
            
            enum_xevent_core_type_pdu     = -5, //that is a event of pdu(usally from/to network)
            enum_xevent_core_type_timer   = -6, //that is a system event of time,more about NTP change
            enum_xevent_core_type_clock   = -7, //that is an event of global clock(distributed logic clock)
            enum_xevent_core_type_create_block = -8, //that is an event to create block by upper layer and pass back to lowwer layer
            enum_xevent_core_type_tc      = -9, //that is an event of time cert block
        };
        
        //general event wrap
        class xvevent_t : virtual public xrefcount_t
        {
        protected:
            xvevent_t(const int _event_type);
            virtual ~xvevent_t();
        private:
            xvevent_t();
            xvevent_t(xvevent_t &&);
            xvevent_t(const xvevent_t & obj);
            xvevent_t& operator = (const xvevent_t & obj);
        public:
            const int           get_type()         const {return m_event_type;}
            const int           get_priority()     const {return m_event_priority;}
            const int           get_error_code()   const {return m_error_code;}
            const std::string&  get_result_data()  const {return m_result_data;}
            const xvip2_t&      get_from_xip()     const {return m_from_xip;}
            const xvip2_t&      get_to_xip()       const {return m_to_xip;}
            const uint64_t      get_cookie()       const {return m_event_cookie;}
            const uint64_t      get_clock()        const {return m_event_clock;}
            
            void                set_from_xip(const xvip2_t & from) {m_from_xip = from;}
            void                set_to_xip(const xvip2_t & to)      {m_to_xip = to;}
            void                set_cookie(const uint64_t cookie){m_event_cookie = cookie;}
            void                set_clock(const uint64_t clock){m_event_clock = clock;}
            
            const int           get_route_path() const; //default is enum_xevent_route_path_down
            void                set_route_path(enum_xevent_route_path path);//mark what path of event will route
            void                remove_route_path(enum_xevent_route_path path); //remove it from set of paths
            
        private:
            xvip2_t             m_from_xip;    //from address
            xvip2_t             m_to_xip;      //target address ,-1 means to broadcast everyone,and 0 means anyone may handle,
            uint64_t            m_event_clock;  //application set latest clock(it might global clock time or height)
            //note: to compatible with mbus, we put mbus::event_ptr into cookie when type == enum_xevent_core_type_mbus
            uint64_t            m_event_cookie; //application may set cookie let event carry
            int16_t             m_event_type;   //init as 0 that is invalid
            uint8_t             m_event_flags;  //flags for event,reserved now
            uint8_t             m_event_paths;  //refer enum_xevent_route_path
        protected:
            int16_t             m_event_priority;//priority level for event
            int16_t             m_error_code;  //default it is 0 = successful
            std::string         m_result_data; //default it is empty
        };
    } //end of namespace of base

    namespace mbus //move definition of xevent_t from mbus into xbase module
    {
        //eventually we should discard xevent completely, and replace all by base::xvevent_t
        class xevent_t : public base::xvevent_t
        {
        public:
            enum error_type
            {
                succ,
                fail
            };
            
            enum direction_type
            {
                to_listener,
                to_sourcer
            };
            
            xevent_t(int _major_type,
                     int _minor_type = 0,
                     direction_type dir = to_listener,
                     bool _sync = true);
       
            virtual ~xevent_t();
            
        public:
            xevent_t();
            xevent_t(xevent_t &&);
            xevent_t(const xevent_t &);
            xevent_t & operator = (const xevent_t &);
            
        public: //not good,but keep it as old code(mbus module) used
            int         major_type;
            int         minor_type;
            error_type  err;
            direction_type direction;
            int64_t     m_time;
            bool        sync;
        };
    
    } //end of namespace of mbus
} //end of namespace of top


 
