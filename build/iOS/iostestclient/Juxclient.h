//
//  Juxclient.h
//  xclient
//
//  Created by Taylor Wei on 9/2/15.
//  Copyright (c) 2016 Dingtone,Inc. All rights reserved.
//

#ifndef __Juxclient_h__
#define __Juxclient_h__

#include <stdint.h>
#include <string>
#include <vector>

namespace Jeesu
{
    typedef enum tag_enum_xclient_error_code
    {
        //code >=0 is ok
        enum_xclient_code_queue_up                  =  2,   //command/event/packet is queued ,they will be handled by async
        enum_xclient_code_async_fire                =  1,   //the command/event already async fired ,that will be handled at next loop
        enum_xclient_code_successful                =  0,   //successful handled
        
        enum_xclient_error_code_fail                = -1,   //general failure
        enum_xclient_error_code_errno               = -2,   //error,see errno for detail
        enum_xclient_error_code_again               = -3,   //try again
        enum_xclient_error_code_time_out            = -4,   //operation time out
        enum_xclient_error_code_obj_closed          = -5,   //object is already closed
        
        enum_xclient_error_code_bad_object          = -6,   //object is empty or invalid
        enum_xclient_error_code_bad_status          = -7,   //object is at invalid status
        enum_xclient_error_code_bad_handle          = -8,   //bad file description, or bad handle
        enum_xclient_error_code_bad_address         = -9,   //the address is invalid
        enum_xclient_error_code_bad_param           = -10,   //funtion parameter is invalid
        enum_xclient_error_code_bad_packet          = -11,   //invalid packet
        enum_xclient_error_code_bad_file            = -12,   //invalid file content
        enum_xclient_error_code_bad_stream          = -13,   //invalid stream content
        enum_xclient_error_code_bad_data            = -14,   //invalid data/content at memory
        enum_xclient_error_code_bad_config          = -15,   //invalid config
        enum_xclient_error_code_bad_thread          = -16,  //execute at wrong thread
        enum_xclient_error_code_bad_userid          = -17,   //invalid userid
        enum_xclient_error_code_bad_groupid         = -18,   //invalid groupid
        enum_xclient_error_code_bad_sessionid       = -19,   //invalid session id
        enum_xclient_error_code_bad_streamid        = -20,   //invalid stream id
        enum_xclient_error_code_bad_channelid       = -21,   //invalid channel id
        enum_xclient_error_code_bad_version_code    = -22,   //wrong version code(too low)
        enum_xclient_error_code_bad_key             = -23,   //invalid encrypt/decrypt key
        enum_xclient_error_code_bad_format          = -24,   //invalid format
        enum_xclient_error_code_bad_type            = -25,   //invalid type
        enum_xclient_error_code_bad_msgid           = -26,   //invalid msg id that not match message 'class
        enum_xclient_error_code_bad_token           = -27,   //invalid token to pass authentication
        enum_xclient_error_code_bad_auth            = -28,   //fail as bad or invalid authentication information
        enum_xclient_error_code_bad_privilege       = -29,   //dont have priviledge to execute or access
        
        enum_xclient_error_code_not_open            = -31,  //object is not opened,similar as enum_error_code_not_avaiable
        enum_xclient_error_code_not_found           = -32,  //not found packet/resource/object
        enum_xclient_error_code_not_avaiable        = -33,  //target status is not ready
        enum_xclient_error_code_not_handled         = -34,  //found target but nothing to handle
        enum_xclient_error_code_not_connect         = -35,  //user not connected yet
        enum_xclient_error_code_not_login           = -36,  //user not login yet
        enum_xclient_error_code_not_respond         = -37,  //peer dont response ,similar as timeout
        enum_xclient_error_code_not_sync            = -38,  //not synchronized completely
        enum_xclient_error_code_not_authorize       = -39,  //need fire authentication first and get priviledge
        
        enum_xclient_error_code_geo_restrict_access     = -42,  //restrict access as geolocation limit
        
        enum_xclient_error_code_over_credit_day_limit   = -43,  //purchased credit or free-credit from  gift/ad has  limit for consume per day
        enum_xclient_error_code_no_balance              = -44,  //dont have enough balance/volume/credit
        enum_xclient_error_code_over_session_duration   = -45,  //reach session' duration limitation,usally it used for vpn/xtunnel
        enum_xclient_error_code_over_session_volume     = -46,  //reach session' volume/traffic limitation,usally it used for vpn/xtunnel
        enum_xclient_error_code_over_day_duration        = -47,  //reach day' duration limitation,usally it used for vpn/xtunnel
        enum_xclient_error_code_over_day_volume         = -48,  //reach day' volume/traffic limitation,usally it used for vpn/xtunn
        
        enum_xclient_error_code_over_device_limit       = -49,  //only allow limited devices of same user using vpn cocurrent
        
        //special for connect
        enum_xclient_error_code_connect_bad_address     = -50,  //provider wrong or bad address and port
        enum_xclient_error_code_connect_bad_protocol    = -51,  //giving the unsupported protocol,e.g. sctp disabled for android as binary size
        enum_xclient_error_code_connect_no_network      = -52,  //dont found any network to connect
        enum_xclient_error_code_connect_host_unreach    = -53,  //host(target) is un reachable
        enum_xclient_error_code_connect_time_out        = -54,  //timeout to waiting for connect
        enum_xclient_error_code_connect_sys_fail        = -55,  //socket connect fail as system error, refer system ' errno
        enum_xclient_error_code_connect_fail            = -56,  //socket connect fail as 'logic restrict,check log to fix bug(client or xcore)
        enum_xclient_error_code_connect_disconnect      = -57,  //socket disconnected
        enum_xclient_error_code_connect_host_down       = -58,  //host down(reachable but host/instance down)
        enum_xclient_error_code_connect_host_offline    = -59, //tell the connecting server is being not available to serve forever
        enum_xclient_error_code_connect_host_load_high  = -60, //tell the connecting server load is too high,try other if possible
        enum_xclient_error_code_connect_host_redirect   = -61, //ask client redirect to other server(return the new IP to client)
        
        enum_xclient_error_code_connect_peer_reset      = -62,  //peer system/socket reset the connection at OS level
        enum_xclient_error_code_connect_sys_fail_assgin_address  = -63,  //Can't  assign requested address at OS level
        
        enum_xclient_error_code_connect_host_wrong_service  = -64,  //connect to wrong service,or wrong mode(basic,premium...) of service
        
        enum_xclient_error_code_connect_idle_timeout        = -65,  //connection/socket closed due to idel with too long
        
        enum_xclient_error_code_account_device_suspend      = -80,  //this device is suspend that not allow login but still valid
        enum_xclient_error_code_account_device_destroyed    = -81,  //device is deactivated,nolong use anymore
        enum_xclient_error_code_account_user_suspend        = -82,  //user is suspend that not allow any device to login
        enum_xclient_error_code_account_user_destroyed      = -83,  //user is deactivated,nolong use anymore
        
        enum_xclient_error_code_too_big             = -90,  //packet or params is too big
        enum_xclient_error_code_recurse_loop        = -91,  //recurse loop to self
        enum_xclient_error_code_fail_lock           = -92,  //fail to acquired distributed lock
        enum_xclient_error_code_fail_finish_auth    = -93,  //fired request but fail to finish authenticate
        enum_xclient_error_code_expired             = -94,  //request already expired
        enum_xclient_error_code_droped              = -95,  //packet is dropped
        
        enum_xclient_error_code_no_token            = -100,  //dont find related token(eg. pushtoken)
        enum_xclient_error_code_no_resource         = -101,  //resouce limit
        enum_xclient_error_code_no_data             = -102,  //no any data
        enum_xclient_error_code_no_privilege        = -103,  //dont have priviledge to execute or access
        
        //application/user space error, different from errno.h that is sytem error define
        enum_xclient_error_code_user_space          = -12345678,
        
        enum_xclient_error_code_address_not_found   = enum_xclient_error_code_user_space - 3,
        enum_xclient_error_code_data_not_available  = enum_xclient_error_code_user_space - 4
    }enum_xclient_error_code;
    
    //following are most common error code from http,refer https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
    enum enum_http_status_error_code
    {
        enum_http_status_error_code_301_permanent_redirect        = 301,  //301 Moved Permanently
        
        enum_http_status_error_code_302_temp_redirect             = 302,  //tempority redirect request
        
        //Temporary redirect
        enum_http_status_error_code_307_tempory_redirect          = 307,  //307 Moved Temporarily
        
        //request fail as client not handle request properly, error string show detail error reason
        enum_http_status_error_code_400_bad_request               = 400,  //400 Bad Request
 
        enum_http_status_error_code_403_forbidden                 = 403,  //403 Forbidden
        
        //the target resource is not found,usally the filekey is not correct or it is already deleted
        enum_http_status_error_code_404_not_found                 = 404,  //404 Not Found
        
        //must provide the Content-Length HTTP header, right now lib handle the content-length
        enum_http_status_error_code_411_contentlength_required    = 411,  //411 Length Required
        
        //client may retry it later that should be tempory error
        enum_http_status_error_code_500_server_internal_error     = 500,  //500 Internal Server Error
    
        enum_http_status_error_code_502_bad_gateway               = 502,
        enum_http_status_error_code_504_gateway_timeout           = 504,
        
        //usally it cause request too frequently in short time, client should slow down and retry later
        enum_http_status_error_code_503_service_not_available     = 503,  //503 Service Unavailable or 503 Slow Down
    };
  
    //4 bit ack_type(max ac_type is 15)
    enum enum_client_msg_deliver_ack_type
    {
        //current packet/msg id lost
        enum_client_msg_deliver_ack_lost           = 0,
        //current packet/msg id received
        enum_client_msg_deliver_ack_recev          = 1,
        
        //the following ack type are not supported yet, just define here
        
        //the last sequenced/orded packet id received
        enum_client_msg_deliver_ack_order_recv     = 2,
        //the last sequenced/orded packet id lost
        enum_client_msg_deliver_ack_order_lost     = 3,
        
        //ack have ids-list that already recevied,useful when just few packet received
        enum_client_msg_deliver_ack_select_recev   = 4,
        //ack have ids-list that not received(lost or timeout),useful when just few packet lost
        enum_client_msg_deliver_ack_select_lost    = 5,
    };
    
    //4bit ack source(max source type is 15)
    enum enum_client_msg_deliver_ack_source
    {
        enum_client_msg_deliver_ack_from_sender_edge_server     = 0,
        enum_client_msg_deliver_ack_from_target_edge_server     = 1,
        enum_client_msg_deliver_ack_from_target_object          = 2,
        enum_client_msg_deliver_ack_from_target_group           = 3, //send msg to group/follower-list/friend-list use this ack type to tell client
        enum_client_msg_deliver_ack_from_target_edge_offbox     = 4,
        
        //client know message is SMS or to PSTN server + enum_deliver_ack_from_target_object = determine it is from sms/pstn gateway
        //it is just helpful acktype that reserved for use later
        enum_client_msg_deliver_ack_from_target_pstn_gateway    = 5,
        enum_client_msg_deliver_ack_from_target_sms_gateway     = 6,
        
        //ack from push_gateway not support yet
        enum_client_msg_deliver_ack_from_target_push_gateway    = 7,
    };
    
    //8bit error code(max one is 127)
    enum enum_client_xip_message_ack_reason
    {
        enum_client_xip_message_ack_reason_deliver_successful         =  0, //deliver successful
        enum_client_xip_message_ack_reason_deliver_timeout            = -1, //deliver(include saveto offline box) time out
        enum_client_xip_message_ack_reason_route_notfound             = -2, //target 'route not found(target might just disconnected)
        enum_client_xip_message_ack_reason_target_notfound            = -3, //target user/group not found at server
        enum_client_xip_message_ack_reason_target_is_offline          = -4, //target is offline,online msg forward to offline
        enum_client_xip_message_ack_reason_target_is_deactived        = -5, //target user is deactived
        enum_client_xip_message_ack_reason_target_offbox_full         = -6, //target offline message box is full
        enum_client_xip_message_ack_reason_target_push_token_empty    = -7, //target push token is empty
        enum_client_xip_message_ack_reason_target_not_support_format  = -8, //target is tool old to support format of message
        enum_client_xip_message_ack_reason_target_device_notfound     = -9, //target device not found at server
    };
    
    enum enum_client_packet_flags
    {
        //enum_msg_reliable_type(highest 2bit,total 4 types allow) used for signal and network control ,shared by all users
        enum_client_packet_reliable_type_must      = (3 << 6),  //default,not dropable,signal or message
        enum_client_packet_reliable_type_most      = (2 << 6),  //smallest dropable or Video KeyFrame, and retry best(2 time for rudp, use premium route path)
        enum_client_packet_reliable_type_medium    = (1 << 6),  //samll dropable allow(e.g raw voice), retry 1 more time when loss (pick alternative routing)
        enum_client_packet_reliable_type_low       = (0 << 6),  //big dropable allow(e.g. FEC packet), no retry for packet lost and  just rely on ip like udp
        
        //enum_packet_order_type(1bit,total 2 types allow) express whether need follow order
        enum_client_packet_order_type_must         = (1 << 5),  //default
        enum_client_packet_order_type_free         = (0 << 5),  //when set,allow deliver packet as non-order
        
        //enum_packet_priority_type(2bit,total 4 types allow) express the message priority
        enum_client_packet_priority_type_critical  = (3 << 3), //signal,connect/authen,1-1 online txt msg,server down msg,routing path change
        enum_client_packet_priority_type_flash     = (2 << 3), //group text and offline/control message,voice
        enum_client_packet_priority_type_priority  = (1 << 3), //rpc call,video,stream,voice message,
        enum_client_packet_priority_type_routine   = (0 << 3), //content/file transfer,test,ping,load report
    };
    
    //3bit total allow define 8[0-7] kinds of  message class
    enum enum_client_msg_class
    {
        //total 3bit for detail class
        enum_client_msg_class_device =  0,   //(default)target is device of user,msg stored at device'box if not it present
        enum_client_msg_class_user   =  1,   //target is app user,msg stored at user'box if not it present
        enum_client_msg_class_sms    =  2,   //target is sms'phone  number
        enum_client_msg_class_mms    =  3,   //target is mms'phonen number
        enum_client_msg_class_email  =  4,   //target is email address
        enum_client_msg_class_topic  =  5,   //target is topic/forum or channel that support subscrible
    };
    
    enum enum_client_msg_flag
    {
        enum_client_msg_flag_invalid        = 0x0000,   //Invalid type
        
        /////////////////////////////read-write for client///////////////////////////
        enum_client_msg_flag_auto_push      = 0x0001,   //ask Server automatically push message to offline user by Push Notification
        enum_client_msg_flag_force_Push     = 0x0002,   //Force server to send push notification even the user is online
        enum_client_msg_flag_online_post    = 0x0004,   //ask post to user directly if it is online
        enum_client_msg_flag_ask_ack        = 0x0008,   //ask ack for messsage deliver
        enum_client_msg_flag_storebox       = 0x0010,   //ask store msg to box if dest is offline,as default it stored at device 'box
        
        enum_client_msg_flag_text           = 0x0020,   //indicate this is a text message, this flag is useful to count daily-usage for msg
        //advance use case,support the feature of "destroy after readed"
        enum_client_msg_flag_expiring       = 0x0080,   //message may add timeout control to remove message after certain duration from readed
   
        /////////////////////////////read-only for client//////////////////////////////
        //read only flags when client received one msg, but xclient remove those bits when sending
        enum_client_msg_flag_offline        = 0x0100,   //indicate this message is offline msg or onlined msg
        enum_client_msg_flag_readed         = 0x0200,   //message already been readed by some device of target user
        enum_client_msg_flag_deleted        = 0x0400,   //message already been deleted already by user
        enum_client_msg_flag_xip_target     = 0x0800,   //message send to target xip address instead of userid/groupid(default)
        
        //following are internal useonly,just show the value to client
        //enum_client_msg_flag_user_resolved     = 0x1000,  //user have been resolved through userid,indicate that can not redirect again
        //enum_client_msg_flag_device_resolved   = 0x2000,  //device have been resolved,indicate that can not redirect again
        //enum_client_msg_flag_exclude_xip       = 0x4000,   //internal use only,exclude the second target xip address
        //enum_client_msg_flag_composited        = 0x8000,   //internal use only. it is a composited message that include 2 sepeated address to send
    };
    
    //following msg type defintion is compatbile with PN1
    enum enum_client_msg_type
    {
        enum_client_msg_type_invalid        = 0,
        
        //client/app may define own type between [1,200]=total 200
        enum_client_msg_type_app_start      = 1,
        enum_client_msg_type_app_end        = 200,
        
        //internal/core use only range [201,232]= total 32
        enum_client_msg_type_internal_start = 201,
        enum_client_msg_type_internal_end   = 232,
        
        //[233-253] are availabe for client as well
        enum_client_msg_type_app_extend_start = 233,
        enum_client_msg_type_app_extend_end   = 253,
        
        //exceptionhandle and test message for xclient
        enum_client_msg_type_reserver_start = 254,
        enum_client_msg_type_reserved_end   = 255
    };
    

    /* the defintion of PN1 client' message type  as follow
     typedef enum enumMSGType
     {
     enumMSGType_Invalid             = 0,   // Invalid type
     enumMSGType_Text                = 1,   // text message
     enumMSGType_Image               = 2,   // image message
     enumMSGType_Map                 = 3,   // map message
     enumMSGType_Contact             = 4,   // contact message
     enumMSGType_Video               = 5,   // video message
     enumMSGType_CallInvite          = 6,   // Call message
     enumMSGType_MissedCall          = 7,   // missed call message
     enumMSGType_GroupChatInvite     = 8,   // group caht invite message
     enumMSGType_ActivationInform    = 9,   // send activation notification to followers
     enumMSGType_Voice               = 10,  // voice message
     enumMSGType_RequestFriend       = 11,  // request to be friend
     enumMSGType_ActivationFBInform  = 12,   // send activation notification to dingtone who is facebook
     enumMsgType_CallAnsweredAnotherDevice = 13, // call was answered in another device
     enumMsgType_SystemPush = 14,//system remote push
     
     enumMsgType_VoiceBeep 			= 15,//notification others enter the session as soon as possible ,because a walkie talkie session is ready.
     enumMsgType_VoiceInvite			= 16,//invite to join a walkie talkie
     enumMSGType_TalkingVoice        = 17,//voice message from talk
     
     enumMSGType_MsgReadNotify       = 18,  //notify the sender when read messages
     enumMSGType_GiftSend            = 19,  //gift send notify
     
     enumMSGType_Bonus               = 20, //"Special offer today! Free 100 bonus credits with 500 credits purchase"
     
     enumMSGType_InviteUserToGroup   = 21,  //invite user join a group.
     
     enumMSGType_CallRecordReady     = 22,  //
     enumMSGType_CallRecordDeliverSuccess = 23, //
     enumMSGType_CallRecordWillExpire  = 24, //
     
     enumMSGType_AgreeToFrendRequest = 25, //
     
     enumMSGType_AddToGroupByOwner = 26,// add to group by owner directly.
     enumMSGType_AddToGroupFromMemberInvite = 27,//member invite you to group,and owner approve the request,add you to group.
     enumMSGType_NewCreditsArrived = 28,
     enumMSGType_EarnCreditsByADArrived = 29, // earn credits by AD(video or offerwall)
     
     enumMSGType_Daily_CheckIn_RemindMe = 30,
     enumMSGType_Tips_DoDaily_CheckIn   = 31,
     enumMSGType_CreditExpired = 32,
     enumMSGType_unlimitCallPlanPurcasedSuccess = 33,
     
     enumMSGType_Content             = 50,  // Content message
     enumMSGType_Signling            = 51,  // Signaling messsage
     
     
     //common one of web message ,usally it just show text like upgrade or new feature
     enumMSGType_PrivateNumber_ActiveFromPurchaseCredit = 180, //private number was actived because purchase 200/200 credit
     enumMSGType_BossMsgWithInnerLink = 181,//
     enumMSGType_PrivateNumberExpired = 190,
     enumMSGType_WebMsg_comext        = 198, //content has individual extention
     enumMSGType_BossMsg              = 199, //
     
     
     enumMSGType_Reserved_start      = 200,//dont occupy them, reserved by jucore
     enumMSGType_Reserved_201        = 201,
     enumMSGType_Reserved_202        = 202,
     enumMSGType_Reserved_203        = 203,
     enumMSGType_Reserved_204        = 204,
     enumMSGType_Reserved_205        = 205,
     enumMSGType_Reserved_206        = 206,
     enumMSGType_Reserved_207        = 207,
     enumMSGType_Reserved_208        = 208,
     enumMSGType_Reserved_209        = 209,
     enumMSGType_Reserved_210        = 210,
     enumMSGType_Reserved_232        = 232,
     
     enumMsgType_Web_Notification    = 246, // the notificaiton message from web server
     enumMSGType_Group_SMS           = 247, //ref all the group sms related message
     enumMSGType_Inbound_SMS_Image   = 248, //Kun: for MMS
     enumMSGType_Voice_Mail_Message_Notify = 249,//for voicemail, got an new voicemail
     enumMSGType_Canceled_Call_Notify  = 250,//for inbound cancel call
     enumMSGType_Missed_Call_Notify  = 251,//for inbound miss call
     enumMSGType_Inbound_Call        = 252, //for inbound inbound call
     enumMSGType_Inbound_SMS        = 253, //for inbound SMS
     
     enumMSGType_Reserved_end        = 254, //dont occupy them, reserved by jucore
     enumMSGType_test                = 255  //dont showup ,just used to test purpose ,e.g. test round-trip time
     }E_MSGType;
     */
    struct xMessage
    {
    public:
        xMessage()
        {
            msgId = 0;
            msgType = enum_client_msg_type_invalid;
            msgSubType = 0;
        }
    public:
        uint64_t     msgId;            //Message Id, unique id  when combine the sender ID.
        uint8_t      msgType;          //refer enum_xmsg_type
        uint8_t      msgSubType;       //defined/customized by application,0 means invalid
        
        std::string  msg_utf8_content;
        std::string  msg_utf8_meta;
    };
    
    struct xPushconfig
    {
    public:
        xPushconfig(){pushBadgeFlags = 0; pushNosound = 0;};
    public:
        std::string  pushDisplayName; //recommend to have, if not present server use it' userid for remote push at receiver side
        std::string  pushonlyText;    //optional(UTF8), if not present(empty) server use msgContent for push
        std::string  pushonlyMeta;    //optional(UTF8), if not present(empty) server use msgMeta for push
        std::string  pushTemplate;    //optional(UTF8), if present,server use it as push template when dont conflict with push_setting from user, in other words:  first check whether user has customized template from push setting ,then check template from DtMessageExtend.
        std::string  pushRingtone;    //optional(UTF8),customized ringtone when push
        int32_t      pushBadgeFlags;  //optional, apple remote push only
        int32_t      pushNosound;     //determine whether need sound notification
    };
    
    enum enum_presence_status
    {
        enum_presence_status_offline    = 1,
        enum_presence_status_online		= 2,
        enum_presence_status_busy       = 3,
        enum_presence_status_away       = 4,
        enum_presence_status_idle       = 5,
        enum_presence_status_hidden     = 6
    };
    
    //allow client pass addtional params to web-server when login
    struct xlogin_client_params
    {
        float                   gps_Latitude;                 //client gps location, pass 0 if dont know
        float                   gps_Longitude;                //client gps location, pass 0 if dont know
        
        std::string             client_timezone;              //client timezone
        std::string             client_language_isocode;      //convert client 'language to iso code
        std::string             clientInfo;                   //core layer do urlencode for client
    };
    
    /* basic rule for api and callback
     1.) every callback is trigger from underly_thread(not main-thread), upper layer dont do heavy job on callback otherwise it may reduce the performance
     2.) the return type of every callback function is bool, upper layer should return true,if it already processed this callback and dont want corelayer do further handle.
     3.) most calblack carry param of "int result","int error_code" or "int result_code" which follow definition of enum_xclient_error_code
          simle code: caller just need treat as error when (result_code != enum_xclient_code_successful)
     4.) RPC ,RSC  and other API return as "int64_t" that is internal task_id,the task_id will be carrry back at related callback api.so upper layer may different it from any api call.
     5.) RPC/RSC and other api return http_result_code when get callback,caller need treat as error when (result_code != 200)
            if http_result_code < 0, refer enum_xclient_error_code,
            if http_result_code != 200, refer http_fail_reason param that has detail and refer enum_http_status_error_code
          but note: the reponse the server executed  may include real error code for this api call
     6.) most api allow upperlayer pass in "uint64_t client_cookie" that will be pass back at related callback ,upper layeer  may store  any (even secret) information
     */
    
    
    //callback for IxbaseClient_t
    class IxbaseClientCallback_t
    {
    protected:
        IxbaseClientCallback_t(){};
        virtual ~IxbaseClientCallback_t(){};
    private:
        IxbaseClientCallback_t(const IxbaseClientCallback_t &);
        IxbaseClientCallback_t & operator = (const IxbaseClientCallback_t &);
    public: //reference control function provide by application
        virtual  int    add_ref() = 0;    //add 1 reference,and return final reference count
        virtual  int    releae_ref() = 0; //release 1 reference,delete/release resource when reach 0
    public:
        virtual bool    on_connect(const int result,const uint64_t public_xip_address,const std::string public_ipv4_address,const std::string private_ipv4_address,const uint64_t client_cookie,const std::string & extra_json_info) = 0;
        
        virtual bool    on_disconnect(const int error_code,const std::string error_reason) = 0;
        
    public: //the callback of RSC(Remote Service Call) Call
        //if http_result_code < 0, refer enum_xclient_error_code,
        //if http_result_code != 200, refer http_fail_reason param that has detail and refer enum_http_status_error_code
        //Note: http_result_code carry the reponse the server executed, it may include real error code for this api call
        //to easly handle rsc all, original_request_param also pass back the original information(apiname,request,cookie)
        virtual bool    on_rsc_call_response(const int64_t rsc_task_id,const int http_result_code,const std::string reason,const std::string rsc_response,const std::string original_request_resource,const std::string original_request_apiname,const std::string & original_request_param,const uint64_t original_client_cookie) = 0;
    };
    
    //Note:  RSC(Remote Service Call, special RPC)  usally to access Edge ' api
    //but    RPC(Remote Process Call,Restful API) usally to access WebServer ' api
    //Note:  any RSC/RPC api ask caller do urlencoded for string before call api
    class IxbaseClient_t
    {
    protected:
        IxbaseClient_t();
        virtual ~IxbaseClient_t();
    private:
        IxbaseClient_t(const IxbaseClient_t &);
        IxbaseClient_t & operator = (const IxbaseClient_t &);
    public: 
        //remote_addr format must be like "tcp://ip:port" or "sctp://ip:port". And connections can be combined under same IP address with different port and protocol,so that one client may have multiple stream/channel connections by which xclient may pick best connection according network quality,or reliable/order property. e.g.  connect(tcp://1.2.3.4:443) + connect(rudp://1.2.3.4:8100)
        //return native socket handle if sucessful ,otherwise return error code(< 0, refer the result refer the enum_xclient_error_code,especial for enum_xclient_error_code_connect_***)
        //each connection must carry valid userid,devieid and dev_token by which server may authenticate whether allow to this service
        //max_auth_wait_seconds decide  how long the connection can wait before the authentication to be finish,0 means default value(60s).
        //max_auth_wait_seconds' main purpose to let client start using app after connect but before authentication,then server decide later to kick connection or not. Actually our connetion layer also has underly authentication mechanisam so it is ok to use right after connection
        //client_params is a json data that carry on "country code,device name etc", client_cookie is just reserved for future
        virtual int     connect(const std::string & remote_addr,const uint64_t client_cookie,const uint64_t user_id,const std::string device_id,const std::string device_token,const std::string client_params,int max_auth_wait_seconds = 60) = 0;
        
        //error code:0 means client want to quit app normally without error
        virtual int     disconnect(const int reason_code, const std::string reason_description) = 0;
        
    public: //RSC(Remote Service Call, one kind of RPC) sent to connected Edge-server,allow call after connected(that already include the embeded authentication).return unique task_id for rtc_call if > 0, otherwise it means error code
        //Note: the total bytes by counting every parameter must less than 65535,otherwise it reject to service
        //Use case:  Edge-server API ,or  RPC call that edge need know api then proxy to web-server
        //core layer do urlencode for api_params. but resource and apiname must be english chars onlyl
        virtual int64_t fire_rsc_call(const std::string resource,const std::string & api_name,const std::string & api_params,const uint64_t clientCookie,uint16_t after_ms_timeout = 10000) = 0;
        
        //caller should call cancel_rsc_call firt when try to call the same rsc call
        virtual bool    cancel_rsc_call(const int64_t rsc_task_id) = 0; //rsc_task_id got from the above fire_rsc_call api
        
    public: //secure rpc/rsc call
        //return " userId="xxx” & deviceId="xxx” & sessionId=“xxx” & keyId=“xxx”  & expire =“xxx” & TrackCode =“xxx” & signature=“xxx”& body=“xxx” "
        //expire: The time when the signature expires, specified as the number of seconds since the epoch (00:00:00 UTC on January 1, 1970). A request received after this time (according to the server) will be rejected; sample value. 1141889120
        virtual std::string encode_secure_request(const uint64_t userid,const std::string & deviceid,const std::string & token,const uint32_t key_id,const std::string & access_key,const std::string & api_name,const std::string & url_params,const std::string & expire,const uint64_t track_code) = 0;
        
        //decode request from input_secure_url_params -> out_raw_url_params
        virtual int32_t     decode_secure_request(const uint64_t userid,const std::string & deviceid,const std::string & token,const uint32_t key_id,const std::string & access_key,const std::string & api_name,const std::string & input_secure_url_params,std::string & out_raw_url_params) = 0;
        
        ////////////////////////////response handle ////////////////////////////////////////
        virtual std::string  encode_secure_response(const uint64_t userid,const std::string & deviceid,const std::string & token,const uint32_t key_id,const std::string & access_key,const std::string & api_name,const int32_t api_errcode,const std::string & raw_api_response,const std::string & expire,const uint64_t track_code) = 0;
        
        //input input_encrypted_response ->out_xxx;return error code(refer enum_error_code)
        virtual int32_t      decode_secure_response(const uint64_t userid,const std::string & deviceid,const std::string & token,const uint32_t key_id,const std::string & access_key,std::string & out_api_name,int & out_api_errcode,std::string & out_api_response,std::string & out_expire,uint64_t & out_track_code,const std::string & input_encrypted_response) = 0;
        
    public://helper function
        int    get_user_type(const uint64_t user_or_group_id); //may determine whether it is user/group or other
        int    get_user_flags(const uint64_t user_id);         //flags for user
        //return the cominations of enum_xgroup_flag,client may test by (flags & enum_xgroup_flag)
        int    get_group_flags(const uint64_t group_id);
        bool   is_group(const uint64_t user_or_group_id);
        int    get_website_id(const uint64_t user_or_group_id);//which website(zone) mange this user/group
        
    public: //helper function to validate whether it is a valid address,return 0 if pass test ,otherwise return cetain error code
        static  int     validate_address(const std::string & remote_address);
    };
    
    //callback for IxtunnelClient_t
    class IxtunnelClientCallback_t : public IxbaseClientCallback_t
    {
    protected:
        IxtunnelClientCallback_t(){};
        virtual ~IxtunnelClientCallback_t(){};
    private:
        IxtunnelClientCallback_t(const IxtunnelClientCallback_t &);
        IxtunnelClientCallback_t & operator = (const IxtunnelClientCallback_t &);
    public:
        virtual int64_t on_packet_recv(const uint8_t* ip_packet_data,const int32_t ip_packet_len) = 0;
    public:
        //note: max_session_speed,max_session_traffic_KB,max_session_time,max_day_traffic_KB and max_day_time might be <= 0,which means dont have limitation or restriction for this item
        //session_traffic_KB present the total taffic used for this session and session_duration_seconds present how long since this session start
        //term of "out" is about "send data out to remote website from client", term of "in" is about "recv data from remote website to client"
        //session_in_speed_Kbs and session_out_speed_Kbs present the recent speed(for recent 5M or 1 minutes),instead of average speed of session
        //limit_speed_code indicate  0 : default limit(basic as  2Mbps, premium as 8Mbps), > 0 for other case 
        //alert_msg_code ask client show message to user, 8036 : limit speed notifiication, 0 : none msg
        virtual void    on_session_update(const std::string device_id,const std::string session_id,const int64_t total_balance_KB,const int32_t session_duration_seconds,const int32_t session_out_traffic_KB,const int32_t session_in_traffic_KB,const int32_t session_out_speed_KBS,const int32_t session_in_speed_KBS,const int32_t max_session_duration_seconds,const int32_t max_session_inout_traffic_KB,const int32_t max_session_upspeed_kbps,const int32_t max_session_downspeed_kbps,const int32_t limit_speed_code,const int32_t alert_msg_code) = 0;
    };
    
    //dedicated xtunnel service
    class IxtunnelClient_t : public IxbaseClient_t
    {
    protected:
        IxtunnelClient_t();
        virtual ~IxtunnelClient_t();
    private:
        IxtunnelClient_t(const IxtunnelClient_t &);
        IxtunnelClient_t & operator = (const IxtunnelClient_t &);
    public:
        //packet_deliver_flags is the combination by enum_client_packet_flags,the default is enum_client_packet_reliable_type_low | enum_client_packet_order_type_free
        //return < 0 if have error 
        virtual int64_t  send_packet(const uint8_t* ip_packet_data,const int32_t ip_packet_len,int packet_deliver_flags) = 0;
    };
    
    //callback for IxrpcClient_t
    class IxrpcClientCallback_t : public IxbaseClientCallback_t
    {
    protected:
        IxrpcClientCallback_t(){};
        virtual ~IxrpcClientCallback_t(){};
    private:
        IxrpcClientCallback_t(const IxrpcClientCallback_t &);
        IxrpcClientCallback_t & operator = (const IxrpcClientCallback_t &);
    public: //the callback of RPC(Remote Process Call)
        //if http_result_code < 0, refer enum_xclient_error_code,
        //if http_result_code != 200, refer http_fail_reason param that has detail and refer enum_http_status_error_code
        //rpc_response carry the reponse the server executed
        //to easly handle rpc all, original_request_param also pass back the original information(apiname,request,cookie)
        virtual bool  on_rpc_call_response(const int64_t rpc_task_id,int http_result_code,const std::string http_fail_reason,const std::string rpc_response,const std::string original_request_target,const std::string original_request_resource,const std::string original_request_apiname,const std::string & original_request_param,const uint64_t original_client_cookie) = 0;
    };
    
    //RPC(Remote Process Call, aka Restful Webapi) to WebServer
    //Note: edge just do proxy for rpc api,dont decode to see what is
    class IxrpcClient_t : public IxbaseClient_t
    {
    protected:
        IxrpcClient_t();
        virtual ~IxrpcClient_t();
    private:
        IxrpcClient_t(const IxrpcClient_t &);
        IxrpcClient_t & operator = (const IxrpcClient_t &);
    public: //RPC(Remote Process Call,aka web-api) target DataCentor or Web-Server through Edge-server, just allow call after logined successful, core layer do urlencode for api_params and api_body as well,but resource and apiname must be english chars onlyl
        //return unique task_id for rpc_call if > 0, otherwise it means error code
        //Note: the total bytes by counting every parameter must less than 65535,otherwise it reject to service
        //http post: domain_or_ip/resource/api_name? urlencoded_api_params + post urlencoded_api_body
        //proxy_server could be cloudfront server(public without signature) or other http/http transparent proxy server
        //leave http_headers_keyvalues as empty if target is our own server,
        //either api_params or api_body could be empty
        virtual int64_t  fire_rpc_call(const std::string & domain_or_ip,const std::string & resource,const std::string & api_name,const std::string & api_params,const std::string & api_body,const std::string & http_headers_keyvalues,const std::string & proxy_server,const uint64_t clientCookie,uint16_t after_ms_timeout = 15000) = 0;
        
        //return unique task_id for rpc_call if > 0, otherwise it means error code
        //the only difference thans above is : using user' web-site ip(according the user id) as the default target
        virtual int64_t  fire_rpc_call(const std::string & resource,const std::string & api_name,const std::string & api_params,const std::string & api_body,const std::string & http_headers_keyvalues,const std::string & proxy_server,const uint64_t clientCookie,uint16_t after_ms_timeout = 15000) = 0;
        
        //caller must call cancel_rpc_call firt before try to call the same rpc call
        virtual bool    cancel_rpc_call(const int64_t rpc_task_id) = 0; //rpc_task_id got from the above fire_rpc_call api

    };
    
    //callback of IxuserClient_t(user service)
    class IxuserClientCallback_t : public IxbaseClientCallback_t
    {
    protected:
        IxuserClientCallback_t(){};
        virtual ~IxuserClientCallback_t(){};
    private:
        IxuserClientCallback_t(const IxuserClientCallback_t &);
        IxuserClientCallback_t & operator = (const IxuserClientCallback_t &);
    public: //callback login,upperlayer shoud know:every callback is triggerd the underly dedicated thread(not main-thread)
        //server send ack to client and let it know server already received login request and processing
        //server_timestamp_ms is the current server-time at ms
        virtual bool  on_client_login_ack(const int result,const uint64_t client_cookie,const uint64_t server_timestamp_ms,const std::string & login_raw_response) = 0;
        
        //server send final response(from webserver) to client.  server_timestamp_ms is the current server-time at ms
        virtual bool  on_client_login_response(const int result,const uint64_t client_cookie,const uint64_t server_timestamp_ms,const std::string & login_raw_response) = 0;
        
        //Note: if web_site_url is valid,client need use it instead of construct by client
        //Note: client need carry edge_site_id/subsiteid/nodeid to web-server who need embeded it into user_id when do register and activation by following format:
        //PN2 user:   [5bit:siteid: 5bit:nodeid]   8(web siteId) 4(userType) 4(subsiteid)  10(appdomainId)             28 (index)
        //result refer enum_xclient_error_code; max web_site_id is 255(8bit),max edge_site_id is 63(6bit)
        virtual bool  on_client_query_register(const int result,const int web_site_id,const std::string web_site_url,const int edge_site_id,const int edge_subsite_id,const int edge_node_id,const uint64_t client_cookie) = 0;
        
    public:  //callback for group operation,fail when if result != 0
        virtual bool  on_create_group(const int result,const uint64_t group_id, const std::string group_token,const std::string group_name,const uint32_t group_version,const int64_t task_id,const uint64_t client_cookie) = 0;        //paired with create_group
        
        virtual bool  on_join_group(const int result,const uint64_t group_id,const uint32_t group_version,const int64_t task_id,const uint64_t client_cookie) = 0; //paired with join_group
        
        virtual bool  on_leave_group(const int result,const uint64_t group_id,const uint32_t group_version,const int64_t task_id,const uint64_t client_cookie) = 0;//paired with leave_group
       
        virtual bool  on_destroy_group(const int result,const uint64_t group_id,const int64_t task_id,const uint64_t client_cookie) = 0;//paired with destroy_group
        
        //response must be json format and is the raw content from server
        virtual bool  on_query_group(const int result,const uint64_t group_id,const uint32_t group_version,const std::string & response,const int64_t task_id,const uint64_t client_cookie) = 0;//paired with query_group api
        
        //response must be json format and is the raw content from server
        virtual bool  on_update_group(const int result,const uint64_t group_id,const uint32_t group_version,const std::string & response,const int64_t task_id,const uint64_t client_cookie) = 0;//paired with update_group api
        
        virtual bool  on_update_group_member(const int result,const uint64_t group_id,const uint32_t group_version,const std::string member_id,const int64_t task_id,const uint64_t client_cookie) = 0;//paired with update_group_member
        
    public: //callback of any user'message api
        //msg_timestamp indicate when the message is send out through server at UTC/GMT time(ms)
        //msg_flags is the combined value by enum_xmsg_flag
        virtual bool  on_msg_recv(const uint64_t from_user,const uint64_t to_user,xMessage & msg,const uint64_t msg_timestamp_ms,uint32_t msg_flags) = 0;
        
        //if client specified the enum_xmsg_flag_ask_ack flag, may get this callback when message lost or delivered
        //ack_device_index is just hint and just valid when ack_source is enum_client_msg_deliver_ack_from_target_object or enum_client_msg_deliver_ack_from_target_edge_offbox
        virtual bool  on_msg_deliver_notify(const uint64_t from_user,const uint64_t to_user,const uint64_t msg_id,enum_client_msg_deliver_ack_type ack_type,enum_client_msg_deliver_ack_source ack_source, enum_client_xip_message_ack_reason ack_reason,const int ack_device_index) = 0;

    };
    
    //notification_setting dont have any thread_safe protect since it is just a helper,client respond for it.
    struct notification_setting
    {
    public:
        notification_setting();
    public:
        std::string  get_notification() const {return push_setting_json;}
    public: //return false if change fail
        bool    on();           //enable  notification,default on
        bool    off();          //disable notification
        //GMT 24 hour format: hours(8bit)::minutes(8bit), whole day is 00:00 ---24:00
        //e.g. 22:00 ---> 08:00 means noalert from evening(22:00) to next morning(08:00)
        //if both no_notification_start_time and no_notification_end_time is 0 on(xx,xx) has same funtion of off();
        bool    off(const uint16_t no_notification_start_time,const uint16_t no_notification_end_time);
        
        bool    sound_on();     //default it is on,enable sound of notification if notification allow(on)
        bool    sound_off();    //disable sound of notification
        //GMT 24 hour format: hours(8bit)::minutes(8bit), whole day is 00:00 ---24:00
        //e.g. 22:00 ---> 08:00 means noalert from evening(22:00) to next morning(08:00)
        //if both no_sound_start_time and no_sound_end_time is 0 sound_on(xx,xx) has same funtion of sound_off();
        bool    sound_off(const uint16_t no_sound_start_time,const uint16_t no_sound_end_time);
        
        bool    badge_on();     //specific for push, decide whether show un_read_msg flag on the app'icon,default is on
        bool    badge_off();    //disable badge show
        
        bool    preview_on();   //allow iOS and Android show up the message 'preview content,default it is on
        bool    preview_off();  //disable preview
    protected:
        std::string  push_setting_json;
    };
    
    //total 4bit only
    enum enum_xgroup_flag
    {
        //1bit for public or need autherized
        enum_xgroup_flag_private    = 0 << 0, //member or owner control group(default) control group(default)
        enum_xgroup_flag_public     = 1 << 0, //everyone allow to join,public dose not mean un-secure
        //2bit for storage control
        enum_xgroup_flag_user_box   = 0 << 1, //message stored at each individual user'box(default type) at storage server of Edge

        enum_xgroup_flag_group_box  = 1 << 1, //message stored at group'box shared by every member   at storage server of Edge
        enum_xgroup_flag_web_box    = 2 << 1, //message stored at group'box of Web Server
        
        //another bit reserved for future
    };
    
    //if member_name,member_property or member_filter is empty, server just ignore it and not update related record
    struct xgroup_member
    {
    public:
        //member_id could be userid,email_address,phone number,format as "user.xxx" "email.xxx", "phone.xxx"
        explicit xgroup_member(const uint64_t user_id);
        explicit xgroup_member(const std::string this_member_id){member_id = this_member_id;}
    private:
        xgroup_member(){};
    public:
        bool set_notification_filter(const notification_setting & notification_setting); //add key(notification) and value(settting) to member_filter
        
        std::string get_member_id() const {return member_id;}
        std::string get_member_filter() const {return member_filter;}
        
    public: //ignore member_name if member_name is empty,same for member_property or member_filter
        std::string member_name;            //show name in the gruop/channel,do urlencode for client
        std::string member_property;        //client define and use , server(web) just store. but recommend use the size under 8KB
    protected:
        std::string member_filter;          //must key-value under json format,english txt only so no-need urlencode
        std::string member_id;              //must be valid ,otherwise server dont handle it
    };
    
    //manager user and presence,related message
    class IxuserClient_t : public IxbaseClient_t
    {
    protected:
        IxuserClient_t();
        virtual ~IxuserClient_t();
    private:
        IxuserClient_t(const IxuserClient_t &);
        IxuserClient_t & operator = (const IxuserClient_t &);
    public: //register,login; multiple thread safe
        //after connected successfuly(ok before login), client may call query_register,the callback is on_client_query_register
        //return unique query_task_id if > 0, otherwise it means error code
        //iso_country_code is follow the standard of iso country code,e.g. 86 for China, 1 for US
        //country_area_code is optional(pass 0 when dont know)
        virtual int64_t query_register(const int32_t iso_country_code,const int32_t country_area_code,uint64_t client_cookie,const uint16_t timeout_ms = 10000) = 0;
        
        //if call login api before connect, then connect will do login as well which may reduce time,key_id is unique for each app
        //Note:device_index indicate that is which device under user, client may get it when activation
        //Note: iOS allow secondary pushtoken based on Pushkit,for android only one pushtoken allow
        //addtional_params allow client pass addtional params to web-server
        //if deviceid is empty,which means do user level login by userid and login_token,otherwise it is device'level
        virtual int  login(const uint64_t userid,const std::string deviceid,const int device_index,const std::string login_token,const std::string pushtoken,const int pushtoken_ver,const std::string pushtoken2,const int pushtoken2_ver,xlogin_client_params * addtional_params,const uint64_t client_cookie,const uint16_t timeout_ms = 15000,bool request_all_offline_msg = false) = 0;
        
        virtual int  logoff() = 0;
        
    public: //group api(thread_safe api).note: edge may do pre-authentication with "group_id" and "group_token" before web-server do fully
        //the owner of group is who create,once create server may return paired group_id and group_token;group_flags refer enum_xgroup_flag
        //group_property can not over 512 bytes ,otherwise return enum_client_error_code_too_big
        virtual int64_t   create_group(const std::string group_name,const uint16_t group_flags,const std::string group_property,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0;
        //since create_group is the first and most important step for group,so members is limited under <=256 to has smaller packet with better performance,otherwise return enum_client_error_code_too_big
        virtual int64_t   create_group(const std::string group_name,const uint16_t group_flags,const std::string group_property,std::vector<xgroup_member> & members,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0;
        
        //member_id could be userid,email_address,phone number,format as "user.xxx" "email.xxx", "phone.xxx".
        virtual int64_t   join_group(const uint64_t group_id,const std::string group_token,xgroup_member & member,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0;
        virtual int64_t   join_group(const uint64_t group_id,const std::string group_token,std::vector<xgroup_member> & members,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0;
        
        //api to allow drop member from this group,client do logic decide wether to allow remove without owner aprove or not
        virtual int64_t   leave_group(const uint64_t group_id,const std::string group_token,const std::string member_id,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0;
        virtual int64_t   leave_group(const uint64_t group_id,const std::string group_token,std::vector<std::string> member_ids,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0;
        
        //depends group type, some group may only allow the original owner of group to destroy it,but other may just ask have validate token
        virtual int64_t   destroy_group(const uint64_t group_id,const std::string group_token,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0;
        
        virtual int64_t   update_group_member(const uint64_t group_id,const std::string group_token,xgroup_member & member,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0;
        
        //client and webserver know how to use, edge just bypass. core layer do urlencode for api call
        virtual int64_t   query_group(const uint64_t group_id,const std::string group_token,const std::string query_action,const std::string query_params,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0; //read-only
        virtual int64_t   update_group(const uint64_t group_id,const std::string group_token,const std::string update_action,const std::string update_params,const uint64_t client_cookie,uint16_t after_ms_timeout = 15000) = 0; //write-only
        
    public: //below all message api are thread_safe
        //xclient using time(clock) +  device_index + msg_class  to calcualte unique message id,
        virtual uint64_t    alloc_message_id(enum_client_msg_class msg_class,bool is_group_msg) = 0;
        //here directly open code for client who may know the meaning and easy to debug
        enum_client_msg_class   get_message_class(const uint64_t msg_id);
        bool                    is_group_message(const uint64_t msg_id);
    public: //device and user message ap; msg_flags is the combined value by enum_xmsg_flag .target_userid could be groupid or single user id
        //userservice alloc message for msg.msgId if it is 0. and client may alloc message by alloc_message_id first then call send_msg_to_xxx,however send_msg_to_xx fail if enum_msg_class not match
        
        //send_msg_to_device store the offline message at device'box that dedicated for each device
        //send message to every device under user if device_id is empty;otherwise just for specified device
        //msg must be enum_msg_class_device
        //and most case target_user_token is empty except sendmsg to some special user that ask authentication
        virtual int  send_msg_to_device(const uint64_t target_user_id,const std::string target_user_token,const std::string target_device_id,xMessage & msg,const uint32_t msg_flags,xPushconfig * config = NULL) = 0;
  
        //msg must be enum_msg_class_user
        //send_msg_to_user store and manage message at user'box that shared by every device, it like email across multiple devices
        //note: some special user(like SMS gateway 'userid may ask authentication to send SMS to them) which ask target_user_token must be valid,otherwise user_token could be empty
        virtual int  send_msg_to_user(const uint64_t target_user_id,const std::string target_user_token,xMessage & msg,const uint32_t msg_flags,xPushconfig * config = NULL) = 0;
 
        //ask download all offline messages for both current device and current user
        //request all of archived messages if from_gmt_seconds is 0
        //from_gmt_seconds could be standard GMT secons(seconds thans 1970/01/01/00:00) or the last received message'timstamp
        //just query the amount of messages that match conditions if msg_count_per_page is 0
        virtual int  request_offline_messages(const uint64_t from_gmt_seconds,const uint16_t msg_count_per_page = 128) = 0;
        
    public: //group message api
        
        //group_verion_code help to server do group'member syncronize cached group'information with db by comparing which one is most recent,client may get it from query_group or other api, it also may pass 0 if client dont know it or not sure
        virtual int  send_msg_to_group(const uint64_t group_id,const std::string group_token,const uint32_t group_verion_code,xMessage & msg,const uint32_t msg_flags,xPushconfig * config = NULL) = 0;
 
        //for enum_xgroup_flag_group_box, must using dedicated api(request_group_messages) to download message of group
        //last_msg_timestamp decide where to start send stored message of group back, msg_count_per_page decide max messages count for reach time request and send back
        //request all of archived messages if from_gmt_seconds is 0
        //from_gmt_seconds could be standard GMT secons(seconds thans 1970/01/01/00:00) or the last received message'timstamp
        //just query the amount of messages that match conditions if msg_count_per_page is 0
        //api return one task_id to let client identify the related callback
        virtual int64_t request_group_messages(const uint64_t group_id,const std::string group_token,const uint64_t from_gmt_seconds,const uint16_t msg_count_per_page = 128) = 0;
        
    public://sms/mms message api
        //msg must be enum_msg_class_sms
        //virtual int  send_msg_to_sms(const std::string target_phone_number,xMessage & msg,const uint32_t msg_flags) = 0;
        //msg must be enum_msg_class_mms and msg.metadata include the attachment address
        //virtual int  send_msg_to_mms(const std::string target_phone_number,xMessage & msg,const uint32_t msg_flags) = 0;

    public: //email messag api
        //msg must be enum_msg_class_email,msg.metadata include the attachment address
        //virtual int  send_msg_to_email(const std::string target_phone_number,xMessage & msg,const uint32_t msg_flags) = 0;
    };
    
    //Juxclient_t is the entry
    class Juxclient_t
    {
    public:
        //note: xclient dont provide destory api,which means client need keep this instance under whole app'lifetime
        //right now network_id must be enum_network_id_pn2  = 3
        //app_version is the client 'app version by following format("major(8bit).middle(8bit).minor(8bit)");
        //app_boundle_id is the app ' id assigned from apple/google app store
        //app_binary_signature is the signature for app binary(optional)
        //auth_key_id is the authentication AES KeyID and auth_aes_key is the paired Authentication AES Key
        static Juxclient_t* instance(const int network_id,const std::string app_domain,const std::string app_id,const std::string app_version,const std::string app_boundle_id,const std::string app_binary_signature,const uint32_t auth_key_id,const std::string auth_aes_key);
    protected:
        Juxclient_t();
        virtual ~Juxclient_t();
    private:
        Juxclient_t(const Juxclient_t &);
        Juxclient_t & operator = (const Juxclient_t &);

    public:
        //wait_timeout_ms decide what is max time wait before shutdown complete,pass 0 will return immediately
        //call it before app quit (optionaly)
        virtual bool                shutdown(const int32_t wait_timeout_ms) = 0;
        
    public: //dedicated user service
        virtual IxuserClient_t*     create_xuser_service(IxuserClientCallback_t * callback_ptr,void* callback_cookie) = 0;
        virtual bool                destory_xuser_service(IxuserClient_t* & xuser_client_ptr) = 0;
        
    public: //dedicated xrpc service
        virtual IxrpcClient_t*      create_xrpc_service(IxrpcClientCallback_t * callback_ptr,void* callback_cookie) = 0;
        virtual bool                destory_xrpc_service(IxrpcClient_t* & xrpc_client_ptr) = 0;
        
    public: //dedicated xtunnel service
        virtual IxtunnelClient_t*   create_xtunnel_service(IxtunnelClientCallback_t * callback_ptr,void* callback_cookie) = 0;
        virtual bool                destory_xtunnel_service(IxtunnelClient_t* & xtunnel_client_ptr) = 0;
    };
}; //end of namesapce of Jeesu

extern "C"
{
    typedef enum tag_enum_client_log_level
    {
        enum_client_log_level_debug    = 0,
        enum_client_log_level_info     = 1,
        enum_client_log_level_key_info = 2,
        enum_client_log_level_warn     = 3,
        enum_client_log_level_error    = 4,
    }enum_client_log_level;
#ifdef DEBUG
    #define jux_dbg(...)         log_client(enum_client_log_level_debug,__VA_ARGS__)
    #define jux_info(...)        log_client(enum_client_log_level_info,__VA_ARGS__)
    #define jux_kinfo(...)       log_client(enum_client_log_level_key_info,__VA_ARGS__)
    #define jux_warn(...)        log_client(enum_client_log_level_warn,__VA_ARGS__)
    #define jux_error(...)       log_client(enum_client_log_level_error,__VA_ARGS__)
#else
    #define jux_dbg(...)         void(0)
    #define jux_info(...)        log_client(enum_client_log_level_info,__VA_ARGS__)
    #define jux_kinfo(...)       log_client(enum_client_log_level_key_info,__VA_ARGS__)
    #define jux_warn(...)        log_client(enum_client_log_level_warn,__VA_ARGS__)
    #define jux_error(...)       log_client(enum_client_log_level_error,__VA_ARGS__)
#endif
    
    void  set_client_log_level(int new_level); //refer enum_client_log_level_xxx
    void  set_client_log_file(const char * new_log_file_name,int is_allow_rotate);//is_allow_rotate decide whether allow rotate log files
    void  log_client(int level,const char* msg, ...);
    void  flush_log();  //just call when really need because it may has performance lost
    void  dump_stack(); //dump current call-stack,and write to trace file

    //note: xclient dont provide destory api,which means client need keep this instance under the whole app'lifetime
    //right now network_id must be enum_network_id_pn2  = 3
    //auth_key_id is the authentication AES KeyID and auth_aes_key is the paired Authentication AES Key
    Jeesu::Juxclient_t* create_xclient_instance(int network_id,const char* app_domain,const char* app_id,const char* app_version,const char* app_boundle_id,const char* app_binary_signature,const uint32_t auth_key_id,const char* auth_aes_key);
};
#endif /* defined(__Juxclient_h__) */
