// Copyright (c) 2018-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase.h"

namespace top
{
    namespace base
    {
        //x25519ECDH_t is for Curve25519 ECDH
        class x25519ECDH_t
        {
        public:
            static bool create_xdf_key_pair(uint8_t my_public_key[32], uint8_t my_private_key[32]); //diffie_hellman public and private key pair
            
            //generate shared secret by self'private key + peer public key according diffie_hellman
            static bool create_xdf_shared_secret(uint8_t my_private_key[32],uint8_t peer_public_key[32],uint8_t out_shared_secret_key[32]);
        };
    }
};


