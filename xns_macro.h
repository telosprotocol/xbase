// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase/xcxx_config.h"

# define NS_BEG()    namespace {
# define NS_END      }

# define NS_BEG1(NS)     namespace NS {
# define NS_END1                      }

#if defined XCXX17

# define NS_BEG2(NS1, NS2)  namespace NS1::NS2 {
# define NS_END2            }

# define NS_BEG3(NS1, NS2, NS3)  namespace NS1::NS2::NS3 {
# define NS_END3                 }

# define NS_BEG4(NS1, NS2, NS3, NS4) namespace NS1::NS2::NS3::NS4 {
# define NS_END4                     }

# define NS_BEG5(NS1, NS2, NS3, NS4, NS5) namespace NS1::NS2::NS3::NS4::NS5 {
# define NS_END5                          }

#else

# define NS_BEG2(NS1, NS2)   NS_BEG1(NS1) NS_BEG1(NS2)
# define NS_END2             NS_END1 NS_END1

# define NS_BEG3(NS1, NS2, NS3)  NS_BEG2(NS1, NS2) NS_BEG1(NS3)
# define NS_END3                 NS_END1 NS_END2

# define NS_BEG4(NS1, NS2, NS3, NS4) NS_BEG3(NS1, NS2, NS3) NS_BEG1(NS4)
# define NS_END4                     NS_END1 NS_END3

# define NS_BEG5(NS1, NS2, NS3, NS4, NS5) NS_BEG4(NS1, NS2, NS3, NS4) NS_BEG1(NS5)
# define NS_END5                          NS_END1 NS_END4

#endif
