// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <functional>

#include "xrefcount.h"
#include "xvevent.h"
#include "xint.h"
#include "xmem.h"

namespace top
{
    namespace base
    {
        class xcall_t;
        class xpacket_t;
        class xmailbox_t;
        class xdatabox_t;
        class xiothread_t;
        class xiosignaler_t;
        class xcontext_t;
    
        //xobject_t require to create object by new ,and destroy by release
        enum enum_xobject_type
        {
            //application may use types of [1,1280]
            enum_xobject_type_max       = 1279,
            
            enum_xobject_type_consensus_max = 1152,
            enum_xobject_type_consensus_min = 1025,
            
            enum_xobject_type_data_max  = 356,
            enum_xobject_type_data_min  = 100,
            
            enum_xobject_type_xcons_max = 99, //for xconsensus
            enum_xobject_type_xcons_min = 65, //for xconsensus
            enum_xobject_type_system_contract = 64,
            
            enum_xobject_app_type_undefine = 1,
            
            //xbase and core modules reserved types of [0,-32767]
            enum_xobject_type_base_max  =  0,  //after that is open for application
            
            enum_xobject_type_base      =  0,
            enum_xobject_type_object    =  0,
            enum_xobject_type_iobject   = -1,
            enum_xobject_type_iohandle  = -2,
            enum_xobject_type_signaler  = -3,
            enum_xobject_type_mailbox   = -4,
            enum_xobject_type_databox   = -5,
            enum_xobject_type_timer     = -6,
            enum_xobject_type_thread    = -7,
            enum_xobject_type_woker     = -8,  //worker thread object
            enum_xobject_type_vevent    = -9,  //general event
            enum_xobject_type_ionode    = -10, //general io-node
            enum_xobject_type_dataunit  = -11, //general data unit
            enum_xobject_type_datapdu   = -12, //general data pdu/msg
            enum_xobject_type_dataobj   = -13, //general data object
            enum_xobject_type_endpoint  = -14, //general endpoint
            enum_xobject_type_valueobj  = -15, //general property/value
            enum_xobject_type_exe_unit  = -16, //general execution object that support 'execute(xvmethod_t & op)' or script

            enum_xobject_type_socket    = -23, //xsocket object
            enum_xobject_type_connection= -24, //xconnection object
            enum_xobject_type_node      = -25, //xnode_t object
            enum_xobject_type_service   = -27, //service

            enum_xobject_type_vplugin    = -28, //xvplugin_t
            enum_xobject_type_vaccount   = -29, //xvaccount_t
            enum_xobject_type_vtable     = -30, //xvtable_t
            enum_xobject_type_vbook      = -31, //xvbook_t
            enum_xobject_type_vledger    = -32, //xvledger_t
            enum_xobject_type_vchain     = -33, //xvchain_t
            enum_xobject_type_vxdbstore  = -34, //xvdbstore_t
            enum_xobject_type_vblockstore= -35, //manage vblock of db/disk
            enum_xobject_type_vstatestore= -36, //state store
            enum_xobject_type_vtxstore   = -37, //xvtxstore_t
            enum_xobject_type_veventbus  = -38, //xveventbus_t

            enum_xobject_type_xdbgplugin = -39, //used for xdbgplugin_t 
            //block-chain related
            enum_xobject_type_xhashplugin= -40, //universal hash function,refer xhash_t object
            enum_xobject_type_vqccert   = -41, //quorum certification
            enum_xobject_type_vheader   = -42, //general virtual block header
            enum_xobject_type_vblock    = -43, //general virtual block object
            enum_xobject_type_vbindex   = -44, //index for block
            enum_xobject_type_vcauth    = -45, //Certificate-Authority
            enum_xobject_type_vnodesvr  = -46, //service for Node management
            enum_xobject_type_vcache    = -47, //cache layer with function of persist store
            
            enum_xobject_type_ventity   = -48, //general virtual entity of block body
            enum_xobject_type_binventity= -49, //general binary entity of block body
            enum_xobject_type_vinput    = -50, //general virtual input of block body
            enum_xobject_type_voutput   = -51, //general virtual outpu of block body
            enum_xobject_type_vbstate   = -52, //general virtual state of account
            enum_xobject_type_voffdata  = -53, //general virtual offchain data of block
            
            //blockchain 'property related
            enum_xobject_type_vproperty     = -54, //general virtual property of account
            enum_xobject_type_vprop_token   = -55, //property of token
            enum_xobject_type_vprop_nonce   = -56, //property of nonce for account
            enum_xobject_type_vprop_code    = -57, //code property
            enum_xobject_type_vprop_int8    = -58, //int8_t property
            enum_xobject_type_vprop_int16   = -59, //int16_t property
            enum_xobject_type_vprop_int32   = -60, //int32_t property
            enum_xobject_type_vprop_int64   = -61, //int64_t property
            enum_xobject_type_vprop_uint64  = -62, //unt64_t property
            enum_xobject_type_vprop_string  = -63, //sting property
            enum_xobject_type_vprop_hash    = -64, //hash string
            enum_xobject_type_vprop_mkeys   = -65, //manage multiple keys
            enum_xobject_type_vprop_mtokens = -66, //manage multiple tokens

            enum_xobject_type_vprop_int8_vector     = -70, //std::vector<std::string,int8_t>
            enum_xobject_type_vprop_int16_vector    = -71, //std::vector<std::string,int16_t>
            enum_xobject_type_vprop_int32_vector    = -72, //std::vector<std::string,int32_t>
            enum_xobject_type_vprop_int64_vector    = -73, //std::vector<std::string,int64_t>
            enum_xobject_type_vprop_uint64_vector   = -74, //std::vector<std::string,uint64_t>
            enum_xobject_type_vprop_string_vector   = -75, //std::vector<std::string,std::string>
            
            enum_xobject_type_vprop_int8_deque      = -76, //std::vector<std::string,int8_t>
            enum_xobject_type_vprop_int16_deque     = -77, //std::vector<std::string,int16_t>
            enum_xobject_type_vprop_int32_deque     = -78, //std::vector<std::string,int32_t>
            enum_xobject_type_vprop_int64_deque     = -79, //std::vector<std::string,int64_t>
            enum_xobject_type_vprop_uint64_deque    = -80, //std::vector<std::string,uint64_t>
            enum_xobject_type_vprop_string_deque    = -81, //std::vector<std::string,std::string>
            
            enum_xobject_type_vprop_int8_map        = -82, //std::map<std::string,int8_t>
            enum_xobject_type_vprop_int16_map       = -83, //std::map<std::string,int8_t>
            enum_xobject_type_vprop_int32_map       = -84, //std::map<std::string,int8_t>
            enum_xobject_type_vprop_int64_map       = -85, //std::map<std::string,int8_t>
            enum_xobject_type_vprop_uint64_map      = -86, //std::map<std::string,int8_t>
            enum_xobject_type_vprop_string_map      = -87, //std::map<std::string,int8_t>
            enum_xobject_type_vprop_hashmap         = -88, //std::map<std::string,std::map<std::string,std::string>>
            
            
            enum_xobject_type_min       = -255,
        };
        
        //all xobject_t and subclass must create by new operation
        class xobject_t : virtual public xrefcount_t
        {
            friend class xiosignaler_t;
        public:
            enum {enum_obj_type = enum_xobject_type_base};
        public:
            xobject_t();
            xobject_t(const int16_t _enum_xobject_type_); //_enum_xobject_type_ refer enum_xobject_type
        protected:
            virtual ~xobject_t();
        private:
            xobject_t(const xobject_t &);
            xobject_t & operator = (const xobject_t &);
        public:
            inline int64_t    get_obj_id() const { return m_object_id;}
            inline int        get_obj_type() const { return m_object_type;}
            inline int        get_obj_flags() const {return m_object_flags;}
            inline int        get_obj_load() const {return m_load;}
            inline int        get_last_error() const {return m_last_error;}
            virtual std::string get_obj_name() const {return std::string();} //each object may has own name as plugin
  
            virtual bool      is_close();
            virtual bool      is_live(const uint64_t timenow_ms){return true;}//test whether has been idel status
            virtual bool      close(bool force_async = true);
            
            //note: query_interface search vertically from subclass ->parent class ->root class of this object
            //note: query_interface not involve add_ref operation,so caller need do it manually if need keep returned ptr longer
            //caller respond to cast (void*) to related  interface ptr
            virtual void*     query_interface(const int32_t _enum_xobject_type_);
            //return true when event is handled completely
            virtual bool      handle_event(const xvevent_t & ev){return false;}//let object have chance to handle event

        public:
            //plugin query and register
            //note:query_plugin search horizontally from this  to parent for plugin
            //caller respond to cast (xobject_t*) to related object ptr,and release_ref it as query_plugin has done add_ref before return
            virtual xobject_t*  query_plugin(const std::string & plugin_uri){return NULL;} //uri must be formated as  ./name, ../name, */name, or name
            //note:release the returned ptr by calling release_ref when nolonger use it
            virtual xobject_t*  query_plugin(const uint32_t plugin_index){return NULL;}//fastest way to find plugin
            //register a plugin with name of object, must finish all registeration at init stage of object for multiple-thread safe
            virtual bool        register_plugin(xobject_t * plugin_ptr,const int32_t plugin_slot = -1) {return false;}
     
            virtual std::string dump() const;  //just for debug purpose
       
            #if defined(__USE_MEMORY_POOL__)
            void* operator    new(size_t size);
            void  operator    delete(void *p);
            #endif
        public:
            inline void       set_obj_flag(const uint16_t flag)  {m_object_flags |= flag;}   //subclass need arrange those flag well
            inline void       reset_obj_flag(const uint16_t flag){m_object_flags &= (~flag);}//subclass need ensure flag just keep 1 bit
            inline bool       check_obj_flag(const uint16_t flag) const {return ((m_object_flags & flag) != 0);}
            inline void       reset_obj_flags(){m_object_flags = 0;}
        protected:
            inline void       set_load(const uint8_t load){m_load = load;}
            inline void       set_last_error(const int16_t err){m_last_error = err;}
            bool              set_type(const int16_t _enum_xobject_type_);//use carefully, only allow set when m_object_type is 0
        private:
            //xobject may bind a xiosignal object that may trigger and wakeup the host thread when need
            //signal is going close when receive error_code as enum_xerror_code_close
            //return false means object not handled this event
            virtual  bool     on_signal_up(int32_t error_code,int32_t cur_thread_id, uint64_t time_now_ms){return false;}
        private: //ARM required 4/8 byte alignment,dont change order the member variable othewise may throw EXC_ARM_DA_ALIGN
            aligned_int64_t   m_object_id;    //unique object id
            int16_t           m_object_type;  //pre-defined object type,see enum_xobject_type
            uint16_t          m_object_flags; //object 'flags
            int16_t           m_last_error;   //0:successful, present internal error when < 0 ,present system error when > 0
            uint8_t           m_closed;       //indicated whether object is closed but not destroyed yet
            uint8_t           m_load;         //[0,101],object is at invliad status when load > 100
        };
    
        //extern/3rd part register into xcontext to tracking memory and xiobject'lifecycle. use it as following:
            //step#1: xcontext_t::instance().set_debug_modes(enum_debug_mode_memory_check)
            //step#2: xcontext_t::instance().set_debug_plugin(xdbgplugin_t * plugin);
        class xdbgplugin_t : public xobject_t
        {
        protected:
            xdbgplugin_t();
            virtual ~xdbgplugin_t();
        private:
            xdbgplugin_t(const xdbgplugin_t &);
            xdbgplugin_t & operator = (const xdbgplugin_t &);
        public:
            virtual void* query_interface(const int32_t type) override;
        public://subclass need overide
            virtual bool       on_object_create(xobject_t* target) = 0;
            virtual bool       on_object_destroy(xobject_t* target) = 0;
            
            virtual bool       on_object_addref(xobject_t* target) = 0;
            virtual bool       on_object_releaseref(xobject_t* target) = 0;
        };
    
        typedef std::function<void(void*)> xfunction_t;
        typedef void (*xfunction_ptr)(void*);
        class xparam_t
        {
        public:
            enum enum_xparam_type
            {
                enum_xparam_type_null       = 0, //empty
                enum_xparam_type_int64      = 1, //int64...
                enum_xparam_type_uint64     = 2, //uint64...
                enum_xparam_type_uint256    = 3, //uint256..
                enum_xparam_type_xobject    = 4, //xobject_t*
                enum_xparam_type_xfunction  = 5, //xfunction_t*
                enum_xparam_type_stdstring  = 6, //std::string
                enum_xparam_type_xcall      = 7, //xcallback_t*
            };
        public:
            xparam_t();
            xparam_t(const int32_t val);
            xparam_t(const int64_t val);
            xparam_t(const uint64_t val);
            xparam_t(xobject_t * val);
            xparam_t(const uint256_t & val);
            xparam_t(const xfunction_t & func_obj);
            xparam_t(const xfunction_t* func_ptr);
            xparam_t(const xfunction_ptr void_ptr);
            xparam_t(const std::string & _strval);
            ~xparam_t();
            xparam_t(const xparam_t & obj);
            xparam_t & operator = (const xparam_t & right);
            
            void copy_from(const xparam_t & obj);
            void move_from(xparam_t & obj);
            void close();
        public:
            inline enum_xparam_type get_type() const {return param_type;}
            inline xobject_t*   get_object()   const {return object_ptr;}
            inline int64_t      get_int64()    const {return int64_val;}
            inline uint64_t     get_uint64()   const {return uint64_val;}
            inline uint256_t*   get_uint256()  const {return uint256_ptr;}
            inline xfunction_t* get_function() const {return function_ptr;}
            inline const std::string& get_string()   const {return string_val;}
        private:
            union
            {
                xobject_t*      object_ptr;
                xfunction_t*    function_ptr;
                uint256_t*      uint256_ptr;
                int64_t         int64_val;
                uint64_t        uint64_val;
            };
            std::string         string_val;
            enum_xparam_type    param_type;
        };
        
        
        typedef std::function<bool(xcall_t&,const int32_t thread_id,const uint64_t timenow_ms) > xcallback_t;
        typedef bool (*xcallback_ptr)(xcall_t&,const int32_t thread_id,const uint64_t timenow_ms);
        class xcall_t
        {
        public:
            xcall_t();
            
            //init as std::function object
            xcall_t(xcallback_t call);
            xcall_t(xcallback_t call,xparam_t   param1); //init m_param1 to  param1
            xcall_t(xcallback_t call,xparam_t   param1,xparam_t   param2); //init m_param1 and m_param2 to  param1,param2
            xcall_t(xcallback_t call,xparam_t   param1,xparam_t   param2,xparam_t  param3);//m_param1,m_param2,m_param3 are inited
            
            //init as reference
            xcall_t(xcallback_t &call);
            xcall_t(xcallback_t &call,xparam_t    param1); //init m_param1 to  param1
            xcall_t(xcallback_t &call,xparam_t    param1,xparam_t   param2); //init m_param1 and m_param2 to  param1,param2
            xcall_t(xcallback_t &call,xparam_t    param1,xparam_t   param2,xparam_t   param3);//m_param1,m_param2,m_param3 are inited
            
            //init as function pointer
            xcall_t(xcallback_ptr call);
            xcall_t(xcallback_ptr call,xparam_t   param1); //init m_param1 to  param1
            xcall_t(xcallback_ptr call,xparam_t   param1,xparam_t   param2); //init m_param1 and m_param2 to  param1,param2
            xcall_t(xcallback_ptr call,xparam_t   param1,xparam_t   param2,xparam_t   param3);//m_param1,m_param2,m_param3 are inited
            
            ~xcall_t();
            xcall_t(const xcall_t & obj);
            xcall_t & operator = (const xcall_t & obj);
        public://optmize for memory copy for xparam_t
            void bind(xparam_t & param1); //m_param1 is overwrited by param1
            void bind(xparam_t & param1,xparam_t & param2); //m_param1 and m_param2 are overwrited by param1 and param2
            void bind(xparam_t & param1,xparam_t & param2,xparam_t & param3);//m_param1,m_param2,m_param3 are overwrited
            void bind_result(xparam_t & result);//m_result is overwrited by  result
            void bind_taskid(const uint32_t task_id){m_task_id = task_id;}
            
            void move_from(xcall_t & obj);
            void copy_from(const xcall_t & obj);
            void close();
            void init(){}; //contruction already done init, here just for compatible for template
        public:
            xparam_t&   get_param1() {return m_param1;}
            xparam_t&   get_param2() {return m_param2;}
            xparam_t&   get_param3() {return m_param3;}
            xparam_t*   get_result() {return m_result;}
            
            uint32_t    get_taskid() const {return m_task_id;}
            int         get_last_err_code() const {return m_last_err_code;}
            void        set_last_err_code(const int code){m_last_err_code = code;}
        public:
            bool operator()(const int32_t thread_id,const int64_t timestamp_ms)
            {
                return (m_function)(*this,thread_id,timestamp_ms);
            }
        private:
            xcallback_t m_function;
            xparam_t    m_param1;
            xparam_t    m_param2;
            xparam_t    m_param3;
            xparam_t*   m_result;   //at most time, m_result is empty as asynchronization
            uint32_t    m_task_id;  //taskid for this call
            int         m_last_err_code;
        };
        
        //io-related status
        enum enum_xobject_status
        {
            enum_xobject_status_invalid      = 0,  //invalid status
            enum_xobject_status_inited       = 1,  //io-object inited
            enum_xobject_status_attached     = 2,  //io-object attached to host thread
            enum_xobject_status_open         = 5,  //ready to use
            enum_xobject_status_error        = 6,  //has eror happend
            enum_xobject_status_closed       = 7,  //been closed status
        };
        //xiobject_t force requirement: object must be plug at io-thread of m_thread_id, maybe has mailbox
        //io object can received command at host thread, command also can be delivered to xiobject_t
        //every io-object must be created by new
        class xiobject_t : public xobject_t
        {
            friend class xdatabox_t;
            friend class xmailbox_t;
            friend class xiosignaler_t;
        protected:
            enum {enum_max_plugins_count = 8};
            //note:_context must be valid until application/process exit
            xiobject_t(xcontext_t & _context,enum_xobject_type eType);//attach iobject at current thread
            xiobject_t(xcontext_t & _context,const int32_t target_thread_id,enum_xobject_type eType);//attach this object to  target_thread_id
            virtual ~xiobject_t();
        private:
            xiobject_t();
            xiobject_t(const xiobject_t&);
            xiobject_t& operator = (const xiobject_t &);
        public:
            uint64_t                     get_time_now();
            inline enum_xobject_status   get_status() const {return m_status;}
            inline int32_t               get_thread_id() const {return m_thread_id;}//0 means no-bind to any thread yet
            inline xdatabox_t*           get_databox() const {return m_ptr_databox;}//packet is deliver to on_databox_packet
            inline xiosignaler_t*        get_signaler() const {return m_ptr_signaler;}
            inline xcontext_t*           get_context() const {return m_ptr_context;}
            
        public: //multiple-thread safe
            virtual bool        is_close() override;
            virtual bool        close(bool force_async = true) override; //must call close before release object,otherwise object never be cleanup
            virtual void*       query_interface(const int32_t type) override;//caller respond to cast (void*) to related  interface ptr
            //virtual int32_t     add_ref() override;
            //virtual int32_t     release_ref() override;
            
            //allow create own mailbox object
            bool                create_mailbox(int32_t min_batch_read = -1,int32_t max_batch_read = -1,int32_t max_queue_len = 65535);
            //allow create own databox object
            bool                create_databox(int32_t min_batch_read = -1,int32_t max_batch_read = -1,int32_t max_queue_len = 65535);
            //allow create own signaler object
            bool                create_signaler();
                    
        public: //multiple thread safe,return error code if fail, refer enum_xerror_code
            //Note: signal/post api execute the xcall_t at target thread through it's own mailbox or the thread'mailbox

            //send() is 100% asynchronize,it ensure to execute call at target thread as the order,
            //pass 0 for cur_thread_id if dont know actualy thread id
            virtual int32_t     send_call(xcall_t & call,int32_t cur_thread_id = 0); //send cmd and wakeup target io-thread,can be called at any thread
            //dispatch() might execute immediately if now it is at target thread,otherwise do send()
            virtual int32_t     dispatch_call(xcall_t & call,int32_t cur_thread_id = 0);
            
            //post is the optmization for larget amount xcall_t who need to deliver to target thread as bunch mode
            virtual int32_t     post_call(xcall_t & call,int32_t cur_thread_id = 0);       //just pass data ,not singal to wake up thread immidiately
            virtual int32_t     signal_call(int32_t cur_thread_id = 0);                   //just wakeup the io-thread of this io object
        
            virtual int32_t     count_calls(int64_t & total_in, int64_t & total_out);  //count how many calls are pending at queue, it is useful for debug purpose
            
        public: //plugin query and register
            //note:query_plugin search horizontally from this  to parent for plugin
            //caller respond to cast (xobject_t*) to related object ptr,and release_ref it as query_plugin has done add_ref before return
            virtual xobject_t*  query_plugin(const std::string & plugin_uri) override; //uri must be formated as  ./name, ../name, */name, or name
            //note:release the returned ptr by calling release_ref when nolonger use it
            virtual xobject_t*  query_plugin(const uint32_t plugin_index) override;//fastest way to find plugin
            
            //register a plugin with name of object, must finish all registeration at init stage of object for multiple-thread safe
            virtual bool        register_plugin(xobject_t * plugin_ptr,const int32_t plugin_slot = -1) override;
            
        protected:
            //on_object_close be called when close command processed at host thread,logic flow: Caller(Thread#A)->Close()->Wake this object thread(B)->clean and then execute: on_object_close
            virtual bool        on_object_close(); //notify the subclass the object is closed
            
            //packet is from   send(xpacket_t & packet) or dispatch(xpacket_t & packet) of xdatabox_t
            //subclass need overwrite this virtual function if they need support signal(xpacket_t) or send(xpacket_t),only allow called internally            
            virtual  bool      on_databox_open(xpacket_t & packet,int32_t cur_thread_id, uint64_t time_now_ms){return false;}
            //notify the owner of mailbox and give owner one chance to process something else
            //mailbox do default action(execute xcall_t) when return false(owner object not process it)
            virtual  bool      on_mailbox_open(xcall_t & call,int32_t cur_thread_id, uint64_t time_now_ms){return false;}//return false if not handle this
        
            //xobject may bind a xiosignal object that may trigger and wakeup the host thread when need
            //signal is going close when receive error_code as enum_xerror_code_close
            //return false means object not handled this event
            virtual  bool     on_signal_up(int32_t error_code,int32_t cur_thread_id, uint64_t time_now_ms) override {return true;}
            
        public://allow send/post/dispatch general lambda function to execute
            
            #ifdef __GCC_50_OR_ABOVE__
            template<class _Rp, class ..._ArgTypes>
            bool send_call(const std::function<_Rp(_ArgTypes...)> & job_function,_ArgTypes... margs)
            {
                //safe to use this inside of lambda since this already been encoded into xcall
                std::function<_Rp(_ArgTypes...)> * _job_function_ptr = new std::function<_Rp(_ArgTypes...)>(job_function);
                auto _internal_asyn_function = [_job_function_ptr,margs...](base::xcall_t & call, const int32_t cur_thread_id,const uint64_t timenow_ms)->bool{
                    
                    (*_job_function_ptr)(margs...);
                    
                    delete _job_function_ptr; //now delete the attached function ptr
                    return true;
                };
                xcall_t asyn_call(_internal_asyn_function,(xobject_t*)this);//allow use this ptr safely
                if(send_call(asyn_call,0) == enum_xcode_successful)
                    return true;
                
                return false;
            }
            template<class _Rp, class ..._ArgTypes>
            bool dispatch_call(const std::function<_Rp(_ArgTypes...)> & job_function,_ArgTypes... margs)
            {
                //safe to use this inside of lambda since this already been encoded into xcall
                std::function<_Rp(_ArgTypes...)> * _job_function_ptr = new std::function<_Rp(_ArgTypes...)>(job_function);
                auto _internal_asyn_function = [_job_function_ptr,margs...](base::xcall_t & call, const int32_t cur_thread_id,const uint64_t timenow_ms)->bool{
                    
                    (*_job_function_ptr)(margs...);
                    
                    delete _job_function_ptr; //now delete the attached function ptr
                    return true;
                };
                xcall_t asyn_call(_internal_asyn_function,(xobject_t*)this);//allow use this ptr safely
                if(dispatch_call(asyn_call,0) == enum_xcode_successful)
                    return true;
                
                return false;
            }
            template<class _Rp, class ..._ArgTypes>
            bool post_call(const std::function<_Rp(_ArgTypes...)> & job_function,_ArgTypes... margs)
            {
                //safe to use this inside of lambda since this already been encoded into xcall
                std::function<_Rp(_ArgTypes...)> * _job_function_ptr = new std::function<_Rp(_ArgTypes...)>(job_function);
                auto _internal_asyn_function = [_job_function_ptr,margs...](base::xcall_t & call, const int32_t cur_thread_id,const uint64_t timenow_ms)->bool{
                    
                    (*_job_function_ptr)(margs...);
                    
                    delete _job_function_ptr; //now delete the attached function ptr
                    return true;
                };
                xcall_t asyn_call(_internal_asyn_function,(xobject_t*)this);//allow use this ptr safely
                if(post_call(asyn_call,0) == enum_xcode_successful)
                    return true;
                
                return false;
            }
            #else
            bool    send_call(const xfunction_t& job_function,void* param);
            bool    dispatch_call(const xfunction_t& job_function,void* param);
            bool    post_call(const xfunction_t& job_function,void* param);
            #endif //end of __GCC_50_OR_ABOVE__
        protected:
            inline xmailbox_t*  get_mailbox() const {return m_ptr_mailbox;}
            inline xiothread_t* get_thread()  const {return m_ptr_thread;}
            int32_t             get_current_thread_id();
            void                set_status(enum_xobject_status newstatue);
            
            uint64_t            update_time_now();//trigger refresh time to more accurately and return latest time now.carefully: it ask call at host thread
            
        private:
            virtual bool        lock()  {return false;}   //if need subclass may provde lock function  and unlock
            virtual bool        unlock(){return false;}
        private:
            xiosignaler_t*      m_ptr_signaler;        //dedicated signaler of this object,it usally be NULL
            xdatabox_t  *       m_ptr_databox;         //dedicated databox of this object, it usally be NULL
            xmailbox_t  *       m_ptr_mailbox;         //dedicated mailbox of this object, it usally be NULL
            xiothread_t *       m_ptr_thread;          //associated io thread with this io object
            xcontext_t  *       m_ptr_context;         //associated with global context object
            int32_t             m_thread_id;           //the logic thread id whom this object belong to under m_pContext
            enum_xobject_status m_status;              //status of io object
        protected: //note: only support max 8 plugins for one object as considering size and reality
            xobject_t*          m_plugins[enum_max_plugins_count];
        };
        
        //xionode_t manage chain structure with parent & child xiobject
        class xionode_t : public xiobject_t
        {
        protected:
            xionode_t(xionode_t & parent_object,enum_xobject_type eType);
            xionode_t(xionode_t & parent_object,const int32_t thread_id,enum_xobject_type eType);
            xionode_t(xcontext_t & _context,const int32_t thread_id,enum_xobject_type eType);
            virtual ~xionode_t();
        private:
            xionode_t();
            xionode_t(const xionode_t &);
            xionode_t & operator = (const xionode_t &);
        public:
            xionode_t*              get_parent_node()   const;
            xionode_t*              get_child_node()    const;
            const xvip2_t &         get_xip2_addr()     const{return m_xip2_addr;}
            const xvip_t            get_xip2_low_addr() const{return m_xip2_addr.low_addr;}
            const uint64_t          get_xip2_high_addr()const{return m_xip2_addr.high_addr;}
            
            //note: query_interface search vertically from subclass ->parent class ->root class of this object
            //note: query_interface not involve add_ref operation,so caller need do it manually if need keep returned ptr longer
            //caller respond to cast (void*) to related  interface ptr
            virtual void*           query_interface(const int32_t type) override;
            
            //note:query_plugin search horizontally from this  to parent for plugin
            //caller respond to cast (xobject_t*) to related object ptr,and release_ref it as query_plugin has done add_ref before return
            virtual xobject_t*      query_plugin(const std::string & plugin_uri) override; //uri must be formated as  ./name, ../name, */name, or name
    
            virtual bool            is_match(const xvip2_t& xip_address);
            
        public://attach/detach childnode to this node at  multiple thread safe
            virtual bool            attach_child_node(xionode_t * child_node,const xvip2_t & child_address,const std::string extra_data);
            virtual bool            detach_child_node(xionode_t * child_node,const std::string extra_data);
            
        public: //multiple_thread safe
            //push_event_up: throw event from lower(child) layer to higher(parent)
            bool         push_event_up(const xvevent_t & event ,xionode_t* from_child,int32_t cur_thread_id,uint64_t timenow_ms);
            
            //push_event_down: push event from higher(parent) layer to lower(child)
            bool         push_event_down(const xvevent_t & event ,xionode_t* from_parent,int32_t cur_thread_id,uint64_t timenow_ms);
            
        protected: //guanrentee be called  at object'thread,triggered by push_event_up or push_event_down
            
            //note: to return false may call parent'push_event_up,or stop further routing when return true
            virtual bool            on_event_up(const xvevent_t & event,xionode_t* from_child,const int32_t cur_thread_id,const uint64_t timenow_ms);
            //note: to return false may call child'push_event_down,or stop further routing when return true
            virtual bool            on_event_down(const xvevent_t & event,xionode_t* from_parent,const int32_t cur_thread_id,const uint64_t timenow_ms);
            
        protected: //notify self about event of childnode join & leave,at self thread (so multiple thread safe)
            
            //notify has child-node joined this node,errorcode refer enum_error_code ,return true when the event is handled
            virtual bool            on_child_node_join(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xionode_t* childnode);
            
            //notify has child-node left from this node,
            virtual bool            on_child_node_leave(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,xionode_t* childnode);
            
        protected://callbacked from parent node to notify result of attach & detach (note:called from parent'running thread)
            
            //notify this node that is joined into parent-node
            virtual bool            on_join_parent_node(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,const xvip2_t & alloc_address,const std::string & extra_data,xionode_t* from_parent);
            
            //notify this node that is left from parent-node
            virtual bool            on_leave_parent_node(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms,const std::string & extra_data,xionode_t* from_parent);
            
        protected:
            virtual bool            on_object_close() override; //notify the subclass the object is closed
            virtual bool            reset_xip_addr(const xvip2_t & new_addr); //reserved for future to replace this xip2 address
        private:
            xionode_t*              m_parent_node;
            xionode_t*              m_child_node;
            xvip2_t                 m_xip2_addr;  //see detail definition at xbase.h for xip2
        };
        
        //for xfd_events_t
        enum enum_xfd_event_type
        {
            //poll event(compatible with libuv)
            /*
             enum uv_poll_event {
             UV_READABLE = 1,
             UV_WRITABLE = 2,
             UV_DISCONNECT = 4,
             UV_PRIORITIZED = 8
             };
             */
            enum_xio_event_poll_in           = 1 << 0,    //when data can read
            enum_xio_event_poll_out          = 1 << 1,    //when data send out/write
            enum_xio_event_poll_disconnect   = 1 << 2,    //want socket/handle is disconnected
            enum_xio_event_poll_prioritized  = 1 << 3,    //event is used to watch for sysfs interrupts or TCP out-of-band messages.
            
            enum_xio_event_poll_idle         = 1 << 4,    //want on_sock_idle
            enum_xio_event_poll_err          = 1 << 5,    //eror on file description
            enum_xio_event_poll_hup          = 1 << 6,    //file description hangup
            enum_xio_event_poll_invalid      = 1 << 7,    //invalid file description
            
            enum_xio_event_poll_close    = enum_xio_event_poll_err | enum_xio_event_poll_hup | enum_xio_event_poll_invalid,
            enum_xio_event_poll_all      = enum_xio_event_poll_in | enum_xio_event_poll_out | enum_xio_event_poll_disconnect | enum_xio_event_poll_prioritized | enum_xio_event_poll_idle | enum_xio_event_poll_close,
        };
        class xiosink_t : virtual public xrefcount_t
        {
        protected:
            xiosink_t(){};
            virtual ~xiosink_t(){};
        private:
            xiosink_t(const xiosink_t &);
            xiosink_t & operator = (const xiosink_t &);
        public: //return true when the event is handled
            //xiohandle_t attached into io-thread of the target thread(host) ,return true when the event is handled
            virtual bool        on_iohandle_attach(xfd_handle_t handle,const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms) = 0;
            
            //xiohandle_t detached from io-thread of the target(host) thread,return true when the event is handled
            virtual bool        on_iohandle_detach(xfd_handle_t handle,const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms) = 0;//detach means it detach from io-thread but maybe the   fdhandle(socket) is still valid
            
            //handle is closed and inited by caller if error_code is 0
            virtual bool        on_iohandle_close(const int32_t error_code,xfd_handle_t handle,const int32_t cur_thread_id,const uint64_t timenow_ms) = 0;
            
            //readable event; return new fd_events_t if want change listened events,b_handled indicate whether it is processed
            //when no-longer need this event set watchEvents to 0,which emove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_read(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms) = 0;
            
            //writeable event;return new fd_events_t if want change listened,and b_handled indicate whether event is handled or not
            //when no-longer need this event set watchEvents to 0 which will remove it from loop
            //return true when the event is handled
            virtual bool        on_iohandle_write(xfd_handle_t handle,xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_msd) = 0;
        };
        
        //xiohandle_t wrap posix descrptor with reactor mode
        class xiohandle_t : public xiobject_t
        {
        public:
            static bool   set_nonblock(xfd_handle_t socket,bool non_block_or_not);
        protected:
            xiohandle_t(xcontext_t & _context,int32_t host_thread_id,xfd_handle_t rawHandle,xiosink_t * event_receiver);
            virtual ~xiohandle_t();
        private:
            xiohandle_t();
            xiohandle_t(const xiohandle_t &);
            xiohandle_t & operator = (const xiohandle_t &);
        public: //thread safe to call from any thread
            xfd_handle_t            get_handle() const {return m_raw_handle;} //query the raw handle
            xfd_events_t            get_events() const {return m_watch_events;}//refer enum_fd_event_type
        public: //thread safe to call from any thread
            virtual  void*          query_interface(const int32_t type) override; //caller respond to cast (void*) to related  interface ptr
            
            //multiple threads safe and return error code  refer enum_error_code,queue_up indicate whether force asyncronize
            virtual  int32_t        attach(bool queue_up = false); //start create native handle really and start to attach to host thread
            virtual  int32_t        detach(bool queue_up = false); //stop to receive event. caller may call attach() to start receive signal
            
            //start/stop receive on_iohandle_read(),multiple threads safe
            virtual  int32_t        start_read(int32_t cur_thread_id) = 0;
            virtual  int32_t        stop_read(int32_t cur_thread_id) = 0;
            //start/stop receive on_iohandle_write(),multiple threads safe
            virtual  int32_t        start_write(int32_t cur_thread_id) = 0;
            virtual  int32_t        stop_write(int32_t cur_thread_id) = 0;
            
            //combine function
            virtual  int32_t        start_write_read(int32_t cur_thread_id) = 0;
 
        protected: //can only be called from host thread
            //xiohandle_t attached into io-thread of the target thread(host) ,return true when the event is handled
            virtual bool        on_iohandle_attach(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms);
            
            //xiohandle_t detached from io-thread of the target(host) thread,return true when the event is handled
            virtual bool        on_iohandle_detach(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms); //detach means it detach from io-thread but maybe the   fdhandle(socket) is still valid
            
            //readable event; return new fd_events_t if want change listened events,b_handled indicate whether it is processed
            //when no-longer need this event set watchEvents to 0,which emove it from loop
            //return true when the event is handled,sock_read_buf indicate how many bytes can read from buffer if sock_read_buf >=0, it means dont know if sock_read_buf < 0
            virtual bool        on_iohandle_read(xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_ms);
            
            //writeable event;return new fd_events_t if want change listened,b_handled indicate event is handled or not
            //when no-longer need this event set watchEvents to 0 which will remove it from loop,return true when the event is handled
            //sock_write_buf indicate how many bytes may write to socket if sock_write_buf >=0,it means dont know if sock_write_buf < 0
            virtual bool        on_iohandle_write(xfd_events_t & watchEvents,const int32_t cur_thread_id,const uint64_t timenow_msd);
            
            //return true when the event is handled
            virtual bool        on_iohandle_close(const int32_t error_code,const int32_t cur_thread_id,const uint64_t timenow_ms);
            
            //can only be called from host thread
            virtual bool        on_object_close() override; //notify the subclass the object is closed
        private:
            virtual bool        process_iohandle_attach_cmd(xcall_t & cmd,const int32_t cur_thread_id,const uint64_t timenow_ms) = 0; //must implement
            virtual bool        process_iohandle_detach_cmd(xcall_t & cmd,const int32_t cur_thread_id,const uint64_t timenow_ms) = 0; //must implement
        public:
            //advance use case: provider(like libcur) that manage real socket handle self to share for mutiple session  so xiohandle_t can not close them,to solve the request we have to provide reset_handle before xiohandle_t.close()
            //and other case xiohandle_t and subclass may manage this handle and close it before object destroy
            xfd_handle_t         reset_handle();
        protected:
            void                 set_events(xfd_events_t new_event) {m_watch_events = new_event;} //refer enum_fd_event_type
            xfd_handle_t         close_handle();  //return the closed handle to reference
        protected:
            xiosink_t *          m_ptr_event_receiver;
            void*                m_padding_sizeof_pointer;
        private:
            xfd_handle_t         m_raw_handle;       //must valid and unique as system wide
            xfd_events_t         m_watch_events; //refer enum_fd_event_type,the events socket currently watching/monitor
        };
        
        //use case: xauto_ptr is mixed with std:unique_ptr and std::shared_ptr, but with limited functions
        //usally receive the ptr of created object, and release it when go out of scope
        template<typename T>  //T must be subclass of i_refcount_t
        class xauto_ptr
        {
            static_assert(std::is_base_of<xrefcount_t, T>::value, "T must be type from xrefcount_t");
        public:
            xauto_ptr(std::nullptr_t)
            :m_raw_ptr(nullptr)
            {
            }
            xauto_ptr(const T * obj_ptr)
            :m_raw_ptr((T*)obj_ptr)
            {
            }
            xauto_ptr(T & obj_ref)
            :m_raw_ptr(&obj_ref)
            {
            }
            xauto_ptr(xauto_ptr && moved)
             :m_raw_ptr(moved.m_raw_ptr)
            {
                moved.m_raw_ptr = nullptr;
            }
            ~xauto_ptr()
            {
                if(m_raw_ptr != nullptr)
                    m_raw_ptr->release_ref();
            }
            
            //test whether it is valid or not
            inline operator bool ()    const noexcept {return (m_raw_ptr != nullptr);}
            inline bool operator == (std::nullptr_t)  const noexcept {return (m_raw_ptr == nullptr);}
            inline bool operator != (std::nullptr_t)  const noexcept {return (m_raw_ptr != nullptr);}
            
            //get raw ptr
            inline T * operator ()()   const noexcept {return m_raw_ptr;}
            inline T *         get()   const noexcept {return m_raw_ptr;}
            inline T* operator ->()    const noexcept {return m_raw_ptr;}
            //test first before call it
            inline T& operator *()     const noexcept {return *m_raw_ptr;}
        
            //disable this convenient way as that compiler might do copy-elision that might cause issue,e.g. T * ptr = xauto_ptr<T> get_xxx()
            //inline operator T* ()      const noexcept {return m_raw_ptr;}
            //inline operator const T* ()const noexcept {return m_raw_ptr;}
        protected:
            xauto_ptr(const xauto_ptr & other)
                :m_raw_ptr((T*)other.m_raw_ptr)
            {
                if(m_raw_ptr != nullptr)
                    m_raw_ptr->add_ref();//gain reference
            }
        private:
            xauto_ptr();
            xauto_ptr & operator = (const xauto_ptr &);
        protected:
            xauto_ptr & operator = (xauto_ptr && moved)
            {
                if (this != &moved)
                {
                    T * old_ptr = m_raw_ptr;
                    m_raw_ptr = moved.m_raw_ptr;
                    moved.m_raw_ptr = nullptr;
                    if(old_ptr != nullptr)
                        old_ptr->release_ref();
                }
                return *this;
            }
        protected:
            T * m_raw_ptr;
        };
        template<class T>
        bool operator!=(std::nullptr_t, xauto_ptr<T> const & ptr) { return (ptr != nullptr);}
        template<class T>
        bool operator==(std::nullptr_t, xauto_ptr<T> const & ptr) { return (ptr == nullptr);}

        template<typename T, typename ... ArgsT>
        xauto_ptr<T>
        make_auto_ptr(ArgsT && ... args) {
            return xauto_ptr<T>(new T(std::forward<ArgsT>(args)... ));
        }
    
        //auto addref when contruct and do releaes_ref when out of scope
        template<typename T>
        class auto_reference : public xauto_ptr<T>
        {
            typedef xauto_ptr<T> base;
        public:
            auto_reference(T * obj_ptr)
                :base(obj_ptr)
            {
                if(obj_ptr != nullptr)
                    obj_ptr->add_ref();
            }
            auto_reference(T & obj_ref)
                :base(obj_ref)
            {
                obj_ref.add_ref();
            }
            ~auto_reference(){};//xauto_ptr::~xauto_ptr will do release
        private:
            auto_reference();
            auto_reference(const auto_reference &);
            auto_reference & operator = (const auto_reference &);
        };

    };//end of namespace of base
}; //end of namespace top

