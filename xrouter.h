// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xendpoint.h"
#include "xgateway.h"

namespace  top
{
    namespace base
    {
        //xwrouter_t should have a flow-table to control how to routing, item of flow table is like [from_xip,to_xip,action,xgateway_t ptr]
        //each item on flow_table may point to gateway object,or not
        class xwrouter_t : public xendpoint_t   //xwrouter_t dispatch packet to different network, it works as like traditional router
        {
        protected:
            xwrouter_t(xcontext_t & context,const int32_t thread_id);
            virtual ~xwrouter_t();
        private:
            xwrouter_t();
            xwrouter_t(const xwrouter_t &);
            xwrouter_t & operator = (const xwrouter_t &);
        protected:
            virtual bool   on_object_close() override; //notify the subclass the object is closed
        };
    }
}
