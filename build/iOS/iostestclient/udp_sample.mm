//
//  udp_sample.cpp
//  xbase-test
//
//  Created by Taylor Wei on 12/16/18.
//  Copyright © 2018 Taylor Wei. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "xlog.h"
#include "xobject.h"
#include "xthread.h"
#include "xtimer.h"
#include "xdata.h"
#include "xpacket.h"
#include "xsocket.h"
#include "xutl.h"

using namespace top;
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
    virtual int32_t          recv(uint64_t from_xip_addr,uint64_t to_xip_addr,base::xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end)
    {
        if(packet.get_size() > 0)
        {
            std::string content((const char *)packet.get_body().data(),(int)packet.get_body().size());
            printf("udpdemo_t recv packet,content [%s] at thread(%d) from address(%s : %d),to address(%s : %d) \n",content.c_str(),cur_thread_id,packet.get_from_ip_addr().c_str(),packet.get_from_sys_port(),packet.get_to_ip_addr().c_str(), packet.get_to_sys_port());
        }
        ++latest_packet_id;
        return 0;
    }
    int  latest_packet_id;
};

int test_udp()
{
    printf("------------------------[test_udp] start -----------------------------  \n");
    
    const std::string local_listen_addr_v4 = "127.0.0.1";
    const std::string local_listen_addr_v6 = "::1";
    const std::string local_listen_addr = local_listen_addr_v6;
    
    top::base::xiothread_t * t1 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);
    top::base::xiothread_t * t2 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);

    //test udp
    {
        sleep(1);
        
        uint16_t     udp_listen_port_1 = 0;
        udp_handle_1 = base::xsocket_utl::udp_listen(local_listen_addr,udp_listen_port_1);
        base::udp_t* udp_socket_1 = new udpdemo_t(base::xcontext_t::instance(),t1->get_thread_id(),udp_handle_1);
        udp_socket_1->start_read(0);
        
        uint16_t     udp_listen_port_2 = 0;
        udp_handle_2 = base::xsocket_utl::udp_listen(local_listen_addr,udp_listen_port_2);
        base::udp_t* udp_socket_2 = new udpdemo_t(base::xcontext_t::instance(),t2->get_thread_id(),udp_handle_2);
        udp_socket_2->start_read(0);
        
        std::string test_value("abcdefghijklm1234567890nopqrstuvwxyz");
        for(int i = 0; i < 1024; ++i)
        {
            test_value += "-";
            test_value += base::xstring_utl::tostring(i);
        }
        for(int i = 0; i < 10000; ++i)
        {
            
            //std::string test_udp_header("udp packet id: ");
            //std::string test_udp_body = test_value + base::xstring_utl::tostring(i);
            std::string test_udp_header("udp packet id: ");
            std::string test_udp_body(188, 'u');
            
            top::base::xpacket_t packet;
            packet.get_header().push_back((uint8_t*)test_udp_header.data(), (int)test_udp_header.size());
            packet.get_body().push_back((uint8_t*)test_udp_body.data(), (int)test_udp_body.size());
            
            if(i % 2 == 0)
            {
                packet.set_to_ip_addr(std::string(local_listen_addr));
                packet.set_to_sys_port(udp_listen_port_2);
                //udp_socket_1->send(0, 0, packet, 0, 0, NULL);
            }
            else
            {
                packet.set_to_ip_addr(std::string(local_listen_addr));
                packet.set_to_sys_port(udp_listen_port_1);
                //if(udp_handle_2 != invalid_handle_t)
                //    udp_socket_2->send(0, 0, packet, 0, 0, NULL);
            }
            //if(i % 10 == 9)
                sleep(1);
        }
        
        sleep(1);
        
        udp_socket_1->close(true);
        udp_socket_1->release_ref();
        
        udp_socket_2->close(true);
        udp_socket_2->release_ref();
    }
    
    t1->close();
    t1->release_ref();
    t2->close();
    t2->release_ref();
    
    printf("/////////////////////////////// [test_udp] finish ///////////////////////////////  \n");
    return 0;
}
