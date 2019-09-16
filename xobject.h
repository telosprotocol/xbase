// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <memory>
#include <functional>

#include "xbase.h"
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
        
        class xrefcount_t
        {
        protected:
            xrefcount_t();
            virtual ~xrefcount_t();
        public:
            virtual int32_t   add_ref();
            virtual int32_t   release_ref();
        public:
            int32_t           get_refcount() const { return m_refcount;}
        protected:
            //the default implementation do delete ,so any object inherited  from  xrefcount_t must create by new operator 
            virtual bool      destroy()
			{
				delete this;
				return true;
			}
        private:
            std::atomic<int32_t>  m_refcount;     //reference count as atom operate
        };
        
        //xobject_t require to create object by new ,and destroy by release
        enum enum_xobject_type
        {
            //application may use types of [1,32767]
            enum_xobject_type_max       = 32767,
            
            enum_xobject_type_consensus_max = 1152,
            enum_xobject_type_consensus_min = 1025,
            
            enum_xobject_type_data_max  = 1024,
            enum_xobject_type_data_min  = 100,
            
            enum_xobject_type_system_contract = 1,
            
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
            enum_xobject_type_dataunit  = -8,
            enum_xobject_type_dataobj   = -9,
            enum_xobject_type_endpoint  = -10,
            enum_xobject_type_socket    = -11, //xsocket object
            enum_xobject_type_connection= -12, //xconnection object
            enum_xobject_type_node      = -13, //xnode_t object
            enum_xobject_type_woker     = -14, //worker thread object
            enum_xobject_type_service   = -15, //service
            
            enum_xobject_type_min       = -32767,
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
  
            virtual bool      is_close();
            virtual bool      close(bool force_async = true);
            //caller respond to cast (void*) to related  interface ptr
            virtual void*     query_interface(const int32_t _enum_xobject_type_);
            virtual std::string dump();  //just for debug purpose
       
            #if defined(__USE_MEMORY_POOL__)
            void* operator    new(size_t size);
            void  operator    delete(void *p);
            #endif
        protected:
            inline void       set_load(const uint8_t load){m_load = load;}
            inline void       set_last_error(const int16_t err){m_last_error = err;}
            inline void       set_flag(const uint16_t flag){m_object_flags |= flag;}
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
        protected:
            inline xmailbox_t*  get_mailbox() const {return m_ptr_mailbox;}
            inline xiothread_t* get_thread()  const {return m_ptr_thread;}
            int32_t             get_current_thread_id();
            void                set_status(enum_xobject_status newstatue);
            
            uint64_t            update_time_now();//trigger refresh time to more accurately and return latest time now.carefully: it ask call at host thread
        private:
            xiosignaler_t*      m_ptr_signaler;        //dedicated signaler of this object,it usally be NULL
            xdatabox_t  *       m_ptr_databox;         //dedicated databox of this object, it usally be NULL
            xmailbox_t  *       m_ptr_mailbox;         //dedicated mailbox of this object, it usally be NULL
            xiothread_t *       m_ptr_thread;          //associated io thread with this io object
            xcontext_t  *       m_ptr_context;         //associated with global context object
            int32_t             m_thread_id;           //the logic thread id whom this object belong to under m_pContext
            enum_xobject_status m_status;              //status of io object
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
            //advance use case: provider(like libcur) that manage real socket handle self to share for mutiple session  so Juiohandle_t can not close them,to solve the request we have to provide reset_handle before Juiohandle_t.close()
            //and other case Juiohandle_t and subclass may manage this handle and close it before object destroy
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
    
        typedef std::function<void()> xfunction_t;
        typedef void (*xfunction_ptr)();
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
            };
        public:
            xparam_t();
            xparam_t(const int32_t val);
            xparam_t(const int64_t val);
            xparam_t(const uint64_t val);
            xparam_t(xobject_t * val);
            xparam_t(const uint256_t & val);
            xparam_t(xfunction_t func_obj);
            xparam_t(xfunction_t* func_ptr);
            xparam_t(xfunction_ptr void_ptr);
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
            inline std::string& get_string()         {return string_val;}
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
            
            void move_from(xcall_t & obj);
            void copy_from(const xcall_t & obj);
            void close();
            void init(){}; //contruction already done init, here just for compatible for template
        public:
            xparam_t&   get_param1() {return m_param1;}
            xparam_t&   get_param2() {return m_param2;}
            xparam_t&   get_param3() {return m_param3;}
            xparam_t*   get_result() {return m_result;}
            
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
            int         m_last_err_code;
        };
        
        //use case: receive the created object ptr, and release it when go out of scope
        //T must be subclass of i_refcount_t
        template<typename T>
        class auto_obj_t
        {
        public:
            auto_obj_t(T * obj_ptr)
            :raw_ptr(obj_ptr)
            {
            }
            
            auto_obj_t(T & obj_ptr)
            :raw_ptr(&obj_ptr)
            {
            }
            
            ~auto_obj_t()
            {
                if(raw_ptr != 0)
                    raw_ptr->release_ref();
            }
            
            operator bool (){return (raw_ptr != NULL);}
            T * operator ()() const {return raw_ptr;}
            operator T* () const {return raw_ptr;}
            operator const T* () const {return raw_ptr;}
            
            T* operator ->() const {return raw_ptr;}
            T& operator *() const {return *raw_ptr;}
        private:
            auto_obj_t();
            auto_obj_t(const auto_obj_t &);
            auto_obj_t & operator = (const auto_obj_t &);
        private:
            T * raw_ptr;
        };
        
        //auto addref when contruct and do releaes_ref when out of scope
        template<typename T>
        class auto_reference_t : public auto_obj_t<T>
        {
            typedef auto_obj_t<T> base;
        public:
            auto_reference_t(T * obj_ptr)
            :base(obj_ptr)
            {
                if(obj_ptr != 0)
                    obj_ptr->add_ref();
            }
            
            auto_reference_t(T & obj_ptr)
            :base(&obj_ptr)
            {
                if(obj_ptr != 0)
                    obj_ptr->add_ref();
            }
            
            ~auto_reference_t(){};  //auto_obj_t::~auto_obj_t will do release
        private:
            auto_reference_t();
            auto_reference_t(const auto_reference_t &);
            auto_reference_t & operator = (const auto_reference_t &);
        };

    };//end of namespace of base
}; //end of namespace top

