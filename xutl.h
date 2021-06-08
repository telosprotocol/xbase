// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <vector>
#include <map>
#include <time.h>
#include "xbase.h"

#if defined(__WIN_PLATFORM__)
    #include <WinSock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>
	struct iovec
	{
		void*  iov_base; /*Base address of I/O Memory region*/
		size_t iov_len;  /*Size of region iov_Base points to*/
	};

#else
    #include <sys/types.h>
    #include <sys/socket.h>
	#include <sys/time.h>
    #include <netinet/in.h>
#endif

//mmsghdr is Linux-class only,but msghdrv is compliance with Posix Standard
#ifdef __LINUX_PLATFORM__
    #define xmmsghdr mmsghdr
#else
    struct  xmmsghdr {
        struct msghdr msg_hdr;  /* Message header */
        unsigned int  msg_len;  /* Number of received bytes for header */
    };
#endif

//utility function
namespace top
{
    namespace base
    {
        //cross-platform ' utility for socket related operation
        class xsocket_utl
        {
        public:
            static  int             get_last_error(); //get last error happend at current thread,wrap for windows & Linux & Unix
            static int              get_socket_error(xfd_handle_t socket_handle); //read SO_ERROR from SOL_SOCKET
            static int              get_socket_family(xfd_handle_t socket);//check and return AF_INET or AF_INET6 or AF_LOCAL etc
            
            static bool             is_ipv4_support(); //check whether network interface & OS support IPv4 stack
            static bool             is_ipv6_support(); //check whether network interface & OS support IPv6 stack
        public:
            //tell ipv4 or ipv6 by ip address
            static bool             is_ipv4_address(const char* ip_address);
            static bool             is_ipv4_address(const std::string & ip_address);
            static bool             is_ipv6_address(const char* ip_address);
            static bool             is_ipv6_address(const std::string & ip_address);
            
            //convert sockaddr to readable ip address
            static std::string      get_ipaddress(sockaddr * sock_addr);
            static bool             get_ipaddress_port(sockaddr * sock_addr,std::string & ip_address, uint16_t & port);
            //to gain performance,get_raw_address just copy raw address as integer format into raw_addr without inet_ntop convert,
            static bool             get_raw_address(sockaddr * net_addr,std::string & raw_addr,uint16_t & ip_port);
            static std::string      get_ipaddress_from_raw(const std::string & raw_addr);
            
            //get bind ip address associated with socket
            static sockaddr_storage get_bind_address(xfd_handle_t socket);
            static bool             get_bind_address(xfd_handle_t socket,std::string & ip_address, uint16_t & port);
            //get peer ip address associated with socket(connected socket only)
            static sockaddr_storage get_peer_address(xfd_handle_t socket);
            static bool             get_peer_address(xfd_handle_t socket,std::string & ip_address, uint16_t & port);
            
            //init socket address structure
            static sockaddr_in      init_sockaddr_in4(const char* ip_address, const uint16_t port);
            static void             init_sockaddr_in4(sockaddr_in  & sock_addr,const char* ip_address, const uint16_t port);
            static sockaddr_in6     init_sockaddr_in6(const char* ip_address, const uint16_t port);
            static void             init_sockaddr_in6(sockaddr_in6 & sock_addr,const char* ip_address, const uint16_t port);
            //determine by ip_address to use ipv4 or ipv6,so ip_address must be valid ip address(e.g. 192.168.1.1)
            static void             init_sockaddr_inx(sockaddr_storage & sock_addr,const std::string & ip_address, const uint16_t port);
            
        public:
            static xfd_handle_t     create_socket(int domain, int type, int protocol);
            static xfd_handle_t     create_ipv4_udp_socket();
            static xfd_handle_t     create_ipv6_udp_socket();
            static xfd_handle_t     create_ipv4_tcp_socket();
            static xfd_handle_t     create_ipv6_tcp_socket();
            static int              shutdown_socket(xfd_handle_t s, const int nHow); //return error code
            static int              close_socket(xfd_handle_t s);                    //return error code
            
            //socket pair at windows OS,return 0 when successful,otherwise return error code
            static int              create_winsocket_pair(xfd_handle_t socks[2], bool make_overlapped);
            //////////////////////////////////////////////////////////////////////////////////////////////////
            //the following api create and return nonblock socket handle
            //note for some special address:
            //0(listenany) address:  ipv4 = 0.0.0.0,   ipv6 = ::0
            //loopback     address:  ipv4 = 127.0.0.1, ipv6 = ::1
            
            //create ipv4 socket for ipv4 address and ipv6 socket for ipv6 address by checking whether ip_address is ipv4 or ipv6
            static xfd_handle_t     tcp_connect(const std::string connect_ip,const uint16_t connect_port);//bind and connect to target
            static xfd_handle_t     tcp_listen(const std::string bind_ip_address,uint16_t & bind_port,int backlog_count = 8192);
            static xfd_handle_t     ipv6_tcp_listen(uint16_t & bind_port,int backlog_count = 8192);//bind_port is in & out parameter
            static xfd_handle_t     ipv4_tcp_listen(uint16_t & bind_port,int backlog_count = 8192);//bind_port carry out the real bound port
            
            //create nonblock ipv4 or ipv6 udp socket by checking whether ip_address is ipv4 or ipv6
            static xfd_handle_t     udp_connect(const std::string connect_ip,const uint16_t connect_port);//note:udp socket is connected
            //local_address must be valid, e.g. "0.0.0.0" for ipv4 or "0::0" for ipv6;
            static xfd_handle_t     udp_listen(const std::string bind_ip_address,uint16_t & bind_port);
            static xfd_handle_t     ipv4_udp_listen(uint16_t & local_port); //return nonblock socket handle
            static xfd_handle_t     ipv6_udp_listen(uint16_t & local_port); //return nonblock socket handle
            
        public:
            static bool             set_nonblock(xfd_handle_t socket,bool is_non_block);
            static bool             set_tcp_nodelay(xfd_handle_t sockfdhandle,bool is_no_delay);
            
            //receive low-water mark(bytes) for TCP Socket,as default epoll/select is trigger/wakeup even only 1 bytes ready to read
            //set it to property value to boost performance of TCP Socket for bad-network-env
            static bool             set_tcp_recv_low_water_level(xfd_handle_t tcp_socket,const int recv_low_waler_bytes);
            //send low-water mark(bytes) for TCP Socket,as default epoll/select is trigger/wakeup even only 1 bytes ready to write
            //set it to property value to boost performance of TCP Socket for bad-network-env
            static bool             set_tcp_send_low_water_level(xfd_handle_t tcp_socket,const int send_low_waler_bytes);
            
            static int              get_send_buffer(xfd_handle_t sockfd_);
            static bool             set_send_buffer (xfd_handle_t sockfd_, const int bufsize_); //bytes
            static int              get_recv_buffer(xfd_handle_t sockfd_);
            static bool             set_recv_buffer (xfd_handle_t sockfd_, const int bufsize_); //bytes
            
        public: //only avaiable for TCP or paired socket or connected UDP socket
            static int      socket_send(xfd_handle_t s, const void* pBuf, const size_t nBufLen, const int nFlag);
            static int      socket_send(xfd_handle_t s, iovec * blocks_vector, int blocks_count, const int nFlag);
            static int      socket_recv(xfd_handle_t s, void* pBuf, const size_t nBufLen, const int flags);
            
        public://just for UDP/ICMP/IP socket
            static int      socket_sendto(xfd_handle_t s, const void* pBuf, const size_t nBufLen,int flags,struct sockaddr* psaddr, int psaddrLen);
            static int      socket_sendto(xfd_handle_t s,iovec * blocks_vector, int blocks_count,int flags,struct sockaddr* psaddr, int psaddrLen);
            static int      socket_recvfrom(xfd_handle_t s, void* pBuf, const size_t nBufLen, const int flags,struct sockaddr * from_addr,socklen_t * addrlen);
            //paired with recvmsg API
            static int      socket_recvmsg(xfd_handle_t s,msghdr & in_out_msghdr,unsigned int flags = 0);
            
            static  bool    is_support_recvmmsg(); //tell whether support recvmmg at OS Level
            //recvmmsg is Linux only work at >= Linux 2.6.33, for anyother case it fallback to "socket_recvmsg"
            //On success, socket_recvmmsg() returns the number of messages received in msgvec; on error, -1 is returned, and errno is set to indicate the error.
            static int      socket_recvmmsg(xfd_handle_t s,xmmsghdr* in_out_msgvec,unsigned int in_msgvec_count,unsigned int flags = 0);
        };
        
        class xstring_utl
        {
        public:
            //test whether consist of number chars from 0-9
            static bool          digital_string(const std::string & test_string);
            
            static std::string   tostring(const int32_t int32value);
            static std::string   tostring(const uint32_t uint32value);
            static std::string   tostring(const int64_t int64value);
            static std::string   tostring(const uint64_t uint64value);
            static std::string   tostring(const float    floatvalue);
            
            static int32_t       toint32(const std::string & string_value);
            static uint32_t      touint32(const std::string & string_value);
            static int64_t       toint64(const std::string & string_value);
            static uint64_t      touint64(const std::string & string_value);
            
            static void          toupper_string(std::string & in_out_string);//convert every char to upper
            static void          tolower_string(std::string & in_out_string);//convert every char to lower
            
            static std::string   to_hex(const std::string & raw_input);//convert string to hex code.e.g "123" -> "7B"
            static std::string   from_hex(const std::string & hex_input);//convert hex to raw string
            
            static const std::string   uint642hex(uint64_t uint64value);       //unint64 to string of hex format
            static const uint64_t      hex2uint64(const std::string & hex_str);//convert hex string to int64 integer
            
            //alpha code: map ['a','b',... 'x','y','z']  to [0,1,...25]
            //e.g. 25= "z",26 = "ba",261 = "kb"
            static std::string   number_to_alpha(const int32_t number); //encode number to alpha
            static int32_t       alpha_to_number(const std::string alpha_string); //decode alpha to number;
            
            //note:values may include  input if not find split_char, and also return false if input is empty string
            //return how many sub string in vector,
            static int           split_string(const std::string & input,const char split_char,std::vector<std::string> & values);
        public:
            
        public:
            static std::string   base64_encode(unsigned char const* , unsigned int len);
            static std::string   base64_decode(std::string const& s);
            
            static std::string   websafe_base64_encode(unsigned char const* , unsigned int len);
            static std::string   websafe_base64_decode(std::string const& s);
        public:
            static std::string   urlencode(const std::string & input);
            static std::string   urldecode(const std::string & input);
            
            //return every urlencoded params connected by "&"
            static std::string   urlencode(std::map<std::string,std::string> & key_value_params);
        };
        
        //note: treat UTC = GMT at API level
        class xtime_utl
        {
        public:
            //offset used to force to syncronize the time,just use by internally
            static int64_t      time_offset(int64_t offset);
            static int64_t      time_now_ms();  //at Nano level of absolute time as local machine,not affected by NTP syn
            static int64_t      time_now_ms(const int64_t gmt_time_ms);  //at Nano level of absolute time as local machine
            
            //CLOCK_MONOTONIC_RAW mode not affected by NTP syn
            static int64_t      gmttime_ms();   //at ms level of UTC base time
            static int64_t      gmttime_ms(const int64_t tick_time_from_time_now_ms);   //at ms level of UTC base time
            //relative time
            static uint32_t     rtime_ms();
            
            static int          sleep_ms (uint32_t ms_time); //sleep at current thread until timeout
            
            //affected by NTP time sync
            static time_t       gmttime();  //return how many seconds since 1970/01/01 and 00.00.00 at abosulte timezone
            static time_t       gmttime(time_t local);  //based local time
            static std::string  gmt_date();   //conver gmt date to string
            static std::string  gmt_date_time(); //convert GMT ate and hours/minutes to string
            
            //affected by NTP time sync
            static int64_t      gettimeofday();   //how many seconds since 1970/01/01 and 00.00.00 at abosulte timezone
            static int64_t      gettimeofday_ms();//how many ms since 1970/01/01 and 00.00.00 at abosulte timezone
            static int          gettimeofday(struct timeval *tv); //wrap for standard c api 
            
            //return seconds from local base time
            static time_t       localtime();
            static std::string  local_date();      //conver local date to string
            static std::string  local_date_time(); //convert local date and hours/minutes to string
            
            static int			localtime_r(const time_t *timep, struct tm *result);
            static time_t       crate_time(int year, int month, int day, int hour, int minute, int second);
        public: //quickly generate Pseudo-random number
            static int32_t      get_fast_random();
            static uint32_t     get_fast_randomu(uint64_t init_seed = 0);
            static uint64_t     get_fast_random64(uint64_t init_seed = 0);
            static uint32_t     get_fast_random(uint32_t mode);
            static uint32_t     get_fast_random(uint32_t min_value, uint32_t max_value);
        public: //simple and fast implementation for hash and crc
            static uint64_t     get_fast_hash(const void * src_data_ptr,const int32_t src_data_len,int32_t hash_seed = 1);
            static uint64_t     get_fast_crc(const void * src_data_ptr,const int32_t src_data_len,int32_t crc_seed = 1);
        };
        
        //note: Unix/Linux like OS treat most system resource as file,even for device/network interface
        //note: file_utl is more about file io instead of file read/write at simple mode that can be done c-api
        class xfile_utl
        {
        public:
            static bool         file_exist(const std::string & file_path_name);
            static bool         folder_exist(const std::string & folder_path);
            static xfd_handle_t open_file(const std::string file_path,bool read_only); //open as non_block mode
            
        public://general write/read for file-handle that could be any handle of device(or system resource) or file
            static int32_t      write(xfd_handle_t handle,void * buffer, int32_t buffer_length);
            static int32_t      read(xfd_handle_t handle,void * buffer, int32_t buffer_length);
            
            static int          writev(xfd_handle_t handle,iovec * blocks_vector, int blocks_count);
            static int          readv(xfd_handle_t handle,iovec * blocks_vector, int blocks_count);
        public:
            //return howmany bytes readed out, return < 0 if fail, file_content carry the content
            //usally it is used for configuration or information with small size
            //note:some config/txt file may have '/n' or '/r' at end of file,caller need handle those case
            static int          read_file(const std::string file_path_name,std::string & file_content);
            //completely replace the underly file with file_content,return how many bytes writed by write_file
            static int          write_file(const std::string file_path_name,const std::string & file_content);
        };
        
        //wrap the system/kenel level operation
        class xsys_utl
        {
        public:
            static int     sys_cpu_cores(); //return how many cpu cores are avaible to use
            
            static uint32_t  get_sys_process_id(); //query id of current process
            static uint32_t  get_sys_thread_id();  //query id of current calling thread
            
            static uint64_t     get_sys_random_number(); //generate a random number by system ' kernel,it read /dev/urandom for Linux and MacOS
            static std::string  get_sys_random_string(const uint32_t string_size); //generate random string by system ' kernel,it read /dev/urandom for Linux and MacOS
            
            #ifndef __WIN_PLATFORM__ 
            //cpu_index identify cpu core ,valid range is [0- sys_cpu_cores())
            static bool    pthread_setaffinity(pthread_t pthread_id,const int cpu_index);
            //return < 0 if fail,other it indicate which core index affinity for pthread_id
            static int     pthread_getaffinity(pthread_t pthread_id);  //check where pthread at
            #endif
            
            //for UNIX-Like system(Mac, Linux,Freebsd etc)
            static std::string  kernel_version();
            //parse the string to digital version number
            static bool         kernel_version(int & version_major, int & version_feature,int & version_minor);
            
            //execute system command(windows platform not implement yet)
            static int     system_cmd(const std::string system_command_line);
            
            //block operation and only be supported by Linux and MacOS
            static bool    is_process_running(const std::string process_name);
            //note: get_process_pid is a block operation with a little while,so use carefully
            //only support by Linux and MacOS
            static int64_t get_process_pid(const std::string process_name);
            //block operation and only be supported by Linux and MacOS
            static int     kill_process(const std::string process_name);
            
            static int64_t sys_uptime(); //return how many seconds after last reboot for system(OS); only Linux support
            
            static std::string get_mhost_name(); //only support Linux/Unix-like os, return machine'host name
        public:
            // Set a file descriptor to not be passed across execs
            static bool    set_cloexec(int fd_handle);//return true to indicate op is done
  
        public://note:per formance here dont do any params check
            
            //From version 3.8, Linux supports multiqueue tuntap,open_tun_mq_devivce is supported by linux only
            //return <= 0 if fail,otherwise return number of created queues for tun device with dev_name
            static int      open_tun_mq_devivce(std::string & dev_name,int request_queues,std::vector<xfd_handle_t> & queue_handles);
            
            //when persist_mode is false at linux, tun device and related resource will be released auto after device_handle closed
            static int      open_tun_device(std::string & dev_name,bool persist_mode);
            static void     close_tune_device(xfd_handle_t fd_device);
            
            //note:tun_device working at ip layer,write to tun_dev as packet received from wired network_interface
            //app send packet by socket api will go through tcp/udp->ip->tun_device that may trigger read_ready for handle
            //so tun_dev_read can read out the ip packet from tun_device
            static int      tun_dev_write(xfd_handle_t tun_dev_handle,void* ptr_ip_packet, const int32_t packet_len);
            //note:if ptr_ip_buffer is not big enough to hold one ip packet, the left bytes of current ip packet will be droped
            //better practice: buffer_len must be > MTU,so allocte as 2 MTU(like 4098) is safe to hold one packet
            static int      tun_dev_read(xfd_handle_t tun_dev_handle, void* ptr_ip_buffer, const int32_t buffer_len);
            
            //return 0 if successful set
            static int      set_device_tx_queue(const std::string device_name,const int new_tx_queue_len); //set tx queue of eth or tun device,only works at linux platform
            
        public://support linux only right now
            /*1. init last_cpu_used_since_boot and last_cpu_idle_since_boot = 0 at begin
             2. get_cpu_load(last_cpu_used_since_boot,total_cpu_idle_since_boot)
             */
            //return percentage of total cpu usage and detail 
            static  int     get_cpu_load(uint64_t & last_cpu_used_since_boot,uint64_t & last_cpu_idle_since_boot); //return [0-100]
            static  int     get_cpu_load(uint64_t & last_cpu_used_since_boot,uint64_t & last_cpu_idle_since_boot,int & totalUser_percent, int & totalUserLow_percent, int & totalSys_percent, int & totalIdle_percent, int & totalIOWait_percent, int & totalIrq_percent,int &  totalSoftIrq_percent, int & totalSteal_percent);
            
            static  int     get_memory_load(int64_t & free_memory_size);  //return [0-100] as percentage for whole system
            static  bool    get_sys_net_info(const std::string interface_name, uint64_t & rx_bytes, uint64_t & rx_packets, uint64_t & rx_drop_packets, uint64_t & tx_bytes, uint64_t & tx_packets, uint64_t & tx_drop_packets); //return false if fail
            
        public:
            //return < 0 if fail, otherwise return 0-100
            static  int     get_disk_load(const std::string disk_path,int64_t & free_disk_size);
            
            //return < 0 if fail,otherwise return available bytes at target disk
            static int64_t  get_free_disk_space(const std::string disk_path);//disk_path could be "/" or "/tmp"
        };

        
        //token bucket used to traffic reshape or bandwidth limitation
        class xtokenbucket_t
        {
        public:
            xtokenbucket_t(const uint32_t rate_per_second,const uint32_t burst_size,const int64_t init_timenow_ms);
            xtokenbucket_t(const xtokenbucket_t & obj);
            xtokenbucket_t & operator = (const xtokenbucket_t &);
            ~xtokenbucket_t();
        private:
            xtokenbucket_t();
        public:
            //return allocated/consumed tokens, return < 0 if not enough
            //max_allow_borrow use to let small packet pass even reach rate-limit, because small packet most for tcp/udp control signal
            //max_allow_borrow can not over 256 Byte(256 * 8) = 2048 bits
            //might be at very little possible to have temporary wrong decison at multiple thread, but that is ok for most time
            int32_t     consume(const int32_t request_token, int64_t timenow_ms,int32_t max_allow_borrow_bits = 512);
            
            //safe_consume is safe at multiple thread with more cost
            int32_t     safe_consume(const int32_t request_token, int64_t timenow_ms,int32_t max_allow_borrow_bits = 512);
            
            inline uint32_t    get_rate_second() const {return m_rate_per_second;}
            inline uint32_t    get_rate_ms() const {return m_rate_per_ms;}
            inline int32_t     get_tokens_count() const {return m_tokens_count;}
        public:
            void        reset(const int64_t timenow_ms); //this api used to reset to init status but keep last rate
            void        reset(uint32_t rate_per_second,uint32_t burst_size,const int64_t init_timenow_ms); //this api used to dynamic chaning rate
        private:
            uint32_t    m_rate_per_second;  //how many tokens per second
            uint32_t    m_rate_per_ms;      //how many tokens per ms
            int32_t     m_burst_size;       //how many tokens can be burst as max limit
            int32_t     m_tokens_count;     //how many tokens current own,could be negative as borrow
            int64_t     m_last_filltime_ms; //the last time fill bucket
        };
    }//end of namespace base
}//end of namespace top
