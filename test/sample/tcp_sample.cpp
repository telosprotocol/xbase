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

using namespace top;
using namespace base;

class server_tcp_t : public base::tcp_t
{
public:
    server_tcp_t(base::xcontext_t & _context,const int32_t target_thread_id,xfd_handle_t native_handle)
     : base::tcp_t(_context,NULL,target_thread_id,native_handle)
    {
        
    }
    virtual ~server_tcp_t()
    {
        printf("server_tcp_t quit for socket handle(%d) for this(%lld) \n",get_handle(),(uint64_t)this);
    };
private:
    server_tcp_t();
    server_tcp_t(const server_tcp_t &);
    server_tcp_t & operator = (const server_tcp_t &);
public:
    virtual int32_t          recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override
    {
        if(packet.get_size() > 0)
        {
            const std::string content((const char *)packet.get_body().data(),(int)packet.get_body().size());
            printf("server_tcp_t recv content [%s] at thread(%d) from address(%s : %d),to address(%s : %d) \n",content.c_str(), cur_thread_id,packet.get_from_ip_addr().c_str(),packet.get_from_ip_port(),packet.get_to_ip_addr().c_str(), packet.get_to_ip_port());
            
            base::xpacket_t packet;
            #ifdef __TEST_PACKET_HEADER_BODY__
            {
                std::string test_tcp_header("server_tcp_t ack packet from port-");
                std::string test_tcp_body = base::xstring_utl::tostring(get_local_real_port());
                
                packet.get_header().push_back((uint8_t*)test_tcp_header.data(), (int)test_tcp_header.size());
                packet.get_body().push_back((uint8_t*)test_tcp_body.data(), (int)test_tcp_body.size());
            }
            #else
            {
                std::string test_tcp_body("server_tcp_t ack packet from port-");
                test_tcp_body += base::xstring_utl::tostring(get_local_real_port());
                
                packet.get_body().push_back((uint8_t*)test_tcp_body.data(), (int)test_tcp_body.size());
            }
            #endif //__TEST_PACKET_HEADER_BODY__
            send(0,0,0,0,packet,0,0,NULL); //ack a test packet back to sender
        }
        return 0;
    }
};

class client_tcp_t : public base::tcp_connect_t
{
public:
    client_tcp_t(base::xcontext_t & _context,const int32_t target_thread_id,std::string ip,int port)
    : tcp_connect_t(_context,NULL,target_thread_id,ip,port,0)
    {
        
    }
    virtual ~client_tcp_t()
    {
        printf("client_tcp_t quit for socket handle(%d) for this(%lld) \n",get_handle(),(uint64_t)this);
    };
private:
    client_tcp_t();
    client_tcp_t(const client_tcp_t &);
    client_tcp_t & operator = (const client_tcp_t &);
public:
    virtual int32_t          recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override
    {
        if(packet.get_size() > 0)
        {
            const std::string content((const char *)packet.get_body().data(),(int)packet.get_body().size());
            printf("client_tcp_t recv content [%s] at thread(%d) from address(%s : %d),to address(%s : %d) \n",content.c_str(), cur_thread_id,packet.get_from_ip_addr().c_str(),packet.get_from_ip_port(),packet.get_to_ip_addr().c_str(), packet.get_to_ip_port());
        }
        return 0;
    }
};


bool       quit_test = false;
const  int max_threads_count = 2;
base::xiothread_t * threads_pool[max_threads_count] =  {0};
std::vector<server_tcp_t*> std_tcp_sockets_vector;

int test_tcp(bool is_stress_test)
{
    printf("------------------------[test_tcp] start -----------------------------  \n");
    
    const std::string local_listen_addr_v4 = "127.0.0.1";
    const std::string local_listen_addr_v6 = "::1";
    const std::string local_listen_addr = local_listen_addr_v6;
    
    for(int i = 0; i < max_threads_count; ++i)
    {
        //note: create_thread is a block operation if pass -1 as timeout
        threads_pool[i] = base::xiothread_t::create_thread(base::xcontext_t::instance(),base::xiothread_t::enum_xthread_type_io,-1);
    }
    
    uint16_t     tcp_listen_port = 0;
    xfd_handle_t tcp_lisetn_handle = base::xsocket_utl::tcp_listen(local_listen_addr,tcp_listen_port);
    auto lambda_listen = [&tcp_lisetn_handle](top::base::xcall_t & call,const int32_t thread_id, const uint64_t time_now_ms)->bool
    {
        base::xsocket_utl::set_nonblock(tcp_lisetn_handle, false); //change back to block mode for test purpose!
        while(quit_test == false)
        {
            sockaddr_storage target_sock_addr = {0};
            socklen_t        sock_addr_len = sizeof(sockaddr_in6);
            int accept_handle = ::accept(tcp_lisetn_handle, (sockaddr*)&target_sock_addr, &sock_addr_len);
            if(accept_handle > 0 ) //valid socket handle
            {
                base::xsocket_utl::set_tcp_nodelay(accept_handle, true);
                base::xsocket_utl::set_send_buffer(accept_handle, 819200);//800k for test purpose
                base::xsocket_utl::set_recv_buffer(accept_handle, 819200);//800k for test purpose
                //setup other configuration like tcp socket buffer
                
                base::xiothread_t * choosed_thread = threads_pool[accept_handle % max_threads_count];
                server_tcp_t * tcp_socket = new server_tcp_t(base::xcontext_t::instance(),choosed_thread->get_thread_id(),accept_handle);
                tcp_socket->start_read(0);
                
                std::string from_ip;
                uint16_t    from_port = 0;
                base::xsocket_utl::get_ipaddress_port((sockaddr*)&target_sock_addr, from_ip, from_port);
                printf("test_tcp accept a tcp handle(%d) from address[%s : %d],tcp running at thread(%d) \n",accept_handle,from_ip.c_str(),from_port,choosed_thread->get_thread_id());
                
                //manager tcp_socket 'lifecycle
                std_tcp_sockets_vector.push_back(tcp_socket);
            }
            else //quit
            {
                break;
            }
        }
        return true;
    };
    top::base::xcall_t  listen_service(lambda_listen);
    base::xiothread_t * listen_thread = base::xiothread_t::create_thread(base::xcontext_t::instance(),base::xiothread_t::enum_xthread_type_service,-1);
    listen_thread->send_call(listen_service);
    
    top::base::xtime_utl::sleep_ms(1000);

    //create a tcp client to simulate the connection at local
    base::xiothread_t * connect_thread = base::xiothread_t::create_thread(base::xcontext_t::instance(),base::xiothread_t::enum_xthread_type_io,-1);
    base::tcp_t * tcp_client_socket = new client_tcp_t(base::xcontext_t::instance(),connect_thread->get_thread_id(),local_listen_addr, tcp_listen_port);
    base::xsocket_utl::set_tcp_nodelay(tcp_client_socket->get_handle(), true);
    base::xsocket_utl::set_send_buffer(tcp_client_socket->get_handle(), 819200);//800k for test purpose
    base::xsocket_utl::set_recv_buffer(tcp_client_socket->get_handle(), 819200);//800k for test purpose
    tcp_client_socket->start_read(0);
    
    
    for(int i = 0; i < 10; ++i) //send 10 test packets
    {
		std::string test_content(" tcp client init packet,id= ");
        top::base::xpacket_t packet;
        //test_content += ",tcp client init packet,id=";
        test_content += base::xstring_utl::tostring(i);
		test_content += " \r\n";
        packet.get_body().push_back((uint8_t*)test_content.data(), (int)test_content.size());
        tcp_client_socket->send(0, 0,0,0, packet, 0, 0, NULL);
		top::base::xtime_utl::sleep_ms(100);
    }
    
	top::base::xtime_utl::sleep_ms(3000);
 
    //clean sockets
    for(std::vector<server_tcp_t*>::iterator it = std_tcp_sockets_vector.begin(); it != std_tcp_sockets_vector.end(); ++it)
    {
        server_tcp_t * sockets = *it;
        sockets->close();
        sockets->release_ref();
    }
   
	top::base::xtime_utl::sleep_ms(2000);
    
    //clean client tcp socket
    tcp_client_socket->close();
    tcp_client_socket->release_ref();
    
    base::xsocket_utl::close_socket(tcp_lisetn_handle);
    quit_test = true; //set quit flag
    
    
	top::base::xtime_utl::sleep_ms(2000); //wait 2 seconds to clean
    
    //cleans threads
    connect_thread->close(false);
    connect_thread->release_ref();
    listen_thread->close(false);
    listen_thread->release_ref();
    
    for(int i = 0; i < max_threads_count; ++i)
    {
        threads_pool[i]->close();
        threads_pool[i]->release_ref();
    }
    printf("/////////////////////////////// [test_tcp] finish /////////////////////////////// \n");
    return 0;
}
