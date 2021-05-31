// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <atomic>
#include "xbase.h"

namespace top
{
    namespace base
    {
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
    } //end of namespace base
} //end of namespace top




 
