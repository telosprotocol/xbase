// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "xobject.h"

namespace top
{
    namespace base
    {
        //timer callback
        //guarentee all event callbacked at same thread
        class xtimersink_t : virtual public xrefcount_t
        {
        protected:
            xtimersink_t(){};
            virtual ~xtimersink_t(){};
        private:
            xtimersink_t(const xtimersink_t &);
            xtimersink_t & operator = (const xtimersink_t &);
        public:
            virtual bool        on_timer_start(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) = 0;   //attached into io-thread
            virtual bool        on_timer_stop(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) = 0;   //detach means it detach from io-thread
            //return true if the event is already handled,return false to stop timer as well
            //start_timeout_ms present when the duration of first callback
            //in_out_cur_interval_ms carry back the new setting() for timer,stop timer if <= 0 means; otherwise ask timer repeat with this new interval
            virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) = 0;
        };
        
        //timer api/interface
        //timer(on_timer_xxx) is triggered and called at this io object 'thread
        class xtimer_t : public xiobject_t
        {
        protected:
            xtimer_t(xcontext_t & _context,int32_t timer_thread_id,xtimersink_t * event_receiver);
            virtual ~xtimer_t();
        private:
            xtimer_t();
            xtimer_t(const xtimer_t &);
            xtimer_t & operator = (const xtimer_t &);
        public:
            //timer_id never changed at timer 'lifetime
            inline int64_t     get_timer_id() const {return m_timer_id;}
            
            //once started again,  m_timer_version increased 1
            inline int64_t     get_timer_version() const {return m_timer_version;}
            
            //get latest timeout setting
            inline int32_t     get_timeout() const {return m_timeout_ms;}
            //get latest repeat interval setting
            inline int32_t     get_repeat_interval() const {return m_repeat_interval_ms;}
        protected:
            void        set_timeout(int32_t time_ms);
            void        set_repeat_interval(int32_t time_ms);
            
        public: //multiple thread safe
            //non-repeat timer keep active to true until on_timer_fire called
            virtual bool        is_active() {return (m_actived != 0);}  //say whether timer is started
            
            //if timeout_ms is zero, the callback fires on the next event loop iteration. If repeat is non-zero, the callback fires first after timeout milliseconds and then repeatedly after repeat milliseconds.
            virtual int32_t     start(const int32_t timeout_ms,const int32_t repeat_interval_ms) = 0;//return error code  refer enum_error_code
            virtual int32_t     stop() = 0; //after stop, may call start again
            
        protected: //can only be called from this object 'io thread
            virtual bool        on_timer_start(const int32_t error_code,const int32_t thread_id,const int64_t timer_id,const uint64_t cur_time_ms);   //attached into io-thread
            virtual bool        on_timer_stop(const int32_t error_code,const int32_t thread_id,const int64_t timer_id,const uint64_t cur_time_ms);   //detach means it detach from io-thread
            
            //in_out_cur_interval_ms carry back the new setting() for timer,stop timer if <= 0 means; otherwise ask timer repeat with this new interval
            //return false to stop timer as well
            virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const uint64_t cur_time_ms, int32_t & in_out_cur_interval_ms);
            
        protected://has addtional clean requirement,so overwrite the default on_object_close
            virtual bool        on_object_close() override;
        private:
            std::atomic<int64_t>m_timer_version;  //each start increase to new version code
            int64_t             m_timer_id;
            xtimersink_t*       m_ptr_event_receiver;
            void*               m_padding;
            int32_t             m_timeout_ms; //when first timer event fire relative now
            int32_t             m_repeat_interval_ms;  //repeat timer. 0 means not repeat
            static std::atomic<int64_t> s_xtimer_id_base;
        protected:
            uint8_t             m_actived;
            uint8_t             m_stopped; //already received on_timer_stop event
        };
        
        //xxtimer_t object is a wrap implementation by construct xtimer_t & xtimersink_t
        class xxtimer_t : public xiobject_t, public xtimersink_t
        {
        public:
            //timer_thread_id must be ready before create xxtimer_t object,otherwise it throw exception
            xxtimer_t(xcontext_t & _context,int32_t timer_thread_id);
        protected:
            xxtimer_t(xcontext_t & _context,int32_t timer_thread_id,enum_xobject_type sub_object_type);
            virtual ~xxtimer_t();
        private:
            xxtimer_t();
            xxtimer_t(const xxtimer_t &);
            xxtimer_t & operator = (const xxtimer_t &);
            
        public://multiple thread safe
            //timer_id never changed at timer 'lifetime
            inline int64_t     get_timer_id() const {return m_raw_timer->get_timer_id();}
            
            //once started again,  m_timer_version increased 1
            inline int64_t     get_timer_version() const {return m_raw_timer->get_timer_version();}
            
            //get latest timeout setting
            inline int32_t     get_timeout() const {return m_raw_timer->get_timeout();}
            //get latest repeat interval setting
            inline int32_t     get_repeat_interval() const {return m_raw_timer->get_repeat_interval();}
            
        public://multiple thread safe
            //non-repeat timer keep active to true until on_timer_fire called
            bool                is_active() {return m_raw_timer->is_active();}
            
            //if timeout_ms is zero, the callback fires on the next event loop iteration. If repeat is non-zero, the callback fires first after timeout milliseconds and then repeatedly after repeat milliseconds. the return error code  refer enum_error_code
            int32_t             start(const int32_t timeout_ms,const int32_t repeat_interval_ms) {return m_raw_timer->start(timeout_ms,repeat_interval_ms); }
            int32_t             stop() {return m_raw_timer->stop();} //after stop, may call start again
            
        private: //following api are from xtimersink_t, and  be called from timer thread,
            virtual bool        on_timer_start(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override;    //attached into io-thread
            virtual bool        on_timer_stop(const int32_t errorcode,const int32_t thread_id,const int64_t timer_id,const int64_t cur_time_ms,const int32_t timeout_ms,const int32_t timer_repeat_ms) override;  //detach means it detach from io-thread
            //return true if the event is already handled,return false to stop timer as well,
            //start_timeout_ms present when the duration of first callback
            //in_out_cur_interval_ms carry back the new setting() for timer,stop timer if <= 0 means; otherwise ask timer repeat with this new interval
            virtual bool        on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) override;
        protected://has addtional clean requirement,so overwrite the default on_object_close
            virtual bool        on_object_close() override;
        private:
            xtimer_t*           m_raw_timer;
        };
    } //end of namespace base
} //end of namespace top
