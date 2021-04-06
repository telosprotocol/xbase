// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase.h"

#include <stdio.h>
#include <stdlib.h>
#include <atomic>
#include <string>

namespace top
{
    namespace base
    {
        
        //TODO: add logic to manage generated log files, e.g. clean the old log files to save disk
        //implement a lock-free log system through system write(append mode)
        class xlogger_t
        {
        public:
            static  xlogger_t & instance();//return default logger
            static  xlogger_t * create_instance(const std::string module_name,const std::string log_dir);//create addtional logger
        protected:
            xlogger_t(const std::string module_name,const std::string log_dir);
            ~xlogger_t();
        private:
            xlogger_t();
            xlogger_t(const xlogger_t &);
            xlogger_t & operator = (const xlogger_t &);
        public:
            bool    init(const std::string log_file_path,bool is_tracking_thread,bool is_rotate_file);
            bool    close();  //clean up resource
            bool    set_log_file_hook(_func_create_log_file_cb _call_back);
            bool    set_log_trace_hook(_func_hook_trace_cb _call_back);
            int     set_trace_lines_per_file(uint32_t max_tracelines_per_file);//decide to rotate to new log file after how many lines
        public:
            void    dbg(const char* msg, ...);
            void    info(const char* msg, ...);
            void    kinfo(const char* msg, ...);
            void    warn(const char* msg, ...);
            void    error(const char* msg, ...);
            void    log(enum_xlog_level level,const char* _module_name,const char* msg,const int msg_length);
            void    log(const char* module_name,const char* _location,enum_xlog_level level,const char* msg, va_list & args);
            void    flush();
            int     get_log_level() const {return m_log_level;}
            void    set_log_level(enum_xlog_level new_level){ m_log_level = new_level;}
            const   std::string get_module_name() {return m_module_name;}
        private:
            void    init_log_file(std::string new_log_file_name);
            bool    get_rotate_file(std::string & strRollingFileName);
            bool    rotate_log_file();
        private:
            int     m_process_id;
            int     m_log_level;
            bool    m_is_tracking_thread;
            bool    m_is_rotate_file;
            _func_hook_trace_cb       m_log_trace_callback;
            _func_create_log_file_cb  m_create_log_file_callback;
        private:
            int          m_atom_lock;
            uint32_t     m_log_lines;  //how many lines are traced out per file
            uint32_t     m_max_log_lines_per_file;
            uint32_t     m_log_file_index;
            int32_t      m_log_file_handle;
            std::string  m_log_file_name;    //the name and path of file
            std::string  m_log_dir;          //dir of log files
            std::string  m_module_name;     //module name as file name
        };
    }
} //end of namespace top
