//
//  xcorebridge.cpp
//  iostestclient
//
//  Created by Taylor Wei on 7/12/15.
//  Copyright (c) 2015 Taylor Wei. All rights reserved.
//

#include "xcorebridge.h"
#include "Jubase.h"
#include "Juutility.h"
#include "Juconnect.h"

using namespace Jeesu;
bool thread_run(void* args,bool & bStopSignal);

Juthread_t * g_thread = 0;
Jucontext_t * g_context = 0;
bool quit_process = false;

std::string  GetDocumentHomeFolder()
{
    NSString *nsDir = [xcorebridge getDocumentDictionary];
    if(nsDir != nil)
    {
        return std::string( [nsDir UTF8String]);
    }
    return std::string();
}

class myclientconnect : public Juioevent_t
{
public:
    myclientconnect()
    {
        m_bconnected = 0;
        m_connected_times = 0;
        m_raw_connection = NULL;
        
        m_total_packets_recv = 0;
    }
    ~myclientconnect()
    {
        if(m_raw_connection != NULL)
            m_raw_connection->release_ref();
    }
public:
    
    virtual int32_t          ioevent_add_ref()
    {
        return 1;
    }
    virtual int32_t          ioevent_release_ref()
    {
        return 1;
    }
    
    virtual void*            ioevent_query_interface(Juobject_t::enum_object_type type)
    {
        return NULL;
    }
    
    bool tcp_connect(std::string ip, int32_t port)
    {
        const std::string uri_string = "tcp://" + ip + ":" + string_utl::Int32ToString(port);
        m_connected_uri = uri_string;
        if(m_raw_connection == NULL)
        {
            Juiothread_t * io_thread = g_context->find_thread(enum_thread_type_io, false);
            m_raw_connection = new Juclientconnect_t(g_context,io_thread->get_thread_id(),this,enum_network_id_pn2,enum_network_type_user);
        }
        return (m_raw_connection->connect(uri_string) != 0);
    }
    
    bool close()
    {
        if(m_raw_connection != NULL)
            m_raw_connection->close(false);
        
        return true;
    }
public:
    bool    is_connected() { return (m_bconnected != 0);}
    int32_t  send_echo_packet()
    {
        std::string echo_string = "welcome";
        //sendout echo packet
        xip2basehead header;
        header.protocol = 0;
        header.flags = 0;
        header.total_len = sizeof(xip2basehead) + sizeof(uint16_t) + echo_string.size();
        
        Jupacket_t echo_pakcet;
        echo_pakcet << header.total_len;
        echo_pakcet << header.flags;
        echo_pakcet << header.protocol;
        echo_pakcet << echo_string;
        
        echo_pakcet.set_transfer_flag(enum_packet_transfer_flag_write_confirm);
        echo_pakcet.set_transfer_flag(enum_packet_transfer_flag_write_progress);
        echo_pakcet.set_time(time_utl::time_now_ms());
        echo_pakcet.set_to(0);
        if(m_raw_connection != NULL)
            return m_raw_connection->send(0,-1,echo_pakcet, 0, 0,NULL);
        
        return 0;
    }
public: //can only be called from host thread, errorcode refer enum_error_code ,return true when the event is handled
    //on_endpoint_connect is a virtual concept that could be socket connect/connetion connect/attached to router
    virtual bool             on_endpoint_connect(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,Juendpoint_t* from)
    {
        if(0 == error_code)
        {
            m_bconnected = 1;
            
            ++m_connected_times;
            if(m_connected_times < 3)
            {
                //m_raw_connection->connect(m_connected_uri);
            }
            
            send_echo_packet();
        }
        return true;
    }
    
    //on_endpoint_disconnect is a virtual concept that could be socket disconnect/connetion disconnect/detach from router
    virtual bool             on_endpoint_disconnect(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,Juendpoint_t* from)
    {
        quit_process = true;
        return true;
    }
    
    //when associated io-object close happen,post the event to receiver
    //error_code is 0 when it is closed by caller/upper layer
    virtual bool             on_endpoint_close(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,Juendpoint_t* from)
    {
        quit_process = true;
        return true;
    }
    
public: //can only be called from host thread, errorcode refer enum_error_code,return true when the event is handled
    //packet.size() > 0 when the packet is readed from socket successful
    virtual int32_t          on_packet_recv(uint64_t from_xip_addr,uint64_t to_xip_addr,Jupacket_t & packet, int32_t cur_thread_id, uint64_t timenow_ms,Juendpoint_t* from)
    {
        static int s_recv_trace_index = 0;
        ++s_recv_trace_index;
        
        ++m_total_packets_recv;
        
        std::string echo_string;
        xip2basehead header = {0};
        
        packet >> header.total_len;
        packet >> header.flags;
        packet >> header.protocol;
        packet >> echo_string;
        if(echo_string != "welcome")
        {
            ju_error("on_packet_recv,wrong packet(id=%lld,size=%d),total_len=%d,string=%s",packet.get_id(),packet.size(),header.total_len,echo_string.c_str());
        }
        else
        {
            if(s_recv_trace_index % 128 == 0)
                ju_dbg("on_packet_recv,correct packet(id=%lld,size=%d),total_len=%d,string=%s,from_xip_addr=%lld,to_xip_addr=%lld",packet.get_id(),packet.size(),header.total_len,echo_string.c_str(),from_xip_addr,to_xip_addr);
        }
        
        send_echo_packet();
        
        return enum_code_successful;
    }
    
    //packet.size() > 0  when the packet is not completely sent out, 0 means the whole packet has done
    //write fail when error_code != 0,error code refer enum_error_code
    //Note: dont hold packet object that will destroyed immediately after on_packet_write
    virtual bool             on_packet_send(const int32_t errorcode_or_left_size,Jupacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms,Juendpoint_t* from)
    {
        static int s_trace_index = 0;
        ++s_trace_index;
        if(errorcode_or_left_size == 0)
        {
            int32_t duration = (int32_t)(timenow_ms - packet.get_time());
            if(s_trace_index % 128 == 0)
                ju_dbg("on_packet_send,complete packet(id=%lld),cur_thread_id=%d,duration=%d",packet.get_id(),cur_thread_id,duration);
        }
        else
        {
            ju_dbg("on_packet_send,partionly packet(id=%lld,left size=%d),cur_thread_id=%d,timenow_ms=%lld",packet.get_id(),packet.size(),timenow_ms);
        }
        return true;
    }
    
    //confirm this packet is already delivered to receiver through ack
    //write fail when error_code != 0,error code refer enum_error_code
    virtual bool             on_packet_deliver(const int32_t error_code,Jupacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms,const int32_t duration_ms,Juendpoint_t* from)
    {
        return true;
    }
    
    //packet.size() > 0 when the packet is recalled successful,otherwise the packet is already removed out from application buffer
    virtual bool             on_packet_recall(const int32_t error_code,Jupacket_t & packet,const int32_t cur_thread_id,const uint64_t timenow_ms,Juendpoint_t* from)
    {
        return true;
    }
protected:
    bool                m_bconnected;
    int32_t             m_connected_times;
    int64_t             m_total_packets_recv;
    std::string         m_connected_uri;
    Juclientconnect_t * m_raw_connection;
};

@implementation xcorebridge
+(NSString*) getDocumentDictionary
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    if( (paths != nil) && ([paths count] > 0) )
    {
        NSString * dict = [paths objectAtIndex:0];
        return dict;
    }
    
    return nil;
}

+(void) start
{    
    dispatch_async(dispatch_get_main_queue(), ^(){
        std::string std_log_file = GetDocumentHomeFolder() + "//xcore-client";
        const char* log_file =  std_log_file.c_str();
        
#ifdef __LOCAL_TEST__
        ju_init_log(enum_log_level_debug, log_file, true);
#else
        ju_init_log(enum_log_level_debug, log_file, true);
#endif
        
        ju_kinfo("start demo");
        if(g_thread == NULL)
        {
            g_thread = Juthread_t::create_thread(thread_run, 0);
            g_thread->start();
        }
    });
}

+(void) start1
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        std::string std_log_file = GetDocumentHomeFolder() + "//xcore-client";
        const char* log_file =  std_log_file.c_str();
        
#ifdef __LOCAL_TEST__
        ju_init_log(enum_log_level_debug, log_file, true);
#else
        ju_init_log(enum_log_level_debug, log_file, true);
#endif
        
        bool bStopSignal = false;
        thread_run(NULL,bStopSignal);
    });
}
@end


bool thread_run(void* args,bool & bStopSignal)
{
    ju_kinfo("start thread_run");

    g_context = Jucontext_t::instance();
    g_context->start(1);
    
    const int connect_to_port = 8888;
    const std::string connect_to_ip("54.241.2.194");
    //const std::string connect_to_ip("10.0.0.6");
    myclientconnect connector;
    connector.tcp_connect(connect_to_ip,connect_to_port);
    
    while(!quit_process)
    {
        sleep(1);
        
        if(connector.is_connected())
            connector.send_echo_packet();
    }
    
    return true;
}

/*
bool thread_run(void* args,bool & bStopSignal)
{
    ju_kinfo("start thread_run");
    
    g_context = Jucontext_t::create_instance(0,0,0);
    g_context->start(1);
    
    const int32_t thread_id = g_context->get_cur_thread_id();
    
    Juconnection_t * g_client_socket = new Juconnection_t(g_context,thread_id);
    
    //#ifdef __LOCAL_TEST__
    //const uint64_t connect_id = g_client_socket->connect(std::string("tcp://127.0.0.1:5002"),0);
    //#else
    const uint64_t connect_id = g_client_socket->connect(std::string("tcp://42.120.18.120:5002"),0);
    //#endif
    
    uint32_t packet_seq_id = 0;
    
    char sztitle[128] = {0};
    sztitle[120] = 'w';
    sztitle[121] = 'e';
    sztitle[122] = 'l';
    sztitle[123] = 'c';
    sztitle[124] = 'o';
    sztitle[125] = 'm';
    sztitle[126] = 'e';
    
    Jupacket_t request;
    xip2Header header;
    header.version = 5;
    header.protocol = 0;
    header.length =  sizeof(xip2Header) + 128 + sizeof(uint32_t) + sizeof(uint64_t);
    request.push_back((uint8_t*)&header, sizeof(header));
    request.push_back((uint8_t*)&packet_seq_id, sizeof(packet_seq_id));
    const uint64_t time_stamp = time_utl::time_now_ms();
    request.push_back((uint8_t*)&time_stamp, sizeof(time_stamp));
    request.push_back((uint8_t*)sztitle, 128);
    
    g_client_socket->send_packet(request, Juendpoint_t::enum_endpoint_flags_flush_send | Juendpoint_t::enum_endpoint_flags_ack_self, thread_id);
    
    bool quit_process = false;
    while(!quit_process)
    {
        Jupacket_t packet;
        int32_t flags = Juendpoint_t::enum_endpoint_flags_block_recv;
        if(g_client_socket->recv_packet(packet,flags,thread_id) == enum_code_successful)
        {
            const uint64_t time_now = time_utl::time_now_ms();
            const uint32_t packet_id = *(uint32_t*)(packet.data() + sizeof(xip2Header));
            const uint64_t packet_time_stamp = *(uint64_t*)(packet.data() + sizeof(xip2Header) + sizeof(uint32_t));
            const int32_t  delay_duration = (int32_t)(time_now - packet_time_stamp);
            
            if( (packet_seq_id % 32) == 0)
                ju_dbg("Juclient_socket::get packet(%d),delay(%d),content(%d)",packet_id,delay_duration,packet.size());
            
            //#ifdef __LOCAL_TEST__
            //sleep(1);
            //#endif
            Jupacket_t respond;
            header.version = 5;
            header.protocol = 0;
            header.length =  sizeof(xip2Header) + 128 + sizeof(packet_seq_id) + sizeof(uint64_t);
            respond.push_back((uint8_t*)&header, sizeof(header));
            
            ++packet_seq_id;
            respond.push_back((uint8_t*)&packet_seq_id, sizeof(packet_seq_id));
            const uint64_t time_stamp = time_now;
            respond.push_back((uint8_t*)&time_stamp, sizeof(time_stamp));
            respond.push_back((uint8_t*)sztitle, 128);
            
            respond.set_to(packet.get_from());
            respond.set_from(0);
            g_client_socket->send_packet(respond,Juendpoint_t::enum_endpoint_flags_flush_send,thread_id);
        }
    }
    
    g_client_socket->disconnect(connect_id);
    g_client_socket->release_ref();
    g_context->close();
    
    return true;
}
*/