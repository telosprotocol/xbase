// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "xlog.h"
#include "xobject.h"
#include "xthread.h"
#include "xtimer.h"
#include "xdata.h"
#include "xpacket.h"
#include "xsocket.h"
#include "xutl.h"
#include "xpipe.h"

using namespace top;
using namespace base;
xfd_handle_t udp_handle_1 = invalid_handle_t;
xfd_handle_t udp_handle_2 = invalid_handle_t;

class udpdemo_t : public base::udp_t
{
public:
    udpdemo_t(base::xcontext_t & _context,const int32_t target_thread_id,xfd_handle_t native_handle)
    : base::udp_t(_context,NULL,target_thread_id,native_handle)
    {
        latest_packet_id = 0;
    }
    virtual ~udpdemo_t()
    {
        printf("udpdemo_t quit for socket handle(%d) for this(%lld) \n",get_handle(),(uint64_t)this);
    };
private:
    udpdemo_t();
    udpdemo_t(const udpdemo_t &);
    udpdemo_t & operator = (const udpdemo_t &);
public:
    virtual int32_t          recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override
    {
        if(packet.get_size() > 0)
        {
            std::string content((const char *)packet.get_body().data(),(int)packet.get_body().size());
            printf("udpdemo_t recv packet,content [%s] at thread(%d) from address(%s : %d),to address(%s : %d) \n",content.c_str(),cur_thread_id,packet.get_from_ip_addr().c_str(),packet.get_from_ip_port(),packet.get_to_ip_addr().c_str(), packet.get_to_ip_port());
        }
        ++latest_packet_id;
        return 0;
    }
    int  latest_packet_id;
};

int test_udp(bool is_stress_test)
{

    
    printf("------------------------[test_udp] start -----------------------------  \n");
    
    const std::string local_listen_addr_v4 = "127.0.0.1";
    const std::string local_listen_addr_v6 = "::1";
    const std::string local_listen_addr = local_listen_addr_v4;
    
    top::base::xiothread_t * t1 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);
    top::base::xiothread_t * t2 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);

	top::base::xtime_utl::sleep_ms(1000);

    //test udp
    {
        uint16_t     udp_listen_port_1 = 0;
        udp_handle_1 = base::xsocket_utl::udp_listen(local_listen_addr,udp_listen_port_1);
        base::udp_t* udp_socket_1 = new udpdemo_t(base::xcontext_t::instance(),t1->get_thread_id(),udp_handle_1);
        udp_socket_1->start_read(0);
        
        uint16_t     udp_listen_port_2 = 0;
        udp_handle_2 = base::xsocket_utl::udp_listen(local_listen_addr,udp_listen_port_2);
        base::udp_t* udp_socket_2 = new udpdemo_t(base::xcontext_t::instance(),t2->get_thread_id(),udp_handle_2);
        udp_socket_2->start_read(0);
        
        const std::string local_ip = udp_socket_2->get_local_ip_address();
        
        for(int i = 0; i < 10; ++i)
        {
            std::string test_udp_header("udp packet id: ");
            std::string test_udp_body = base::xstring_utl::tostring(i);
            
            top::base::xpacket_t packet;
            packet.get_header().push_back((uint8_t*)test_udp_header.data(), (int)test_udp_header.size());
            packet.get_body().push_back((uint8_t*)test_udp_body.data(), (int)test_udp_body.size());
            
            if(i % 2 == 0)
            {
                packet.set_to_ip_addr(std::string(local_listen_addr));
                packet.set_to_ip_port(udp_listen_port_2);
                udp_socket_1->send(0, 0,0,0,packet, 0, 0, NULL);
            }
            else
            {
                packet.set_to_ip_addr(std::string(local_listen_addr));
                packet.set_to_ip_port(udp_listen_port_1);
                //if(udp_handle_2 != invalid_handle_t)
                    udp_socket_2->send(0, 0,0,0, packet, 0, 0, NULL);
            }
			top::base::xtime_utl::sleep_ms(100);
        }
        
		top::base::xtime_utl::sleep_ms(1000);
        
        udp_socket_1->close(true);
        udp_socket_1->release_ref();
        
        udp_socket_2->close(true);
        udp_socket_2->release_ref();
    }
    
    int udp_listen_port_start = 8000;
    if(udp_listen_port_start != 0)
    {
        for(int i = 0; i < 1; ++i)
        {
            uint16_t     udp_listen_port = udp_listen_port_start++;
            int udp_handle = base::xsocket_utl::udp_listen(local_listen_addr,udp_listen_port);
            base::udp_t* udp_socket = new udpdemo_t(base::xcontext_t::instance(),t1->get_thread_id(),udp_handle);
            udp_socket->start_read(0);
			top::base::xtime_utl::sleep_ms(1000);
            udp_socket->close(true);
			top::base::xtime_utl::sleep_ms(1000);
            udp_socket->release_ref();
        }
    }

    uint16_t init_port = 9123;
    int test_udp_handle = base::xsocket_utl::udp_listen(local_listen_addr,init_port);
    if(test_udp_handle > 0)
    {
        int err = base::xsocket_utl::close_socket(test_udp_handle);
        if(err == 1)
        {
            err = base::xsocket_utl::close_socket(test_udp_handle);
        }
    }
    
	top::base::xtime_utl::sleep_ms(1000);
    
    t1->close();
    t1->release_ref();
    t2->close();
    t2->release_ref();
    
    printf("/////////////////////////////// [test_udp] finish ///////////////////////////////  \n");
    return 0;
}
