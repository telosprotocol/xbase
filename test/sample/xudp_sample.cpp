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
#include "xbasepdu.h"
#include "xconnection.h"

using namespace top;
using namespace base;

class xp2pudp_t : public xudp_t
{
public:
    xp2pudp_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,int64_t virtual_handle,xsocket_property & property)
     : xudp_t(_context,parent,target_thread_id,virtual_handle,property)
    {
        printf("xp2pudp_t new,handle(%d),local address[%s : %d]-[%d : %d] vs  peer address[%s : %d]-[%d : %d]; cur_thread_id=%d,object_id(%lld) for this(%lld)\n",get_handle(), get_local_ip_address().c_str(),get_local_real_port(),get_local_logic_port(),get_local_logic_port_token(),get_peer_ip_address().c_str(),get_peer_real_port(),get_peer_logic_port(),get_peer_logic_port_token(),get_thread_id(),get_obj_id(),(uint64_t)this);
        printf("-------------------------------\n");
    }
    virtual ~xp2pudp_t()
    {
        printf("xp2pudp_t quit,handle(%d),local address[%s : %d]-[%d : %d] vs  peer address[%s : %d]-[%d : %d]; cur_thread_id=%d,object_id(%lld) for this(%lld)\n",get_handle(), get_local_ip_address().c_str(),get_local_real_port(),get_local_logic_port(),get_local_logic_port_token(),get_peer_ip_address().c_str(),get_peer_real_port(),get_peer_logic_port(),get_peer_logic_port_token(),get_thread_id(),get_obj_id(),(uint64_t)this);
        printf("-------------------------------\n");
    };
private:
    xp2pudp_t();
    xp2pudp_t(const xp2pudp_t &);
    xp2pudp_t & operator = (const xp2pudp_t &);
protected:
    //notify the child endpont is ready to use when receive on_endpoint_open of error_code = enum_xcode_successful
    virtual bool             on_endpoint_open(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child) override
    {
        const int result = xudp_t::on_endpoint_open(error_code,cur_thread_id,timenow_ms,from_child);
        //add code here
        printf("-------------------------------\n");
        printf("xp2pudp_t connected successful,handle(%d),local address[%s : %d]-[%d : %d] vs  peer address[%s : %d]-[%d : %d]; cur_thread_id=%d,object_id(%lld) for this(%lld)\n",get_handle(), get_local_ip_address().c_str(),get_local_real_port(),get_local_logic_port(),get_local_logic_port_token(),get_peer_ip_address().c_str(),get_peer_real_port(),get_peer_logic_port(),get_peer_logic_port_token(),get_thread_id(),get_obj_id(),(uint64_t)this);
        return result;
    }
    //when associated io-object close happen,post the event to receiver
    //error_code is 0 when it is closed by caller/upper layer
    virtual bool             on_endpoint_close(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child) override
    {
        printf("xp2pudp_t close successful,cur_thread_id=%d,object_id(%lld) for this(%lld) \n",cur_thread_id,get_obj_id(),(uint64_t)this);
        return xudp_t::on_endpoint_close(error_code,cur_thread_id,timenow_ms,from_child);
    }
    
    //notify upper layer,child endpoint update keeplive status
    virtual bool             on_endpoint_keepalive(const std::string & _payload,const int32_t cur_thread_id,const uint64_t timenow_ms,xendpoint_t* from_child) override
    {
        //note: _payload is from send_keepalive_packet();
        printf(" xp2pudp_t<--keepalive,handle(%d),local[%s : %d]-[%d : %d] vs peer[%s : %d]-[%d : %d]; thread_id=%d,object_id(%lld)\n",get_handle(), get_local_ip_address().c_str(),get_local_real_port(),get_local_logic_port(),get_local_logic_port_token(),get_peer_ip_address().c_str(),get_peer_real_port(),get_peer_logic_port(),get_peer_logic_port_token(),get_thread_id(),get_obj_id());

        return xudp_t::on_endpoint_keepalive(_payload,cur_thread_id,timenow_ms,from_child);
    }
protected:
    virtual int32_t     on_ping_packet_recv(xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from) override
    {
        //p2p dedicated packet,let subclass handle it
        xlink_ping_pdu  _pdu(*get_context(),0);//version 0
        if(_pdu.serialize_from(packet) > 0)
        {
            xlinkhead_t & _header = _pdu.get_header();
            const uint32_t from_logic_port = _header.get_from_logic_port();
            const uint32_t from_logic_port_token = _header.get_from_logic_port_token();
            const uint32_t target_logic_port = _header.get_to_logic_port();
            const uint32_t target_logic_port_token = _header.get_to_logic_port_token();
            //const std::string from_ip_port_address = packet.get_from_ip_addr() + ":" + xstring_utl::tostring(packet.get_from_ip_port());
            
            printf("xp2pudp_t      ::on_ping_packet_recv,cur_thread_id=%d,ping packet from ([%s : %d]-[%d : %d]) to ([%s : %d]-[%d : %d] ,_payload=%s,TTL =%d \n",cur_thread_id,_pdu._from_ip_address.c_str(),_pdu._from_ip_port,from_logic_port,from_logic_port_token,_pdu._to_ip_address.c_str(),_pdu._to_ip_port,target_logic_port,target_logic_port_token, _pdu._pdu_payload.c_str(),_pdu._pdu_fire_TTL);
            
            std::vector<std::string> peer_infos;
            if(4 == xstring_utl::split_string(_pdu._pdu_payload, '-', peer_infos))
            {
                m_peer_ip_addr = peer_infos[0];
                m_peer_real_port = xstring_utl::toint32(peer_infos[1]);
                
                m_peer_logic_port = xstring_utl::toint32(peer_infos[2]);
                m_peer_logic_port_token = xstring_utl::toint32(peer_infos[3]);
                
                connect(m_peer_ip_addr,m_peer_real_port);
            }
        }
        return enum_xcode_successful;
    }
    
    //receive data packet that already trim header of link layer
    virtual int32_t          recv(uint64_t from_xip_addr_low,uint64_t from_xip_addr_high,uint64_t to_xip_addr_low,uint64_t to_xip_addr_high,xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from_child_end) override
    {
        if(packet.get_packet_flags() & enum_xpacket_deliver_ack_flag) //ask deliver ack packet
        {
            const std::string payload_string((const char*)packet.get_body().data(),packet.get_body().size());
            printf("xp2pudp_t      ::recv data packet,cur_thread_id=%d,data packet from ([%s : %d]) to ([%s : %d]), payload=%s \n",cur_thread_id,packet.get_from_ip_addr().c_str(),packet.get_from_ip_port(),packet.get_to_ip_addr().c_str(),packet.get_to_ip_port(),(const char*)payload_string.c_str());
            
            packet.reset_ip_address();//when reuse the packet of recving, it must call reset_ip_address() to avoid send packet to self again
            
            //demo only
            //send(to_xip_addr_low,to_xip_addr_high,from_xip_addr_low,from_xip_addr_high,packet, cur_thread_id, timenow_ms, from_child_end);

        }
        else
        {
            printf("xp2pudp_t      ::recv data packet,cur_thread_id=%d,data packet from ([%s : %d]) to ([%s : %d])\n",cur_thread_id,packet.get_from_ip_addr().c_str(),packet.get_from_ip_port(),packet.get_to_ip_addr().c_str(),packet.get_to_ip_port());
        }
        
        //go default handle,where may deliver packet to parent object
        return xudp_t::recv(from_xip_addr_low,from_xip_addr_high,to_xip_addr_low,to_xip_addr_high,packet,cur_thread_id,timenow_ms,from_child_end);
    }
};

//customized xudplistener to support P2P
class xp2pudplisten_t : public xudplisten_t
{
public:
    //native_handle must valid,and transfer the owner of native_handle to xudplisten_t who may close handle when destroy
    xp2pudplisten_t(xcontext_t & _context,xendpoint_t * parent,const int32_t target_thread_id,xfd_handle_t native_handle)
     : xudplisten_t(_context,parent,target_thread_id,native_handle)
    {
        printf("xp2pudplisten_t::new, handle(%d),local ip address[%s : %d]-[%d : %d]\n",get_handle(), get_local_ip_address().c_str(),get_local_real_port(),get_local_logic_port(),get_local_logic_port_token());
    }
protected:
    virtual ~xp2pudplisten_t(){}
private:
    xp2pudplisten_t();
    xp2pudplisten_t(const xp2pudplisten_t &);
    xp2pudplisten_t & operator = (const xp2pudplisten_t &);
    
protected:
    virtual xslsocket_t*       create_xslsocket(xendpoint_t * parent,xfd_handle_t handle,xsocket_property & property, int32_t cur_thread_id,uint64_t timenow_ms) override
    {
        return new xp2pudp_t(*get_context(),parent,cur_thread_id,handle,property);
    }
 
protected: //p2p support,receive ping packet from peer
    virtual int32_t     on_ping_packet_recv(xpacket_t & packet,int32_t cur_thread_id,uint64_t timenow_ms,xendpoint_t* from) override
    {
        //p2p dedicated packet,let subclass handle it
        xlink_ping_pdu  _pdu(*get_context(),0);//version 0
        if(_pdu.serialize_from(packet) > 0)
        {
            xlinkhead_t & _header = _pdu.get_header();
            const uint32_t from_logic_port = _header.get_from_logic_port();
            const uint32_t from_logic_port_token = _header.get_from_logic_port_token();
            const uint32_t target_logic_port = _header.get_to_logic_port();
            const uint32_t target_logic_port_token = _header.get_to_logic_port_token();
            std::string from_ip_port_address = packet.get_from_ip_addr() + "-" + xstring_utl::tostring(packet.get_from_ip_port());
            //const std::string to_ip_port_address   = packet.get_to_ip_addr() + ":" + xstring_utl::tostring(packet.get_to_ip_port());
            printf("xp2pudplisten_t::on_ping_packet_recv,cur_thread_id=%d,ping packet from ([%s : %d]-[%d : %d]) to ([%s : %d]-[%d : %d] ,_payload=%s,TTL =%d \n",cur_thread_id,_pdu._from_ip_address.c_str(),_pdu._from_ip_port,from_logic_port,from_logic_port_token,_pdu._to_ip_address.c_str(),_pdu._to_ip_port,target_logic_port,target_logic_port_token, _pdu._pdu_payload.c_str(),_pdu._pdu_fire_TTL);
            
            connect_peer[from_ip_port_address] = _pdu._pdu_payload;
            if(connect_peer.size() == 2)
            {
                //exchange information
                for(std::map<std::string,std::string>::iterator it = connect_peer.begin(); it != connect_peer.end(); ++it)
                {
                    std::string peer_ip_addr = it->first;
                    std::string peer_logic_addr = it->second;
                    if(peer_ip_addr != from_ip_port_address)
                    {
                        peer_ip_addr += "-";
                        peer_ip_addr += peer_logic_addr;
                        send_ping(packet.get_from_ip_addr(), packet.get_from_ip_port(), peer_ip_addr, 1,0,from_logic_port, from_logic_port_token);
                    }
                }
            }
        }
        return enum_xcode_successful;
    }
protected:
    std::map<std::string,std::string> connect_peer;
};

int test_xudp(bool is_stress_test)
{
    uint8_t test = 0;
    std::uint8_t type1 = test;
    
    printf("------------------------[test_xudp] start -----------------------------  \n");
 
    bool _support_recvmmsg = xsocket_utl::is_support_recvmmsg(); //default as false
    printf("system kernel(%s) and _support_recvmmsg(%d) \n",xsys_utl::kernel_version().c_str(),_support_recvmmsg);
    
    std::string test_raw_data = "random data: ";
    const uint32_t random_len1 = top::base::xtime_utl::get_fast_randomu() % 4192;
    for(int j = 0; j < random_len1; ++j) //avg 2048 bytes per packet
    {
        int8_t random_seed1 = (int8_t)(top::base::xtime_utl::get_fast_randomu());
        if(random_seed1 < 0)
            random_seed1 = -random_seed1;
        if(random_seed1 <= 33)
            random_seed1 += 33;
        
        //test_raw_data.push_back('a');
        test_raw_data.push_back(random_seed1);
    }
    printf("random string_length = %d \n",(int)test_raw_data.size());
    //printf("random string = %s \n",test_raw_data.c_str());
    
    
 
    const std::string local_listen_addr_v4 = "127.0.0.1";
    const std::string local_listen_addr_v6 = "::1";
    const std::string local_listen_addr = local_listen_addr_v4;
    
    top::base::xiothread_t * t1 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);
    top::base::xiothread_t * t2 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);
    top::base::xiothread_t * t3 = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);

    top::base::xiothread_t * client_socket_io_thread = top::base::xiothread_t::create_thread(top::base::xcontext_t::instance(),0,-1);
    
    /////////////CLIENT ,SERVER create own xudp listenser
    xudplisten_t *   client_listener_ptr =  NULL;
    xudplisten_t *   server_listener_ptr =  NULL;
    xudplisten_t *   ping_listener_ptr   =  NULL;
    {
        uint16_t     udp_listen_port_1 = 0;
        xfd_handle_t udp_handle_1 = base::xsocket_utl::udp_listen(local_listen_addr,udp_listen_port_1);
        uint16_t     udp_listen_port_2 = 0;
        xfd_handle_t udp_handle_2 = base::xsocket_utl::udp_listen(local_listen_addr,udp_listen_port_2);
        uint16_t     udp_listen_port_3 = 0;
        xfd_handle_t udp_handle_3 = base::xsocket_utl::udp_listen(local_listen_addr,udp_listen_port_3);
        
        xsocket_utl::set_recv_buffer(udp_handle_1, 1048576);
        xsocket_utl::set_recv_buffer(udp_handle_2, 1048576);
        xsocket_utl::set_recv_buffer(udp_handle_3, 1048576);
        
        client_listener_ptr = new xp2pudplisten_t(top::base::xcontext_t::instance(),NULL,t1->get_thread_id(),udp_handle_1);
        client_listener_ptr->start_read(0);
        
        server_listener_ptr = new xp2pudplisten_t(top::base::xcontext_t::instance(),NULL,t2->get_thread_id(),udp_handle_2);
        server_listener_ptr->start_read(0);
        
        ping_listener_ptr = new xp2pudplisten_t(top::base::xcontext_t::instance(),NULL,t3->get_thread_id(),udp_handle_3);
        ping_listener_ptr->start_read(0);
    }

    
    xp2pudp_t * peer_xudp_socket_1 = NULL;
    xp2pudp_t * peer_xudp_socket_2 = NULL;
    //CLIENT create socket and connection to server where accept AND new socket
    if(1)
    {
        printf("////////////////////////[test_xudp] ,case#1: client:connect -> server:accept ///////////////////////////// \n");
        printf("////////////////////////                                                     ///////////////////////////// \n");
        
        peer_xudp_socket_1 = (xp2pudp_t*)client_listener_ptr->create_xslsocket(enum_socket_type_xudp,client_socket_io_thread->get_thread_id());
        //peer_xudp_socket_1 = (xp2pudp_t*)client_listener_ptr->create_xslsocket(enum_socket_type_xudp,top::base::xcontext_t::instance().get_current_thread_id());
        peer_xudp_socket_1->connect(server_listener_ptr->get_local_ip_address(),server_listener_ptr->get_local_real_port());
       
        top::base::xtime_utl::sleep_ms(2000); //leave 2 second to connect  then close
        peer_xudp_socket_1->close();
        
        top::base::xcontext_t::instance().delay_release_object(peer_xudp_socket_1);
        peer_xudp_socket_1 = NULL;
    }
    
    //CLIENT and SERVER create own socket first, then CLIENT initialize the connection
    if(1)
    {
        printf("////////////////////////[test_xudp] ,case#2: client:connect -> peer:accept ///////////////////////////// \n");
        printf("////////////////////////                                                   ///////////////////////////// \n");
        peer_xudp_socket_1 = (xp2pudp_t*)client_listener_ptr->create_xslsocket(enum_socket_type_xudp,client_socket_io_thread->get_thread_id());
        peer_xudp_socket_2 = (xp2pudp_t*)server_listener_ptr->create_xslsocket(enum_socket_type_xudp,client_socket_io_thread->get_thread_id());
        
        //directly conntect to peer socket
        peer_xudp_socket_1->connect(peer_xudp_socket_2->get_local_ip_address(),peer_xudp_socket_2->get_local_real_port(),peer_xudp_socket_2->get_local_logic_port(),peer_xudp_socket_2->get_local_logic_port_token());

        top::base::xtime_utl::sleep_ms(3000); //leave 2 second to connect then close
        
        peer_xudp_socket_1->close();
        peer_xudp_socket_2->close();
        
        top::base::xcontext_t::instance().delay_release_object(peer_xudp_socket_1);
        top::base::xcontext_t::instance().delay_release_object(peer_xudp_socket_2);

        peer_xudp_socket_1 = NULL;
        peer_xudp_socket_2 = NULL;
    }
    
    
    //CLIENT and SERVER create own socket first, then both of CLIENT and SERVER initialize the connection
    if(1)
    {
        printf("////////////////////////[test_xudp] ,case#3: client:connect <-> peer:connect ///////////////////////////// \n");
        printf("////////////////////////                                                     ///////////////////////////// \n");
        
        peer_xudp_socket_1 = (xp2pudp_t*)client_listener_ptr->create_xslsocket(enum_socket_type_xudp,client_socket_io_thread->get_thread_id());
        peer_xudp_socket_2 = (xp2pudp_t*)server_listener_ptr->create_xslsocket(enum_socket_type_xudp,client_socket_io_thread->get_thread_id());
        
        //directly conntect to peer socket 2
        peer_xudp_socket_1->connect(peer_xudp_socket_2->get_local_ip_address(),peer_xudp_socket_2->get_local_real_port(),peer_xudp_socket_2->get_local_logic_port(),peer_xudp_socket_2->get_local_logic_port_token());
        
        //directly conntect to peer socket 1,support double-connect
        peer_xudp_socket_2->connect(peer_xudp_socket_1->get_local_ip_address(),peer_xudp_socket_1->get_local_real_port(),peer_xudp_socket_1->get_local_logic_port(),peer_xudp_socket_1->get_local_logic_port_token());
        
        top::base::xtime_utl::sleep_ms(2000); //leave 2 second to connect  then close
        
        peer_xudp_socket_1->close();
        peer_xudp_socket_2->close();
        top::base::xcontext_t::instance().delay_release_object(peer_xudp_socket_1);
        top::base::xcontext_t::instance().delay_release_object(peer_xudp_socket_2);
        peer_xudp_socket_1 = NULL;
        peer_xudp_socket_2 = NULL;
    }
    
    //CLIENT and SERVER create own socket first,then exchange information by ping
    if(1)
    {
        printf("////////////////////////[test_xudp] ,case#4: client:connect <--> ping <--> peer:connect ///////////////////////////// \n");
        printf("////////////////////////                                                                ///////////////////////////// \n");
        peer_xudp_socket_1 = (xp2pudp_t*)client_listener_ptr->create_xslsocket(enum_socket_type_xudp,client_socket_io_thread->get_thread_id());
        peer_xudp_socket_2 = (xp2pudp_t*)server_listener_ptr->create_xslsocket(enum_socket_type_xudp,client_socket_io_thread->get_thread_id());
        
        //ping socket 1 to ping server
        const std::string _ping_payload1 = xstring_utl::tostring(peer_xudp_socket_1->get_local_logic_port()) + "-" + xstring_utl::tostring(peer_xudp_socket_1->get_local_logic_port_token());
        peer_xudp_socket_1->send_ping(ping_listener_ptr->get_local_ip_address(),ping_listener_ptr->get_local_real_port(), _ping_payload1, 1);
        
        //ping socket 2 to ping server
        const std::string _ping_payload2 = xstring_utl::tostring(peer_xudp_socket_2->get_local_logic_port()) + "-" + xstring_utl::tostring(peer_xudp_socket_2->get_local_logic_port_token());
        peer_xudp_socket_2->send_ping(ping_listener_ptr->get_local_ip_address(),ping_listener_ptr->get_local_real_port(), _ping_payload2, 1);
        
        top::base::xtime_utl::sleep_ms(3000); //leave 3 second to connect
        
        //send data before connected
        for(int i = 0; i < 10; ++i) //send 21 packet in advance
        {
            //const std::string test_pre_send_raw_data = test_raw_data + ": presend data before connection with presend-id: " + xstring_utl::tostring(i + 1);
            
            const std::string test_pre_send_raw_data = test_raw_data + " : " + xstring_utl::tostring(i + 1);
            xpacket_t test_packet(top::base::xcontext_t::instance()); //assume now it is connected
            test_packet.get_body().push_back((uint8_t*)test_pre_send_raw_data.data(), (int)test_pre_send_raw_data.size());
            test_packet.set_process_flag(enum_xpacket_process_flag_compress); //ask compress
            //test_packet.set_process_flag(enum_xpacket_process_flag_encrypt);  //ask compress
            test_packet.set_process_flag(enum_xpacket_process_flag_checksum); //ask checksum
            test_packet.set_packet_flag(enum_xpacket_deliver_ack_flag);
            test_packet.set_MTUL(2);  //set MTU as 1 * 256
            
            
            if(get_xpacket_reliable_type(test_packet.get_packet_flags()) >= enum_xpacket_reliable_type_most )
            {
                peer_xudp_socket_1->send(0, 0, 0, 0, test_packet, 0, 0, NULL);
            }
        }
        

        
        top::base::xtime_utl::sleep_ms(5000); //leave 3 second to connect
    }
    
    if(0)
    {
        printf("////////////////////////[test_xudp] ,case#5: xipconnection <--> xipconnection ///////////////////////////// \n");
        printf("////////////////////////                                                                ///////////////////////////// \n");
        peer_xudp_socket_1 = (xp2pudp_t*)client_listener_ptr->create_xslsocket(enum_socket_type_xudp,client_socket_io_thread->get_thread_id());
        peer_xudp_socket_2 = (xp2pudp_t*)server_listener_ptr->create_xslsocket(enum_socket_type_xudp,client_socket_io_thread->get_thread_id());
        
        xconnection_t * peer_connect_1 = new xconnection_t(top::base::xcontext_t::instance(),peer_xudp_socket_1->get_thread_id(),NULL,peer_xudp_socket_1);
        xconnection_t * peer_connect_2 = new xconnection_t(top::base::xcontext_t::instance(),peer_xudp_socket_2->get_thread_id(),NULL,peer_xudp_socket_2);
 
        //directly conntect to peer socket
        peer_xudp_socket_1->connect(peer_xudp_socket_2->get_local_ip_address(),peer_xudp_socket_2->get_local_real_port(),peer_xudp_socket_2->get_local_logic_port(),peer_xudp_socket_2->get_local_logic_port_token());
 
        xpacket_t test_packet1(top::base::xcontext_t::instance()); //assume now it is connected
        test_packet1.get_body().push_back((uint8_t*)test_raw_data.data(), (int)test_raw_data.size());
        test_packet1.set_process_flag(enum_xpacket_process_flag_compress); //ask compress
        test_packet1.set_process_flag(enum_xpacket_process_flag_encrypt);  //ask compress
        test_packet1.set_process_flag(enum_xpacket_process_flag_checksum); //ask checksum
        peer_connect_1->send(1, 1, 2, 2, test_packet1, 0, 0, NULL);

        xpacket_t test_packet2(top::base::xcontext_t::instance()); //assume now it is connected
        test_packet2.get_body().push_back((uint8_t*)test_raw_data.data(), (int)test_raw_data.size());
        test_packet2.set_process_flag(enum_xpacket_process_flag_compress); //ask compress
        test_packet2.set_process_flag(enum_xpacket_process_flag_encrypt);  //ask compress
        test_packet2.set_process_flag(enum_xpacket_process_flag_checksum); //ask checksum
        peer_connect_2->send(2, 2, 1, 1, test_packet2, 0, 0, NULL);
    }
    
    top::base::xtime_utl::sleep_ms(30000);
    if(peer_xudp_socket_1 != NULL)
    {
        peer_xudp_socket_1->close();
        peer_xudp_socket_1->release_ref();
    }
    if(peer_xudp_socket_2 != NULL)
    {
        peer_xudp_socket_2->close();
        peer_xudp_socket_2->release_ref();
    }
    client_listener_ptr->stop_read(0);
    client_listener_ptr->close();
    server_listener_ptr->stop_read(0);
    server_listener_ptr->close();
    ping_listener_ptr->stop_read(0);
    ping_listener_ptr->close();
    
    top::base::xtime_utl::sleep_ms(1000);
    t1->close();
    t1->release_ref();
    t2->close();
    t2->release_ref();
    t3->close();
    t3->release_ref();
    
    client_listener_ptr->release_ref();
    server_listener_ptr->release_ref();
    ping_listener_ptr->release_ref();
    
    printf("////////////////////////[test_xudp] finish ///////////////////////////// \n");
    return 0;
  
}
