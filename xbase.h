// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#undef __WIN_PLATFORM__
#undef __MAC_PLATFORM__
#undef __IOS_PLATFORM__
#undef __ANDROID_PLATFORM__
#undef __LINUX_PLATFORM__

//Posix dependens,Windows'build ask Cygwin or Mingw
#ifndef POSIX
    #define POSIX
#endif

#if defined(POSIX) || defined(__posix)
    #ifndef __POSIX_PLATFORM__  //posix can combine with linux/android
        #define __POSIX_PLATFORM__
    #endif
#endif

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    #ifndef __WIN_PLATFORM__
        #define __WIN_PLATFORM__
    #endif
    #ifndef WIN32
        #define WIN32
    #endif
#elif defined(__APPLE__) //APPLE
    #ifndef __MACH__
        #error  __APPLE__ platform but no __MACH__
    #endif

    #include <TargetConditionals.h>
    #if (TARGET_IPHONE_SIMULATOR == 1) || (TARGET_OS_IPHONE == 1)
        //TARGET_IPHONE_SIMULATOR: iOS simulator
        //TARGET_OS_IPHONE: iOS device(like iPhone,iPad,iPod,iWatch)
        #ifndef __IOS_PLATFORM__
            #define __IOS_PLATFORM__
        #endif
    #elif (TARGET_OS_MAC == 1)   //Mac OSX
        #ifndef __MAC_PLATFORM__
            #define __MAC_PLATFORM__
        #endif
    #else
        #error Unsupport platform!
    #endif

#elif defined(__ANDROID__) || defined(ANDROID) || (__ANDROID) || (ANDROID__)
    #ifndef __ANDROID_PLATFORM__
        #define __ANDROID_PLATFORM__
    #endif
    #ifndef ANDROID
        #define ANDROID
    #endif

#elif defined(LINUX) || defined(linux) || defined(__linux) || defined(__linux__)
    #ifndef __LINUX_PLATFORM__
        #define __LINUX_PLATFORM__
    #endif
    #ifndef LINUX  //most code use this macro
        #define LINUX
    #endif

    #define GET_LINUX_KERNAL_VERSION(major,middle,minor) (((int)major << 16) | ((int)middle << 8) | ((int)minor))
    #ifndef  __LINUX_KERNAL_VERSION__
        #define  __LINUX_KERNAL_VERSION__ GET_LINUX_KERNAL_VERSION(2,6,32)   //as default it is 2.6.32
    #endif
#else
     #error  Unsupport platform!
#endif

#if defined(__LINUX_PLATFORM__) || defined(__MAC_PLATFORM__) || defined(__WIN_PLATFORM__)

    #define __XUSE_THREAD_LOCAL_MEMORY_ALLOCATION__
    #ifdef __XSTRICT_64BIT_ADDRESS_ACCESS___
        //x86 allow access 64bit/float variable without address aligment
        #undef __XSTRICT_64BIT_ADDRESS_ACCESS___
    #endif
#else  //non-x86(e.g. ARM) may have special required for the address of 64bit and float variable
    //usally compiler may genereate special and correct code to access 64bit/float variable,but it is not true for template
    #define __XSTRICT_64BIT_ADDRESS_ACCESS___
#endif

//note:clang is part of gcc family, so clang compiler also defined __GNUC__/__GNUG__
#if defined(__GNUC__) || defined(__GNUG__)     //gcc compiler major version
    #define likely(x)   __builtin_expect(!!(x), 1) //gcc instruction to let compiler optimze it
    #define unlikely(x) __builtin_expect(!!(x), 0)
#else
    #define likely(x)   (x)
    #define unlikely(x) (x)
#endif

#if defined(_M_X64) || defined(__x86_64__) || defined(x86_64)
    //__x86_64__    x86 64bit Windows/Linux
    #define XCPU_ARCH_X86
    #define XCPU_ARCH_64_BITS
    #define XCPU_ARCH_LITTLE_ENDIAN
#elif defined(_M_IX86) || defined(__i386__) || defined(i386)   
    //__i386__      iOS simulator 32 bit or 32bit Mac
    #define XCPU_ARCH_X86
    #define XCPU_ARCH_32_BITS
    #define XCPU_ARCH_LITTLE_ENDIAN
#elif defined(__LP64__) || defined(arm64)
    //__LP64__      is the iOS/Xcode 64bit compile flag
    #ifdef  __IOS_PLATFORM__
        #define XCPU_ARCH_ARM
    #else   //mac is also x86 family chip
        #define XCPU_ARCH_X86
    #endif

    #define XCPU_ARCH_64_BITS
    #define XCPU_ARCH_LITTLE_ENDIAN

#elif defined(__ARMEL__) || defined(ARMV7) || defined(ARMV7s) || defined(__ARM_ARCH_7A__)
    //__ARMEL__     ARMV7/ARMV7s
    #define XCPU_ARCH_ARM
    #define XCPU_ARCH_32_BITS
    #define XCPU_ARCH_LITTLE_ENDIAN
#else
    #define XCPU_ARCH_32_BITS
    #define XCPU_ARCH_LITTLE_ENDIAN
    #error "Unsupport CPU ARCH!"
#endif

//PACKED compiler directive to allow use of pointers directly to the data we want, while forcing the compiler to handle the alignment issues
//as PACKED, the ARM compiler will always generate the appropriate instructions to access the memory correctly, regardless of alignment
//PACKED just use for struct,union ,it apply align for each members of struct
//https://gcc.gnu.org/onlinedocs/gcc-4.0.4/gcc/Type-Attributes.html

#ifdef __ENABLE_C11_ALIGNAS__  //C++11  standard
	#define _ALIGNPACKED_1		alignas(1)
	#define _ALIGNPACKED_4		alignas(4)
	#define _ALIGNPACKED_8		alignas(8)
	#define _ALIGNMENT_8_		alignas(8)
	#define _ALIGNMENT_VAR_(type,name)  alignas(sizeof(type)) type name
#elif defined(__WIN_PLATFORM__)
	#define _ALIGNPACKED_1		__declspec(align(1))
	#define _ALIGNPACKED_4		__declspec(align(4))
	#define _ALIGNPACKED_8		__declspec(align(8))

	#define _ALIGNMENT_8__		__declspec(align(8))
	#define _ALIGNMENT_VAR_(type,name)  __declspec(align(sizeof(type))) type name
#else
	//apply for struct
	#define _ALIGNPACKED_1		__attribute__( ( packed, aligned(1) ) )
	#define _ALIGNPACKED_4		__attribute__( ( packed, aligned(4) ) )
	#define _ALIGNPACKED_8		__attribute__( ( packed, aligned(8) ) )
	//apply as individual member or variable 
	#define _ALIGNMENT_8__		__attribute__((aligned(8)))
	#define _ALIGNMENT_VAR_(type,name)  type name __attribute__((aligned((sizeof(type)))))
#endif


#if defined(__LINUX_PLATFORM__) || defined(__MAC_PLATFORM__) || defined(__WIN_PLATFORM__) //ask each variable'address is aligned strictly at 64bit
    #define _ASSERT_ALIGNMENT_(var)  assert( (((int64_t)&(var)) % sizeof((var))) == 0 )

	#ifdef XCPU_ARCH_ARM  //arm cpu
	//ask 64bit alignment for variable,since ARM must have alignemnt  address t to access int64
		#define  aligned_int64_t      int64_t    _ALIGNMENT_8__
		#define  aligned_uint64_t     uint64_t   _ALIGNMENT_8__
	#else
		#define  aligned_int64_t      int64_t
		#define  aligned_uint64_t     uint64_t
	#endif
#else
    //need remove the warning()
    #define _ASSERT_ALIGNMENT_(var)  void(0)

    //ask 64bit alignment for variable,since ARM must have alignemnt  address t to access int64
    #define  aligned_int64_t      int64_t    _ALIGNMENT_8__
    #define  aligned_uint64_t     uint64_t   _ALIGNMENT_8__
#endif

#define __ASSERT_ALIGNMENT__(var)    _ASSERT_ALIGNMENT_(var)

//force access the value of at main-memory ,instead of read from CPU register, CPU L1/L2/L3
#define _VOLATILE_ACCESS_(type, var)  (*(volatile type*) &(var))
#ifdef  XCPU_ARCH_X86
    #define _CONST_CPU_CACHE_LINE_BYTES_     64 //x86 mostly 64 bytes as one cache line
#else
    #define _CONST_CPU_CACHE_LINE_BYTES_     32 //ARM use 32 bytes as one cache line
#endif

#ifdef XCPU_ARCH_64_BITS //check at compiler
    typedef char __ptr_size_check [2 * ( (sizeof(void*) == 8) != 0) - 1];
#else
    typedef char __ptr_size_check [2 * ( (sizeof(void*) == 4) != 0) - 1];
#endif

#define  xprintf     snprintf


#include <stddef.h>
#include <stdint.h>

//_MSC_VER      Windows Visual Studio Compiler flag
#ifdef __WIN_PLATFORM__
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#ifndef HAVE_STRUCT_TIMESPEC
		#define HAVE_STRUCT_TIMESPEC
	#endif	
    #include <WinSock2.h>
	#include <winnt.h>
	#include <Windows.h>
#else
	#include <unistd.h>
#endif

#ifndef NULL
	#define NULL 0
#endif

#ifndef invalid_handle_t
	#define invalid_handle_t -1
#endif

	typedef int32_t     xfd_handle_t;
	typedef uint32_t    xfd_events_t;

#ifdef DEBUG
    #define __ASSERT_CHECK_ALIGNMENT__
    #define __ASSERT_CHECK_MEMORY__
    #define __ASSERT_CHECK_THREAD_ID__
#endif

//Note:ios platform 'block may exectue at dynamic threads,ask client call sdk api at certain and fixed thread othewise thread_id may run out
//Note: even thread is terminated already, that still consumed 1 because thread_id already increase
enum { enum_max_xthread_count    = 127}; //total allow 127 threads in process

//just unique within one machine/instance/server,but not cross every servers
//Note:max unique id is 1 << 48,after that turn  around to 0 and increase again
enum enum_local_xid_type
{
    enum_local_xid_type_object   = 0,    //tracking object by object id
    enum_local_xid_type_packet   = 1,    //tracking packet by packet id
    enum_local_xid_type_command  = 2,    //tracking command by command id
    enum_local_xid_type_memory   = 3,    //tracking memory allocation
    enum_local_xid_type_count    = 4     //4 type
};

//xmem_handle is the speical memory pointer or handle that dedicated memory layout
typedef void*  xmem_handle; //dummy type to let compiler warning  somebody when use those api wrong


//here concept of process follow the OS and is not about CPU socket(one CPU-Socket may have multiple cpu-core)
//more information: CPU SMB architecture is about one CPU Socket(aka CPU Process) with multiple CPU-Core they shared one system-data-bus
//mutiple-cpu-sockets mostly call multiple-cpu-processor,almost all PC is one-cpu-processor with multiple cpu-core
//one OS-Process can not across CPU Socket as design,but threads of one process are possible to run at different cpu-core under one CPU Socket
//which throw two way to optimization: one os-process per cpu-core with best performance and lowest lock cost; or one os-thread per cpu-core with more expensive for multiple-threads folow and lowest cost for call between inter-threads(because only one process).
//as balance, enum_xprocess_run_mode_single_cpu_core is better option by consider performance and robust with condition of good design.
enum enum_xprocess_run_mode
{
    //usally use this at most time, xbase/xcore layer may optimize based on this setting.
    //depends instance capacity and deploy policy,might have mutitple process but each use singl-cpu-core
    enum_xprocess_run_mode_single_cpu_core   = 0, //single process stick at one single cpu core event CPU have multiple core
    
    //carefully place threads at different cpu-core,need very carefully design interactive between threads
    //this mode usally ask one-thread-per-cpu-core work as standalone and just interact with others at very rare case
    enum_xprocess_run_mode_multiple_cpu_core = 1  //one process'threads maybe distribute to different CPU core
};

enum enum_public_private_key_type
{
    enum_public_private_key_type_ECC25519 = 0,
};

///< 0 means internal error,> 0 for system errno, if = 0 means completetely executed with successful
typedef enum tag_enum_xerror_code
{
    enum_xcode_successful                =  0,   //successful handled
    
    enum_xerror_code_fail                = -1,   //general failure
    enum_xerror_code_errno               = -2,   //general failure with error,see errno for detail
    enum_xerror_code_again               = -3,   //try again,it might be successful at next time
    enum_xerror_code_closed              = -4,   //object is already closed
    enum_xerror_code_time_out            = -5,   //operation time out
    enum_xerror_code_disconnect          = -6,   //connection/socket disconnected or reseted by peer
    enum_xerror_code_blocked             = -7,   //blocked by some reason(e.g.too frequently)
    enum_xerror_code_expired             = -8,   //session is expired already

    enum_xerror_code_bad_param           = -10,   //funtion parameter is invalid
    enum_xerror_code_bad_packet          = -11,   //invalid packet
    enum_xerror_code_bad_file            = -12,   //invalid file content
    enum_xerror_code_bad_stream          = -13,   //invalid stream content
    enum_xerror_code_bad_data            = -14,   //invalid data/content at memory
    enum_xerror_code_bad_config          = -15,   //invalid config
    enum_xerror_code_bad_thread          = -16,   //execute at wrong thread
    enum_xerror_code_bad_userid          = -17,   //invalid userid
    enum_xerror_code_bad_groupid         = -18,   //invalid groupid
    enum_xerror_code_bad_sessionid       = -19,   //invalid session id
    enum_xerror_code_bad_streamid        = -20,   //invalid stream id
    enum_xerror_code_bad_channelid       = -21,   //invalid channel id
    enum_xerror_code_bad_version_code    = -22,   //wrong version code(too low)
    enum_xerror_code_bad_key             = -23,   //invalid encrypt/decrypt key
    enum_xerror_code_bad_format          = -24,   //invalid format
    enum_xerror_code_bad_type            = -25,   //invalid type
    enum_xerror_code_bad_msgid           = -26,   //invalid msg id that not match message 'class
    enum_xerror_code_bad_token           = -27,   //invalid token to pass authentication
    enum_xerror_code_bad_auth            = -28,   //fail as bad authentication information
    enum_xerror_code_bad_privilege       = -29,   //dont have priviledge to execute or access
    enum_xerror_code_bad_object          = -30,   //object is empty or invalid
    enum_xerror_code_bad_status          = -31,   //object is at invalid status
    enum_xerror_code_bad_handle          = -32,   //bad file description, or bad handle
    enum_xerror_code_bad_address         = -33,   //the address is invalid
    enum_xerror_code_bad_size            = -34,   //object size/content size is invalid
    enum_xerror_code_bad_tag             = -35,   //boundary tag is invalid
    enum_xerror_code_bad_threadid        = -36,   //invalid thread id
    enum_xerror_code_bad_initialize      = -37,   //usally class object not init & consruct properly
    enum_xerror_code_bad_transaction     = -38,   //bad transaction
    enum_xerror_code_bad_block           = -39,   //bad block
 
    enum_xerror_code_no_memory           = -50,   //memory running out
    enum_xerror_code_no_address          = -51,   //address running out
    enum_xerror_code_no_handle           = -52,   //socket or file handle running out
    enum_xerror_code_no_resource         = -53,   //other resource running out
    
    enum_xerror_code_not_implement       = -60,  //not implementation
    enum_xerror_code_not_open            = -61,  //object is not opened,similar as enum_error_code_not_avaiable
    enum_xerror_code_not_found           = -62,  //not found packet/resource/object
    enum_xerror_code_not_avaiable        = -63,  //target status is not ready,or resouce limit
    enum_xerror_code_not_handled         = -64,  //found target but nothing to handle
    enum_xerror_code_not_connect         = -65,  //user not connected yet
    enum_xerror_code_not_login           = -66,  //user not login yet
    enum_xerror_code_not_respond         = -67,  //peer dont response ,similar as timeout
    enum_xerror_code_not_sync            = -68,  //not synchronized completely
    enum_xerror_code_not_authorize       = -69,  //need fire authentication first to get priviledge
    
    enum_xerror_code_over_credit_day_limit   = -80,  //purchased credit or free-credit from  gift/ad has  limit for consume per day
    enum_xerror_code_no_balance              = -81,  //dont have enough balance/volume/credit
    enum_xerror_code_over_session_duration   = -82,  //reach session' duration limitation,usally it used for vpn/xtunnel
    enum_xerror_code_over_session_volume     = -83,  //reach session' volume/traffic limitation,usally it used for vpn/xtunnel
    enum_xerror_code_over_day_duration       = -84,  //reach day' duration limitation,usally it used for vpn/xtunnel
    enum_xerror_code_over_day_volume         = -85,  //reach day' volume/traffic limitation,usally it used for vpn/xtunnel
    enum_xerror_code_over_device_limit       = -86,  //only allow limited devices of same user using vpn cocurrent
    enum_xerror_code_over_buffer             = -87,  //over the buffer 'limit
    enum_xerror_code_over_packet_limit       = -88,  //over the max packet 'limit
    enum_xerror_code_over_limit              = -89,  //over general limitation
    
    enum_xerror_code_fail_attach         = -100,
    enum_xerror_code_fail_detach         = -101,
    enum_xerror_code_fail_alloc_addr     = -102,
    enum_xerror_code_fail_alloc_mem      = -103,
    
    enum_xerror_code_recurse_loop        = -110,   //recurse loop endless
    enum_xerror_code_packet_droped       = -111,   //packet/call is dropped when it allow to do it
    enum_xerror_code_addr_not_match      = -112,   //address not matched,packet deliver to wrong object as address not match
    
    enum_xerror_code_no_data             = -120,
}enum_xerror_code;


//enum_xnetwork_id_vpn_service      = 8,    //VPN
//enum_xnetwork_id_msg_service      = 9,    //Messaging/signal systeml
//enum_xnetwork_id_storage_service  = 10,   //files network like AWS S3
//enum_xnetwork_id_cdn_service      = 11,   //CDN
//enum_xnetwork_id_stream_service   = 12,   //streaming
//enum_xnetwork_id_voice_service    = 13,   //real-time voice
//enum_xnetwork_id_video_service    = 14,   //real-time video
//enum_xnetwork_id_db_service       = 15,   //DB clusters

//4bit network id
typedef enum tag_xnetwork_id
{
    enum_xnetwork_id_test             = 0,    //test only,not allow use at product env
    enum_xnetwork_id_pn3              = 1,    //reserved for the centrialized product network
    
    //entry of foundation networks
    enum_xnetwork_id_xbootstrap       = 3,    //bootstrap network
    enum_xnetwork_id_xdns             = 4,    //dns network
    
    //block chain networks
    enum_xnetwork_id_root_chain       = 5,    //root for cross chains
    enum_xnetwork_id_main_chain       = 6,    //main chain of top
    enum_xnetwork_id_side_chain       = 7,    //side chain of top,could be side of side chain ,even more hierarchy
    
    //service networks
    enum_xnetwork_id_root_service     = 8,    //root for cross services
    enum_xnetwork_id_main_service     = 9,    //main service of top as dictory & market place
    enum_xnetwork_id_side_service     = 10,    //side chain of top,could be side of side service ,even more hierarchy
    
    enum_xnetwork_type_max            = 15,
}enum_xnetwork_id;

/*  /////////////////////////////////XIP,XIP2; XID,XID2 defintion/////////////////////////////////////
 //XIP is 64bit address of the logic & virtual IP at overlay network
 XIP definition as total 64bit = [xnet_prefix:8bit][xhost-id:32bit][xend-id:24bit] =
 {
    //xnetwork_type   refer enum_xnetwork_type
    //xaddress_domain is    enum_xaddress_domain_xip
    //xaddress_type   is    enum_xip_type
   -[enum_xaddress_domain_xip:1bit | enum_xip_type:2bit | xnetwork_type:5bit]
   -[zone-id:8bit|cluster-id:8bit|group-id:8bit|node-id:8bit]
   -[process-id:4bit|router-id:4bit|switch-id:8bit|local-id:8bit]
 }
 
 //XIP2 is 128bit address like IPv6 design on top of XIP
 XIP2 = { [xnetwork-id:24bit] - [xnetwork_version#:8bit] - [xinterface_id:32bit] - [XIP: 64bit] } =
 {
   high 64bit:
        -[xinterface_id:32bit]    //unique device id like hash(Mac Address),or hash(Account#),or hash(public Key)
            or [xnetwork clock:32bit] //global unique clock height(usally it is time-block'height)
        -[xnetwork_version#:8bit] //allow network upgrading time-on-time and 0 present any. e.g. elect round# at Chain Network
        -[xnetwork-id: 8-8-8 bit] //A-Class,B-Class,C-Class,D-Class,E-Class Network...
   low  64bit:
        -[XIP: 64bit]
 }

 xnetwork-id = [8-8-8] define like IPv4 Spec, as detail =
 {
    //Note: Netid:0  of Tier#1 Network has been reserved for Loopback & Local & LAN. 0.0.x for Loopback, 0.1.x for Local,and 0.2.x for LAN; but 0.0.0 present as any address. Note: 0.0.1 is register & elect endpoint,0.0.254 for WAN router endpoint,0.0.255 for broadcast entry
    //      Netid:124('T')  of Tier#1 Network has been reserved for "TOP Network"
    //      Netid:1  of Tier#1 Network has been reserved for "ROOT" Network"
    //      Netid:2  of Tier#1 Network has been reserved for "TZ    Network"
    //      Netid:3  of Tier#1 Network has been reserved for "SKY   Network"
    A-Class Network:  0nnnnnnn .nnnnnnnn nnnnnnnn,  total 2^7 - 2  = 126     Tier#1 main networks, and each has 2^16 -2 = 65534 sub-networks;
    B-Class Network:  10nnnnnn nn.nnnnnn nnnnnnnn,  total 2^8 - 2  = 254     Tier#2 main networks, and each has 2^14 -2 = 16382 sub-networks
    C-Class Network:  110nnnnn nnnn.nnnn nnnnnnnn,  total 2^9  -2  = 510     Tier#3 main networks, and each has 2^12 -2 = 4094  sub-networks
    D-Class Network:  1110nnnn nnnnnn.nn nnnnnnnn,  total 2^10 -2  = 1022    Tier#4 main networks, and each has 2^10 -2 = 1022  sub-networks
    E-Class Network:  11110nnn nnnnnnnn. nnnnnnnn,  total 2^11 -2  = 2046    Tier#5 main networks, and each has 2^8 -2  = 254   sub-networks
 }
 
 //XID is 64bit ID of the logic & virtual account/user# at overlay network
 XID definition as total 64bit = [xnet_prefix:8bit][xzone-id:8bit][id#:48bit] =
 {
    //xnetwork_type   refer enum_xnetwork_type
    //xaddress_domain is    enum_xaddress_domain_xid
    //xaddress_type   is    enum_xid_type
    -[enum_xaddress_domain_xid:1bit | enum_xid_type:2bit | xnetwork_type:5bit]
    -[xzone-id:8bit| if have
    -[48bit definintion according enum_xid_type]
 }
 
 //XID2 is 128bit ID of the logic & virtual account/user# at overlay network on top of XID
 XID2 = { [xnetwork-id:24bit] - [xnetwork_version#:8bit] - [xinterface_id:32bit] - [XID: 64bit] } =
 {
  high 64bit:
    -[xinterface_id:32bit]    //unique device id like hash(Mac Address),or hash(Account#),or hash(public Key)
     or [xnetwork clock:32bit] //global unique clock height(usally it is time-block'height)
    -[xnetwork_version#:8bit] //allow network upgrading time-on-time and 0 present any. e.g. elect round# at Chain Network
    -[xnetwork-id: 8-8-8 bit] //A-Class,B-Class,C-Class,D-Class,E-Class Network...

  low 64 bit:
    -[XID]
 }
 */

//note: the definition for XIP&XID is same as the low 64bit of XIP2
//XIP&XID' net-prefix(8bit),shared definition for XIP and XID

#define     is_xip_address_equal(addr_this,addr_other)  (bool)(((addr_this) << 3) == ((addr_other) << 3)) //exclude addr_domain & addr_type
#define     is_xip_local_address_equal(addr_this,addr_other)  (bool)(((addr_this) & 0xFFFFF) == ((addr_other) & 0xFFFFF)) //just compare local address

//   -[enum_xaddress_domain_xip:1bit | enum_xip_type:2bit | xnetwork_type:5bit]
#define     get_address_domain_from_xip(xip)            (int32_t)(((xip) >> 63) & 0x01)      //1bit
#define     get_address_type_from_xip(xip)              (int32_t)(((xip) >> 61) & 0x03)      //2bit
#define     get_network_type_from_xip(xip)              (int32_t)(((xip) >> 56) & 0x1F)      //5bit

#define     set_address_domain_to_xip(xip,domain)       (  (xip) | (((uint64_t)(domain) & 0x01)   << 63) )
#define     set_address_type_to_xip(xip,type)           (  (xip) | (((uint64_t)(type)   & 0x03)   << 61) )
#define     set_network_type_to_xip(xip,type)           (  (xip) | (((uint64_t)(type)   & 0x1F)   << 56) )
#define     reset_address_domain_to_xip(xip)            (  (xip) & (((uint64_t)   0x7FFFFFFFFFFFFFFF)) )
#define     reset_address_type_to_xip(xip)              (  (xip) & (((uint64_t)   0x9FFFFFFFFFFFFFFF)) )
#define     reset_network_type_to_xip(xip)              (  (xip) & (((uint64_t)   0xE0FFFFFFFFFFFFFF)) )



    //XIP'host-id(32bit),   -[zone-id:8bit|cluster-id:8bit|group-id:8bit|node-id:8bit]
#define     get_zone_id_from_xip(xip)                   (int32_t)(((xip) >> 48) & 0xFF)             //8bit
#define     get_cluster_id_from_xip(xip)                (int32_t)(((xip) >> 40) & 0xFF)             //8bit
#define     get_group_id_from_xip(xip)                  (int32_t)(((xip) >> 32) & 0xFF)             //8bit
#define     get_node_id_from_xip(xip)                   (int32_t)(((xip) >> 24) & 0xFF)             //8bit
    //server id = zone_id:cluster_id:group_id:node_id,any server is valid to handle packet if server_id  is  0
#define     get_server_id_from_xip(xip)                 (uint32_t)(((xip) >> 24) & 0xFFFFFFFF)      //32bit

#define     set_zone_id_to_xip(xip,id)                  (  (xip) | (((uint64_t)(id) & 0xFF)       << 48) )
#define     set_cluster_id_to_xip(xip,id)               (  (xip) | (((uint64_t)(id) & 0xFF)       << 40) )
#define     set_group_id_to_xip(xip,id)                 (  (xip) | (((uint64_t)(id) & 0xFF)       << 32) )
#define     set_node_id_to_xip(xip,id)                  (  (xip) | (((uint64_t)(id) & 0xFF)       << 24) )
#define     set_server_id_to_xip(xip,id)                (  (xip) | (((uint64_t)(id) & 0xFFFFFFFF) << 24) )
#define     reset_zone_id_to_xip(xip)                   (  (xip) & (((uint64_t) 0xFF00FFFFFFFFFFFF)) )
#define     reset_cluster_id_to_xip(xip)                (  (xip) & (((uint64_t) 0xFFFF00FFFFFFFFFF)) )
#define     reset_group_id_to_xip(xip)                  (  (xip) & (((uint64_t) 0xFFFFFF00FFFFFFFF)) )
#define     reset_node_id_to_xip(xip)                   (  (xip) & (((uint64_t) 0xFFFFFFFF00FFFFFF)) )
#define     reset_server_id_to_xip(xip)                 (  (xip) & (((uint64_t) 0xFF00000000FFFFFF)) )

//XIP'endpoint-id(24bit)  -[process-id:4bit|local-router-id:4bit|switch-id:8bit|local-id:8bit]
//any process is valid to handle packet if process_id is 0
#define     get_process_id_from_xip(xip)                (int32_t)(((xip) >> 20) & 0x0F)             //4bit
#define     get_router_id_from_xip(xip)                 (int32_t)(((xip) >> 16) & 0x0F)             //4bit
#define     get_switch_id_from_xip(xip)                 (int32_t)(((xip) >> 8)  & 0xFF)             //8bit
#define     get_local_id_from_xip(xip)                  (int32_t)(((xip)     )  & 0xFF)             //8bit
//client_id = router_id:switch_id:lcoal_id
#define     get_client_id_from_xip(xip)                 (int32_t)(((xip)   ) & 0xFFFFF)             //20bit

#define     set_process_id_to_xip(xip,id)               (  (xip) | (((uint64_t)(id) & 0x0F) << 20) ) //4bit
#define     set_router_id_to_xip(xip,id)                (  (xip) | (((uint64_t)(id) & 0x0F) << 16) ) //4bit
#define     set_switch_id_to_xip(xip,id)                (  (xip) | (((uint64_t)(id) & 0xFF) << 8)  ) //8bit
#define     set_local_id_to_xip(xip,id)                 (  (xip) | (((uint64_t)(id) & 0xFF) ) )      //8bit
#define     reset_process_id_to_xip(xip)                (  (xip) & (((uint64_t)0xFFFFFFFFFF0FFFFF)) ) //4bit
#define     reset_router_id_to_xip(xip)                 (  (xip) & (((uint64_t)0xFFFFFFFFFFF0FFFF)) ) //4bit
#define     reset_switch_id_to_xip(xip)                 (  (xip) & (((uint64_t)0xFFFFFFFFFFFF00FF)) ) //8bit
#define     reset_local_id_to_xip(xip)                  (  (xip) & (((uint64_t)0xFFFFFFFFFFFFFF00)) ) //8bit


//XIP2 & XID2 'network id from high 64bit ,shared definition for XIP2 and XID2
//it is invalid when network_id is 0
/*
 -[xinterface_id:32bit]    //unique device id like hash(Mac Address),or hash(Account#),or hash(public Key)
    or [xnetwork clock:32bit] //global unique clock height(usally it is time-block'height)
 -[xnetwork_version#:8bit] //allow network upgrading time-on-time and 0 present any. e.g. elect round# at Chain Network
 -[xnetwork-id: 8-8-8 bit] //A-Class,B-Class,C-Class,D-Class,E-Class Network...
 */
#define     get_network_clock_from_xip2(xip2_high64bit)     (uint32_t)(((xip2_high64bit)>> 32) & 0xFFFFFFFF) //32bit
#define     get_interface_id_from_xip2(xip2_high64bit)      (uint32_t)(((xip2_high64bit)>> 32) & 0xFFFFFFFF) //32bit
#define     get_network_ver_from_xip2(xip2_high64bit)       (int32_t)(((xip2_high64bit) >> 24) & 0xFF)       //8bit
#define     get_network_id_from_xip2(xip2_high64bit)        (int32_t)(((xip2_high64bit)      ) & 0xFFFFFF)   //24bit
#define     get_network_idver_from_xip2(xip2_high64bit)     (uint32_t)(((xip2_high64bit)     ) & 0xFFFFFFFF) //ver:id = 32bit

#define     set_interface_id_to_xip2(xip2_high_64bit,id)    ( (xip2_high_64bit) | (((uint64_t)(id)  & 0xFFFFFFFF)<< 32) )
#define     set_network_ver_to_xip2(xip2_high_64bit,ver)    ( (xip2_high_64bit) | (((uint64_t)(ver) & 0xFF)      << 24) )
#define     set_network_id_to_xip2(xip2_high_64bit,id)      ( (xip2_high_64bit) | (((uint64_t)(id)  & 0xFFFFFF)       ) )
#define     reset_interface_id_to_xip2(xip2_high_64bit)     ( (xip2_high_64bit) & (((uint64_t)(  0x00000000FFFFFFFF))) )
#define     reset_network_ver_to_xip2(xip2_high_64bit)      ( (xip2_high_64bit) & (((uint64_t)(  0xFFFFFFFF00FFFFFF))) )
#define     reset_network_id_to_xip2(xip2_high_64bit)       ( (xip2_high_64bit) & (((uint64_t)(  0xFFFFFFFFFF000000))) )

#define     const_max_routers_count         16
#define     const_max_xnetwork_types_count  32

//each process may has max 255 loopback endpoint at loopback network
typedef enum tag_enum_loopback_network
{
    enum_loopback_network_invalid               =   0x00,
    
    //services for general purpose
    enum_loopback_network_any                   =   0x01, //listen at any pos where receive packet as default if not found matched one
    enum_loopback_network_time_service          =   0x02,
    enum_loopback_network_message_service       =   0x03,
    enum_loopback_network_proxy_service         =   0x04,
    enum_loopback_network_xdns_service          =   0x05,
    
    //services for chain
    enum_loopback_network_chaintime_service     =   0x10,
    enum_loopback_network_elect_service         =   0x11,
    enum_loopback_network_consensus_service     =   0x12,
    enum_loopback_network_contract_service      =   0x13,
    enum_loopback_network_archive_service       =   0x14,
    enum_loopback_network_audit_service         =   0x15,
    enum_loopback_network_accounting_service    =   0x16,
    enum_loopback_network_payment_service       =   0x17,
    enum_loopback_network_bank_service          =   0x18,
    enum_loopback_network_exchange_service      =   0x19,
    enum_loopback_network_market_service        =   0x1a,
    enum_loopback_network_court_service         =   0x1b,
    
    //servcies for platform & application
    enum_loopback_network_xvpn_service          =   0x30,
    enum_loopback_network_xmessage_service      =   0x31,
    enum_loopback_network_xcdn_service          =   0x32,
    enum_loopback_network_xstorage_service      =   0x33,
    
    enum_loopback_network_broadcast_service     =  255,
    enum_loopback_network_max_entry             =  255,
}enum_loopback_network;


#define get_xip_service_address(network_type,service_id) (((uint64_t)enum_xip_type_static) << 61) | (((uint64_t)network_type & 0x1F) << 56) | ((uint64_t)service_id & 0xFF)

#define get_xip_service_address2(network_type,server_id,process_id,service_id) \
    (((uint64_t)enum_xip_type_static) << 61) | (((uint64_t)network_type & 0x1F) << 56) | (((uint64_t)server_id & 0xFFFFFFFF) << 24) | (((uint64_t)process_id & 0x0F) << 20) | ((uint64_t)service_id & 0xFF)

//each network(network_id, network_type) allow create max as 255 services
typedef enum tag_enum_xip_service_id
{
    enum_xip_service_id_invalid     = 0,
    enum_xip_service_id_connection  = 1,
    //enum_xip_service_id_xconnection = 2,
    enum_xip_service_id_tunnel_tun  = 21,
    
    enum_xip_service_id_max         = 255,
}enum_xip_service_id;

//5bit = 32 network subnet
typedef enum tag_enum_xnetwork_type
{
    enum_xnetwork_type_any             = 0,    //not-specified,or not defined,or for any subnet
    enum_xnetwork_type_test            = 1,    //test network
    enum_xnetwork_type_edge            = 2,    //edge network that ineractive with client/user directly
    enum_xnetwork_type_route           = 3,    //route network that routing packet to different addresss
    enum_xnetwork_type_gateway         = 4,    //gateway/oracle network that interactive with 3rd provider
    enum_xnetwork_type_service         = 5,    //service network that provde service capacity like vpn
    enum_xnetwork_type_auth_audit      = 6,    //authetication or authorize or audit
    enum_xnetwork_type_consensus       = 7,    //consensus network
    enum_xnetwork_type_archive         = 8,    //archive/storage network
}enum_xnetwork_type;

//1bit address domain
typedef enum tag_enum_xaddress_domain
{
    enum_xaddress_domain_xip            = 0,  //top xip address(like ipv4/ipv6 but special design for distributed system)
    enum_xaddress_domain_xid            = 1,  //top xid address(like account_address,user_id etc,depends on address type)
}enum_xaddress_domain;

//2bit address type for xid
typedef enum tag_enum_xid_type
{
    enum_xid_type_account		= 0,    //chain/network account address [8bit:prefix][54bit:hash48(account_address)]
    enum_xid_type_userid        = 1,    //User ID as [8bit:prefix][8bit:zone-id][48bit:user#]
    enum_xid_type_name          = 2,    //like dns name [8bit:prefix][54bit:hash54(name)]
    enum_xid_type_session       = 3,    //[8bit:prefix] [54bit:session_id]
}enum_xid_type;

//2 bit address type for xip
typedef enum tag_enum_xip_type
{
    enum_xip_type_dynamic		= 0,    //xip address recyle and changed the magic to avoice duplicate
    enum_xip_type_multicast     = 1,    //multiple cast
    enum_xip_type_nat		    = 2,    //nat address
    enum_xip_type_static		= 3     //static address and never expired,[0-255] are service address
}enum_xip_type;

typedef enum tag_enum_xprotocol_type //total 12bit [0--4095]
{
    enum_xprotocol_type_unknow               = 0,  //unknow type,treate as invalid type
    
    //////////////////////////////xbase define foundation ///////////////////////////////////
    enum_xprotocol_type_xbase_min             = 1,
    
    enum_xprotocol_type_xbase_xlink_start     = 1,  //start for link layer protocol
    enum_xprotocol_type_xbase_xlink_handshake = enum_xprotocol_type_xbase_xlink_start + 0,  //handshake at logic link layer
    enum_xprotocol_type_xbase_xlink_keepalive = enum_xprotocol_type_xbase_xlink_start + 1,  //keepalive at logic link layer
    enum_xprotocol_type_xbase_xlink_data      = enum_xprotocol_type_xbase_xlink_start + 2,  //data at logic link layer
    enum_xprotocol_type_xbase_xlink_signal    = enum_xprotocol_type_xbase_xlink_start + 3,  //ack and report for the logic link layer
    enum_xprotocol_type_xbase_xlink_ping      = enum_xprotocol_type_xbase_xlink_start + 4,  //ping for p2p at link layer
    enum_xprotocol_type_xbase_xlink_null      = enum_xprotocol_type_xbase_xlink_start + 5,  //used to fool the firewall
    enum_xprotocol_type_xbase_xlink_last      = enum_xprotocol_type_xbase_xlink_start + 14, //the last protocol of xlink  network
    
    enum_xprotocol_type_xbase_xip_start       = enum_xprotocol_type_xbase_xlink_last  + 1,  //start for xip (64bit xip address) protocols
    enum_xprotocol_type_xbase_xip_handshake   = enum_xprotocol_type_xbase_xip_start   + 0,  //handshake at xip overlay network
    enum_xprotocol_type_xbase_xip_keepalive   = enum_xprotocol_type_xbase_xip_start   + 1,  //keepalive at xip overlay network
    enum_xprotocol_type_xbase_xip_data        = enum_xprotocol_type_xbase_xip_start   + 2,  //data at xip overlay network
    enum_xprotocol_type_xbase_xip_signal      = enum_xprotocol_type_xbase_xip_start   + 3,  //ack and report at xip overlay network
    enum_xprotocol_type_xbase_xip_last        = enum_xprotocol_type_xbase_xip_start   + 15, //the last protocol of xip overlay network
    
    enum_xprotocol_type_xbase_xip2_start      = enum_xprotocol_type_xbase_xip_last    + 1,  //start for xip2 (128 xip address) protocols
    enum_xprotocol_type_xbase_xip2_handshake  = enum_xprotocol_type_xbase_xip2_start  + 0,  //handshake at xip2 overlay network
    enum_xprotocol_type_xbase_xip2_keepalive  = enum_xprotocol_type_xbase_xip2_start  + 1,  //keepalive at xip2 overlay network
    enum_xprotocol_type_xbase_xip2_data       = enum_xprotocol_type_xbase_xip2_start  + 2,  //data at xip2 overlay network
    enum_xprotocol_type_xbase_xip2_signal     = enum_xprotocol_type_xbase_xip2_start  + 3,  //ack and report at xip2 overlay network
    enum_xprotocol_type_xbase_xip2_last       = enum_xprotocol_type_xbase_xip2_start  + 15, //the last protocol of xip2 overlay network
    //enum_xprotocol_type_xbase_xip2_last is 47
    
    enum_xprotocol_type_xbase_xdns            = 96,  //logic dns resolve system for service discover
    enum_xprotocol_type_xbase_xrouting        = 97,  //routeing at xip overlay network
    enum_xprotocol_type_xbase_xproxy          = 98,  //proxy at xip overlay network
    enum_xprotocol_type_xbase_xmutiplcast     = 99,  //mutiplcast packet
    enum_xprotocol_type_xbase_xbroadcast      = 100, //broadcast packet
  
    enum_xprotocol_type_xbase_max             = 255,
    
    ///////////////////////////xcore define core feature //////////////////////////////////////
    enum_xprotocol_type_core_min             = 256,
    enum_xprotocol_type_core_xid             = 256, //XID  protocol with 64bit  XID  as address
    enum_xprotocol_type_core_xid2            = 257, //XID2 protocol with 128bit XID2 as address
    enum_xprotocol_type_core_xid3            = 258, //XID3 protocol with 387bit XID3 as address
    
    enum_xprotocol_type_core_signal          = 259, //carry signal
    enum_xprotocol_type_core_messaging       = 260, //carry message and topic
    enum_xprotocol_type_core_rpc             = 261, //RPC call
    
    enum_xprotocol_type_core_max             = 511,
    
    ////////////////////////////unused & reserved protocols/////////////////////////////////////
    enum_xprotocol_type_reserve_min          = 512,
    enum_xprotocol_type_reserve_max          = 1023,
    
    ////////////////////////////Blockchain domain features/////////////////////////////////////
    enum_xprotocol_type_chain_min            = 1024,
    enum_xprotocol_type_chain_max            = 2047,
    
    ////////////////////////////all services using protocol////////////////////////////////////
    enum_xprotocol_type_service_min          = 2048,
    enum_xprotocol_type_service_max          = 3071,
    
    /////////////////////////////////apps///////////////////////////////////////
    enum_xprotocol_type_app_min              = 3072,
    enum_xprotocol_type_app_max              = 4095,
    //////////////////////////////////////
}enum_xprotocol_type;

//total 4bit=[0-15] version for enum_xprotocol_type_xbase_xlink_handshake
enum  enum_xlink_handshake_version
{
    //connect
    enum_xlink_handshake_version_init        = 0,
    enum_xlink_handshake_version_init_ack    = 1,
    enum_xlink_handshake_version_sync        = 2,  //client fire sync/link request
    enum_xlink_handshake_version_sync_ack    = 3,  //server response sync
    enum_xlink_handshake_version_ack         = 4,  //ack result
    
    //disconnect
    enum_xlink_handshake_version_shutdown           = 5,
    enum_xlink_handshake_version_shutdown_ack       = 6,
    enum_xlink_handshake_version_shutdown_comlete   = 7,
    
    //error & reconnect
    enum_xlink_handshake_version_rst         = 10,  //refuse to link/connect,client should not retry again
    enum_xlink_handshake_version_error       = 11,  //peer disconnect,client may reconnect again later
    enum_xlink_handshake_version_resync      = 12,  //ask peer enter shandshake process again(fire sync pdu)
};

//total 4bit=[0-15] version for enum_xprotocol_type_xbase_xip and enum_xprotocol_type_xbase_xip2
enum enum_xip_protocol_version
{
};

//total 6bit ,allow max pdu type is 63, under enum_xip2_link_protocol_version_handshake
enum  enum_xlink_hand_shake_pdu_type
{
    //each socket must response sync_ack or sync_rst for sync pdu
    enum_xlink_hand_shake_pdu_type_sync        = 0,  //client fire sync/link request
    enum_xlink_hand_shake_pdu_type_sync_ack    = 1,  //server response sync
    enum_xlink_hand_shake_pdu_type_sync_rst    = 2,  //refuse to link/connect,client should not retry again
    
    enum_xlink_hand_shake_pdu_type_fin         = 3,  //peer disconnect,client may reconnect again later
    enum_xlink_hand_shake_pdu_type_resync      = 4,  //ask peer enter shandshake process again(fire sync pdu)
    
    enum_xlink_hand_shake_pdu_type_max         = 63,
};

//[enum_xpacket_reliable_type(2bit),enum_xpacket_priority_type(2bit),enum_xpacket_order_type[1bit],enum_xpacket_verify_type(2bit)others(3bit)]
typedef enum tag_enum_xpacket_flag
{
    //enum_xpacket_reliable_type(highest 2bit,total 4 types allow) used for signal and network control ,shared by all users
    enum_xpacket_reliable_type_must      = (3 << 14),  //default,not dropable,signal or message
    enum_xpacket_reliable_type_most      = (2 << 14),  //smallest dropable or Video KeyFrame, and retry best(2 time for rudp, use premium route path)
    enum_xpacket_reliable_type_medium    = (1 << 14),  //samll dropable allow(e.g raw voice), retry 1 more time when loss (pick alternative routing)
    enum_xpacket_reliable_type_low       = (0 << 14),  //big dropable allow(e.g. FEC packet), no retry for packet lost and  just rely on ip like udp
    
    //enum_xpacket_priority_type(2bit,total 4 types allow) express the message priority
    enum_xpacket_priority_type_critical  = (3 << 12), //signal,connect/authen,1-1 online txt msg,server down msg,routing path change
    enum_xpacket_priority_type_flash     = (2 << 12), //group text and offline/control message,voice
    enum_xpacket_priority_type_priority  = (1 << 12), //rpc call,video,stream,voice message,
    enum_xpacket_priority_type_routine   = (0 << 12), //content/file transfer,test,ping,load report
    
    enum_xpacket_p2p_flag                = (3 <<  10), //indicate whether packet is peer-to-peer packet
    enum_xpacket_multicast_flag          = (2 <<  10), //indicate whether packet is multicast to specified group
    enum_xpacket_gossip_flag             = (1 <<  10), //indicate whether packet is transfer over gossip
    
    //enum_xpacket_order_type(1bit,total 2 types allow) express whether need follow order
    enum_xpacket_order_type_must         = (1 <<  9),  //default
    //enum_xpacket_order_type_free       = (0 <<  9),  //when set,allow deliver packet as non-order
    
    enum_xpacket_deliver_ack_flag        = (1 <<  8),  //when set ask peer ack the confirmation
 
    //enum_xpacket_verify_type(1bit,total 2 types allow),express the how verify the packet'integrity
    enum_xpacket_verify_type_crc32       = (1 <<  7), //crc32
    //enum_xpacket_verify_type_hash32    = (0 <<  7), //xxhash32 as default
    
    enum_xpacket_extlength_as_flags      = (1 <<  6), //'extflags_len' present addtional flags for packet
    //enum_xpacket_extlength_as_length    = (0 <<  6), //high-8bit of packet length, present 16MB packet by combine with packet_len(low 16bit)

    enum_xpacket_fragment_flag           = (1 <<  5), //indicate whether packet is a fragment packet that need be recombine to full one
    enum_xpacket_fec_flag                = (1 <<  4), //indicate whether packet is encode as erasue code(FEC)
    enum_xpacket_redundancy_flag         = (1 <<  3), //indicate whether packet is delivered as redundancy(e.g. through both udp & tcp)
    enum_xpacket_encrypted_flag          = (1 <<  2), //indicate whether packet is encrypted
    enum_xpacket_obfuscation_flag        = (1 <<  1), //indicate wehther packet processed by obfuscation(anti-dpi) algorithm
    enum_xpacket_compressed_flag         = (1 <<  0)  //indicate whether packet has AES(128bit) CTR compressed,non compress as default
}enum_xpacket_flag;

typedef enum tag_enum_xpacket_flag_count
{
    enum_xpacket_reliable_type_count     = 4,        //[enum_packet_reliable_type_low,enum_packet_reliable_type_must]
    enum_xpacket_priority_type_count     = 4,        //[enum_packet_priority_type_routine,enum_packet_priority_type_critical]
    enum_xpacket_order_type_count        = 2,        //[enum_packet_order_type_free,enum_packet_order_type_must]
    enum_xpacket_verify_type_count       = 2,        //[enum_xpacket_verify_type_none,enum_xpacket_verify_type_hash32]
}enum_xpacket_flag_count;

#define get_xpacket_reliable_type(flags)    (enum_xpacket_flag)((flags) & 0xC000) //1100 0000 0000 0000
#define get_xpacket_priority_type(flags)    (enum_xpacket_flag)((flags) & 0x3000) //0011 0000 0000 0000
#define get_xpacket_transfer_mode(flags)    (enum_xpacket_flag)((flags) & 0x0C00) //0000 1100 0000 0000
#define get_xpacket_order_type(flags)       (enum_xpacket_flag)((flags) & 0x0200) //0000 0010 0000 0000
#define get_xpacket_deliver_flag(flags)     (enum_xpacket_flag)((flags) & 0x0100) //0000 0001 0000 0000
#define get_xpacket_verify_type(flags)      (enum_xpacket_flag)((flags) & 0x0080) //0000 0000 1000 0000
#define get_xpacket_exflags_value(flags)    (enum_xpacket_flag)((flags) & 0x0040) //0000 0000 0100 0000
#define get_xpacket_fragment_flag(flags)    (enum_xpacket_flag)((flags) & 0x0020) //0000 0000 0010 0000
#define get_xpacket_fec_flag(flags)         (enum_xpacket_flag)((flags) & 0x0010) //0000 0000 0001 0000
#define get_xpacket_redundancy_flag(flags)  (enum_xpacket_flag)((flags) & 0x0008) //0000 0000 0000 1000
#define get_xpacket_encrypted_flag(flags)   (enum_xpacket_flag)((flags) & 0x0004) //0000 0000 0000 0100
#define get_xpacket_obfication_flag(flags)  (enum_xpacket_flag)((flags) & 0x0002) //0000 0000 0000 0010
#define get_xpacket_compressed_flag(flags)  (enum_xpacket_flag)((flags) & 0x0001) //0000 0000 0000 0001

/*every packet(msg,transaction...) going through network must follow this header */
#define XBASE_HEADER_FIELDS         \
    /*[4bit:version][12bit:protocol],refer enum_xpacket_type   */     \
    uint16_t  ver_protocol;         \
    /*refer enum_xpacket_flag*/     \
    uint16_t  flags;                \
    /*packet length (include header) as little endia format. may present 16MB by combine with fragment_offset  */ \
    uint16_t  packet_len;           \
    /*refer enum_xpacket_extend_as_exflags*/    \
    uint8_t   extlength;            \
    /*header' size only       */    \
    uint8_t   header_len;           \

//_xbase_header always is 12 bytes
typedef struct tag_xbase_header
{
    XBASE_HEADER_FIELDS
}_ALIGNPACKED_1 _xbase_header;
#define enum_xbase_header_len sizeof(_xbase_header)

#define enum_max_xpacket_header_len  ((int32_t)255)         //256-1
#define enum_max_xpacket_body_len    ((int32_t)16776960)    //enum_max_xpacket_len - enum_max_xpacket_header_len
#define enum_max_xpacket_len         ((int32_t)16777215)    //16MB-1,include header size

//extend as high 8bit of total 24bit length if enum_xpacket_extend_as_exlength present
#define get_xpacket_len(_xbase_header_ptr)  ((_xbase_header_ptr->flags & enum_xpacket_extend_as_exlength) ? (*((uint32_t*)&_xbase_header_ptr->packet_len) >> 8) : (_xbase_header_ptr->packet_len))

#define get_xpacket_protocol(header)         ((header)->ver_protocol & 0xFFF) //Lowest 12bit
#define get_xpacket_protocol_version(header) ((header)->ver_protocol >> 12)   //High 4bit
#define set_xpacket_protocol_version(header,_protocol,_version) (header)->ver_protocol = (((_version) << 12) | ((_protocol) & 0xFFF))

#define enum_xpacket_max_fragment_count      255

//xlink used for udp that shared by multiple logic connect,using logic port & token to connect/accept/listen
//standard _xlink_header is enum_xlink_header_len(26 bytes) except if xlink options defined in header(check header_len is > enum_xlink_header_len)
typedef struct tag_xlink_header
{
    XBASE_HEADER_FIELDS
    uint32_t  checksum;                 //crc32(...) or xxhash32(the below data of hash_crc) by depending flags
    uint16_t  sequnceid;                //increased packet id for scope defined from_logic_port & to_logic_port
    uint16_t  to_logic_port;            //target logic port of UDP socket
    uint16_t  to_logic_port_token;      //to_logic_port_token paired with  to_logic_port
    uint16_t  from_logic_port;          //source logic port of UDP socket
    uint16_t  from_logic_port_token;    //access token paired with from_logic_port
    uint16_t  fragment_id;              //fragments identitfy under  same sequence_id, enable if when set enum_xpacket_fragment_flag
    uint16_t  fragments_count;          //total how many fragments
    uint8_t   compressrate;             //indicate how many times is compressed if enum_xpacket_process_flag_compress up
}_ALIGNPACKED_1 _xlink_header;
#define enum_xlink_header_len sizeof(_xlink_header)

#define enum_max_xlink_header_len    ((int32_t)255)      //256-1
#define enum_max_xlink_body_len      ((int32_t)65280)    //enum_max_xlink_packet_len - enum_max_xlink_header_len
#define enum_max_xlink_packet_len    ((int32_t)65535)    //64KB-1,include header size
#define enum_min_xlink_mtu           ((int32_t)256)
#define enum_max_xlink_mtu           ((int32_t)65536)

//standard _xip_header is enum_xip_header_len(40 bytes) except if xip options defined in header(check header_len is > enum_xip_header_len)
typedef struct tag_xip_header
{
    XBASE_HEADER_FIELDS
    uint8_t   compressrate;     //indicate how many times is compressed if enum_xpacket_process_flag_compress up
    uint8_t   TTL;              //total how many hop allowed, it also present total fragments count if enum_xpacket_fragment_flag set
    uint8_t   to_xport;         //0 means invalid. it might be logic xip port(1-255) or it is nat port for nat address(enum_xip_type_nat)
    uint8_t   from_xport;       //0 means invalid. it might be logic xip port(1-255) or it is nat port for nat address(enum_xip_type_nat)
    uint32_t  checksum;         //crc32(...) or xxhash32(the below data of hash_crc) by depending flags
    uint32_t  sesssion_id;      //session-id under [from,to] context
    uint16_t  sequence_id;      //increased packet id under session_id(if have),or under from_addr
    uint16_t  to_xaddr_token;   //access token to allow send packet to
    uint64_t  to_xaddr;         //refer XIP/XID definition
    uint64_t  from_xaddr;       //refer XIP/XID definition
}_ALIGNPACKED_1 _xip_header;
#define enum_xip_header_len sizeof(_xip_header)

//standard _xip_header is enum_xip_header_len(56 bytes) except if xip options defined in header(check header_len is > enum_xip_header_len)
typedef struct tag_xip2_header
{
    XBASE_HEADER_FIELDS
    uint8_t   compressrate;     //indicate how many times is compressed if enum_xpacket_process_flag_compress up
    uint8_t   TTL;              //total how many hop allowed, it also present total fragments count if enum_xpacket_fragment_flag set
    uint8_t   to_xport;         //0 means invalid. it might be logic xip port(1-255) or it is nat port for nat address(enum_xip_type_nat)
    uint8_t   from_xport;       //0 means invalid. it might be logic xip port(1-255) or it is nat port for nat address(enum_xip_type_nat)
    uint32_t  checksum;         //crc32(...) or xxhash32(the below data of hash_crc) by depending flags
    uint32_t  sesssion_id;      //session-id under [from,to] context
    uint16_t  sequence_id;      //increased packet id under session_id(if have),or under from_addr
    uint16_t  to_xaddr_token;   //access token to allow send packet to
    uint64_t  to_xaddr_low;     //refer XIP/XID definition
    uint64_t  from_xaddr_low;   //refer XIP/XID definition
    uint64_t  to_xaddr_high;    //refer XIP2/XID2 definition
    uint64_t  from_xaddr_high;  //refer XIP2/XID2 definition
}_ALIGNPACKED_1 _xip2_header;
#define enum_xip2_header_len sizeof(_xip2_header)

typedef struct tag_xapp_header
{
    XBASE_HEADER_FIELDS
    uint32_t  checksum;         //crc32(...) or xxhash32(the below data of hash_crc) by depending flags
    uint8_t   compressrate;     //indicate how many times is compressed if enum_xpacket_process_flag_compress up
}_ALIGNPACKED_1 xapp_header;

//LINUX,WINDOWS dont have sin_len
#if defined(__LINUX_PLATFORM__) || defined(__ANDROID_PLATFORM__) || defined(__WIN_PLATFORM__)
    #undef HAVE_SIN6_LEN
    #undef HAVE_SIN_LEN
    #undef HAVE_SA_LEN
    #undef HAVE_SCONN_LEN
#else
    #ifndef HAVE_SIN6_LEN  //linux,ios,android all have sockaddr_in6.sin6_len
        #define  HAVE_SIN6_LEN
    #endif

    #ifndef HAVE_SIN_LEN  //linux,ios,android all have sockaddr_in.sin_len
        #define  HAVE_SIN_LEN
    #endif

    #ifndef HAVE_SA_LEN  //linux,ios,android all have sockaddr.sa_len
        #define HAVE_SA_LEN
    #endif

    #if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
        #ifndef HAVE_SCONN_LEN
        #define HAVE_SCONN_LEN
        #endif
    #endif
#endif


//convert windows socket error to Linux&Unix-like error
//https://docs.microsoft.com/en-us/windows/desktop/WinSock/windows-sockets-error-codes-2
#ifdef  __WIN_PLATFORM__
	#ifndef _INC_ERRNO
		#define _INC_ERRNO
	#endif

	#undef EAGAIN
	#undef ENETDOWN
	#undef ENETRESET
	#undef ECONNRESET
	#undef ECONNABORTED
	#undef EISCONN
	#undef ENOTCONN
	#undef EINTR
	#undef ETIMEDOUT
	#undef EBADF
	#undef EWOULDBLOCK
	#undef EINPROGRESS 
	#undef EALREADY
	#undef ENOTSOCK 
	#undef EDESTADDRREQ
	#undef EMSGSIZE
	#undef EPROTOTYPE
	#undef ENOPROTOOPT
	#undef EPROTONOSUPPORT
	#undef ESOCKTNOSUPPORT
	#undef EOPNOTSUPP
	#undef EPFNOSUPPORT
	#undef EAFNOSUPPORT
	#undef EADDRINUSE
	#undef EADDRNOTAVAIL
	#undef ENETDOWN
	#undef ENETUNREACH
	#undef ENETRESET
	#undef ECONNABORTED
	#undef ECONNRESET
	#undef ENOBUFS
	#undef EISCONN
	#undef ENOTCONN
	#undef ESHUTDOWN
	#undef ETOOMANYREFS
	#undef ETIMEDOUT
	#undef ECONNREFUSED
	#undef ELOOP
	#undef ENAMETOOLONG
	#undef EHOSTDOWN
	#undef EHOSTUNREACH
	#undef ENOTEMPTY
	#undef EPROCLIM
	#undef EUSERS
	#undef EDQUOT
	#undef ESTALE
	#undef EREMOTE
	#undef EINVAL

    #define	EAGAIN          WSAEWOULDBLOCK		/* Resource temporarily unavailable */	
    #define ENETDOWN        WSAENETDOWN         /* Network is down */
    #define	ENETRESET       WSAENETRESET        /* Network dropped connection on reset */
    #define ECONNRESET      WSAECONNRESET       /* Connection reset by peer.*/
    #define	ECONNABORTED	WSAECONNABORTED		/* Software caused connection abort */
    #define	EISCONN         WSAEISCONN          /* Socket is already connected */
    #define	ENOTCONN        WSAENOTCONN         /* Socket is not connected */
    #define EINTR           WSAEINTR            /* Interrupted system call */
    #define EINVAL          WSAEINVAL           /* Invalid argument.*/
    #define ETIMEDOUT       WSAETIMEDOUT        /* Operation timed out */
    #define EBADF           WSA_INVALID_HANDLE  /*Specified event object handle is invalid*/

	#define EWOULDBLOCK             WSAEWOULDBLOCK
	#define EINPROGRESS             WSAEINPROGRESS	/* Operation now in progress */
	#define EALREADY                WSAEALREADY		/* Operation already in progress */
	#define ENOTSOCK                WSAENOTSOCK
	#define EDESTADDRREQ            WSAEDESTADDRREQ
	#define EMSGSIZE                WSAEMSGSIZE		 /* Message too long */
	#define EPROTOTYPE              WSAEPROTOTYPE
	#define ENOPROTOOPT             WSAENOPROTOOPT
	#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
	#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
	#define EOPNOTSUPP              WSAEOPNOTSUPP
	#define EPFNOSUPPORT            WSAEPFNOSUPPORT
	#define EAFNOSUPPORT            WSAEAFNOSUPPORT
	#define EADDRINUSE              WSAEADDRINUSE
	#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
	#define ENETDOWN                WSAENETDOWN
	#define ENETUNREACH             WSAENETUNREACH	/* Network is unreachable.*/
	#define ENETRESET               WSAENETRESET
	#define ECONNABORTED            WSAECONNABORTED
	#define ECONNRESET              WSAECONNRESET
	#define ENOBUFS                 WSAENOBUFS		/* No buffer space available */
	#define EISCONN                 WSAEISCONN
	#define ENOTCONN                WSAENOTCONN
	#define ESHUTDOWN               WSAESHUTDOWN
	#define ETOOMANYREFS            WSAETOOMANYREFS
	#define ETIMEDOUT               WSAETIMEDOUT
	#define ECONNREFUSED            WSAECONNREFUSED	/* Connection refused.*/
	#define ELOOP                   WSAELOOP
	#define ENAMETOOLONG            WSAENAMETOOLONG
	#define EHOSTDOWN               WSAEHOSTDOWN
	#define EHOSTUNREACH            WSAEHOSTUNREACH	 /* No route to host. */
	#define ENOTEMPTY               WSAENOTEMPTY
	#define EPROCLIM                WSAEPROCLIM
	#define EUSERS                  WSAEUSERS
	#define EDQUOT                  WSAEDQUOT
	#define ESTALE                  WSAESTALE
	#define EREMOTE                 WSAEREMOTE
#else
	#include <errno.h>
#endif

#define  xsnprintf     snprintf
//implement a lock-free log system through system write(append mode)
//note: xwrite_log using default logger of process which actually allow multiple logger(refer xlog.h)
#ifdef __cplusplus
extern "C"
{
#endif
    int         xerrno(void);         //get current sytem error code(just wrap for errno()
    const char *xerrorstr(int error); //convert system error to string
    //level refer enum_xlog_level
    void        xdump_stack(int level,const char *_module_name);        //dump current call-stack,and write to trace file
    void        xabort(const char * errmsg); //abor process
    
    typedef enum tag_enum_xlog_level
    {
        enum_xlog_level_debug    = 0,
        enum_xlog_level_info     = 1,
        enum_xlog_level_key_info = 2,
        enum_xlog_level_warn     = 3,
        enum_xlog_level_error    = 4,
        
        enum_xlog_level_max      = 4,
        enum_xlog_level_uninit   = 9
    }enum_xlog_level;
    
    //allow redefine __MODULE__
    #define __MODULE__  "xbase"
    
    //note: need extern "C" of relate callback function when at C++
    //return nil if using default file sytem inside of xbase, or return a valid handle for log_file_name if using own file system
    typedef int  (*_func_create_log_file_cb)(const char * log_file_name);//callback to extern when new log file is going to create
    typedef bool (*_func_hook_trace_cb)(enum_xlog_level level,const char* _module_name,const char* msg,const int msg_length);//return true to prevent to writed into log file,return false as just hook purpose
    
    //the following api apply to default module(0)
    //init xlog by calling xinit_log or xset_log_level and xset_log_file
    void  xinit_log(const char* log_file_dir,bool tracking_thread,bool is_allow_rotate);
    void  xclose_log();  //just only be called when whole process is going to quit, not recommend use it
    void  xset_log_level(enum_xlog_level new_level);
    //return current setting level if already inited,othewise it return error code(< 0)
    int   xget_log_level();
    int   xset_log_file_hook(_func_create_log_file_cb _create_file_cb);//it is nil as default
    int   xset_log_trace_hook(_func_hook_trace_cb  _hook_trace_cb);
    int   xset_trace_lines_per_file(const int max_tracelines_per_file);//decide to rotate to new log file after how many lines
    
    void  xwrite_log(const char * module,enum_xlog_level level,const char* msg, ...);
    void  xwrite_log2(const char * module,const char * file, const char* function, const int line,enum_xlog_level level,const char* msg, ...);
    void  xflush_log();  //just call when really need because it may has performance lost

    void  xassert_debug(bool result,const char * module,const char * file, const char* function, const int line, const char * expression);
    void  xassert_debug2(bool result,const char * module,const char * file, const char* function, const int line,const char * expression,const char* msg, ...);
    void  xassert_release(bool result,const char * module,const char * file, const int line, const char * expression);
    void  xassert_release2(bool result,const char * module,const char * file, const int line,const char * expression,const char* msg,...);
    
#ifdef __cplusplus
}
#endif

#ifdef DEBUG
    #define xassert(x)            xassert_debug((x),__MODULE__,__FILE__,__FUNCTION__,__LINE__,#x)
    #define xassert2(x,msg,...)   xassert_debug2((x),__MODULE__,__FILE__,__FUNCTION__,__LINE__,#x,msg,__VA_ARGS__)
    #define xdbgassert(x)         xassert_debug((x),__MODULE__,__FILE__,__FUNCTION__,__LINE__,#x)
#else
    //to optmize the performance, just use __LINE__ and __FILE__
    #define xassert(x)            xassert_release((x),__MODULE__,__FILE__,__LINE__,#x)
    #define xassert2(x,msg,...)   xassert_release2((x),__MODULE__,__FILE__,__LINE__,#x,msg,__VA_ARGS__)
    #define xdbgassert(x)         void(0)  //TODO enable it
#endif

#ifdef DEBUG
    #ifdef __DISABLE_FILE_LINE_LOG__
        #define xdbg(...)         xwrite_log(__MODULE__,enum_xlog_level_debug,__VA_ARGS__)
        #define xdbg_info(...)    xwrite_log(__MODULE__,enum_xlog_level_info,__VA_ARGS__)
        #define xinfo(...)        xwrite_log(__MODULE__,enum_xlog_level_info,__VA_ARGS__)
        #define xkinfo(...)       xwrite_log(__MODULE__,enum_xlog_level_key_info,__VA_ARGS__)
        #define xwarn(...)        xwrite_log(__MODULE__,enum_xlog_level_warn,__VA_ARGS__)
        #define xerror(...)       xwrite_log(__MODULE__,enum_xlog_level_error,__VA_ARGS__)
        #define xwarn_err(...)    xwrite_log(__MODULE__,enum_xlog_level_error,__VA_ARGS__)
    #else
        #define xdbg(...)         xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_debug,__VA_ARGS__)
        #define xdbg_info(...)    xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_info,__VA_ARGS__)
        #define xinfo(...)        xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_info,__VA_ARGS__)
        #define xkinfo(...)       xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_key_info,__VA_ARGS__)
        #define xwarn(...)        xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_warn,__VA_ARGS__)
        #define xerror(...)       xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_error,__VA_ARGS__)
        #define xwarn_err(...)    xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_error,__VA_ARGS__)
    #endif
    //allow carry __FILE__,__FUNCTION__,and __LINE__ as well
    #define xdbg2(...)        xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_debug,__VA_ARGS__)
    #define xinfo2(...)       xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_info,__VA_ARGS__)
    #define xkinfo2(...)      xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_key_info,__VA_ARGS__)
    #define xwarn2(...)       xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_warn,__VA_ARGS__)
    #define xerror2(...)      xwrite_log2(__MODULE__,NULL,__FUNCTION__,__LINE__,enum_xlog_level_error,__VA_ARGS__)
#else
    #define xdbg(...)         void(0)
    #define xdbg_info(...)    void(0)
    #define xinfo(...)        xwrite_log(__MODULE__,enum_xlog_level_info,__VA_ARGS__)
    #define xkinfo(...)       xwrite_log(__MODULE__,enum_xlog_level_key_info,__VA_ARGS__)
    #define xwarn(...)        xwrite_log(__MODULE__,enum_xlog_level_warn,__VA_ARGS__)
    #define xerror(...)       xwrite_log(__MODULE__,enum_xlog_level_error,__VA_ARGS__)
    #define xwarn_err(...)    xwrite_log(__MODULE__,enum_xlog_level_warn,__VA_ARGS__) //downgrade to warning at relase mode

    //force using simple way to trace at release mode
    #define xdbg2(...)         void(0)
    #define xinfo2(...)        xwrite_log(__MODULE__,enum_xlog_level_info,__VA_ARGS__)
    #define xkinfo2(...)       xwrite_log(__MODULE__,enum_xlog_level_key_info,__VA_ARGS__)
    #define xwarn2(...)        xwrite_log(__MODULE__,enum_xlog_level_warn,__VA_ARGS__)
    #define xerror2(...)       xwrite_log(__MODULE__,enum_xlog_level_error,__VA_ARGS__)
#endif
