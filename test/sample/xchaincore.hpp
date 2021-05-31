// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef xchaincore_hpp
#define xchaincore_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include "xint.h"
#include "xdata.h"
#include "xobject.h"

namespace  top
{
    namespace chain
    {
        //main state of account
        class xaccount_mstate
        {
        protected: //help to quickly tracking & load related stuff
            uint64_t    m_last_unit_number;     //height or number,0 is invalid, 1 is for genius unit
            uint256_t   m_last_unit_hash;       //point the last unit that might be full_unit or light_unit
            uint64_t    m_last_full_unit_number;//height or number,0 is invalid, 1 is for genius unit
            uint256_t   m_last_full_unit_hash;  //point the last full_unit
            
            //user fire each transactions linked as a chain
            uint64_t    m_last_send_trans_number; //heigh or number of transaction submited by account,0 is invalid but 1 fo genius trans
            uint256_t   m_last_send_trans_hash; //all transaction fired by account, they are construct a chain
            
            //consensus mechanisam connect each received transaction as a chain
            uint64_t    m_last_recv_trans_number; //heigh or number of transaction submited by account,0 is invalid
            uint256_t   m_last_recv_trans_hash;   //all receipt contruct a mpt tree, m_account_address.root point to storage object
            
            uint256_t   m_genius_unit_hash;        //the first unit for this account
            uint256_t   m_genius_send_trans_hash;  //m_account_address.genius_hash point to transaction object
            
        protected: //note: the below properties are not allow to be changed by outside,it only be changed by a valid unit indirectly
            std::string m_account_address;      //m_account_address should encode account type(like smartcontract,gateway...,or for testnet)
            std::string m_account_name;         //not unique globally, it just a name comments to pair with account
            uint64_t    m_account_balance;      //token balance,
            int64_t     m_account_credit;       //credit score,it from the contribution for chain/platform/network etc
            uint64_t    m_account_nonce;        //account 'latest nonce,increase atomic
            uint64_t    m_account_create_time;     //when the account create
            int32_t     m_account_status;       //status for account like lock,suspend etc
            uint256_t   m_account_code_hash;    //if it is smartcontract account,m_account_address.code_hash point to storage object
            
        protected: //organize accounts
            std::string m_parent_account;       //who hold this account,for smartcontract it is the account who deploy code
            std::vector<std::string>  m_child_accounts;//child accounts' address,it allow have multiple child account as different function like Vote,Trading,Governouse.... e.g. one master account generate multiple key-pair by HD Wallet,each key-pair is assigned to one account.
        };
        
        //at TOP, each account(even contract account) is a state-machine that input by transaction(event) + last account'state -> ouput its own unit(new state)
        //each transaction link together as transaction-chain from genesis transaction(create_account)
        //each unit link together as unit-chain from genesis unit(create_account -> genesis unit)
        //the account get the final & consistent states by replaying every transaction from genesis state. transaction encode as action log (or bin-log)
        //the account get the final & consistaence states by go through every unit from genesis unit. unit encode as state-log(or data-log)
        //to prune data at chain, combine states(units) to one full-unit(account full state) by every 64／128 unit or as need.
        //
        class xunit_header
        {
            /*when happen for whom*/
            //at when
            uint64_t    m_timestamp;            //time to generate this unit
            uint64_t    m_zone_election_round;  //which round of election
            uint64_t    m_shard_election_round; //which round of election
            //at where
            uint64_t    m_shard_xid;            //where happen transaction & generate unit,refer XID definition
            
            /* account' last state*/
            uint256_t	m_prev_unit_hash;       //the previous unit'hash
            uint64_t    m_prev_unit_height;     //current unit hight = m_prev_unit_height + 1
            
            /*input transactions*/
            uint16_t    m_transactions_count;    //total how many transactions,by which contruct to m_transactions_root
            uint256_t	m_transactions_root;     //root of  merkle tree for transactions
            
            /*ouput new state after execute transactions*/
            uint64_t    m_random_nonce;         //each unit has random nounce
            uint64_t    m_account_nonce;        //the latest nonce of account
            uint8_t     m_unit_status;          //locked ,pending,or other status
            
            /*final orgnaized into unit structure*/
            uint8_t		m_unit_type;         //distinguish fullunit, lightunit, must set, or failed
            uint8_t     m_unit_flags;        //
            uint8_t     m_unit_version;      //what is unit version,as compatible design
            uint32_t    m_unit_size;         //total unit size
            uint256_t   m_unit_hash;         //unit digest, all elements
        };
        
        class xlightunit_t : public base::xdataobj_t, public xunit_header
        {
            //for who
            uint64_t    m_unit_owner_hash;      //who is owner(account address) for the unit, or which account address unit is pointed to
            
            /*new state*/
            int64_t     m_balance_change;       //deposit or deduct,or 0 as nothing change
            uint256_t   m_modify_property_root; //optional,root of mkerle tree, element store MPT root of the changed property
            
            /* account' last state*/
            uint64_t    m_last_fullunit_hash64;  //most recent full_unit hash
            
            /*who produce ,verify & audit*/
            uint64_t    m_unit_producer_hash;   //who is produce this unit, at PBFT it is leader ' account address
            uint64_t    m_unit_auditor_hash;    //who audit this unit
            uint256_t   m_signatures_root;      //who signature & verify
        };
        
        //at a lot case, just need synchornize xfullunit_header
        class xfullunit_header : public xunit_header
        {
        private:
            /*who produce ,verify & audit*/
            std::string     m_unit_producer;            //who is produce this unit, at PBFT it is leader ' account address
            std::string     m_unit_auditor;             //who audit this unit
            std::vector<std::string>	m_signatures;   //a array of who signature for this unit
        };
        
        class xfullunit_t : public base::xdataobj_t,public xfullunit_header
        {
            xaccount_mstate m_account_state;        //main state
        protected:
            uint256_t       m_unit_root;            //root of mkerly tree ,and each element is light unit 'hash
            uint256_t       m_property_root;        //root of mkerly tree ,but each element is a MPT root
        private:
            //TODO, add merkle tree / merkle patricia tree
        };

        
        //account is a container and also is the final state after all units replay from genius unit,or from last full_unit
        //account object has some hash property,we need convert hash to full storage key by : "m_account_address.hash" to load them
        //all data of xaccount_t are from units which generate properties, functions.
        //note: "account_uri" is like global dns name at globally unique and map to the associated account_address, we may introduce this feature later
        class xaccount_t : public base::xdataobj_t,public xaccount_mstate
        {
        public:
            enum{enum_obj_type = enum_xdata_type_max - 1};
        public:
            xaccount_t()
             : base::xdataobj_t((enum_xdata_type)enum_obj_type)
            {
                m_obj_key = m_account_address; //m_account_address is key
            }
        protected:
            virtual int32_t    do_write(base::xstream_t & stream) override        //serialize whole object to binary
            {
                const int32_t begin_size = stream.size();
                
                //TODO
                
                const int32_t end_size = stream.size();
                return (end_size - begin_size);
            }
            virtual int32_t    do_read(base::xstream_t & stream) override //serialize from binary and regeneate content of xdataobj_t
            {
                const int32_t begin_size = stream.size();
  
                //TODO
                
                const int32_t end_size = stream.size();
                return (begin_size - end_size);
            }

        private: //raw data & units
            //raw properties that construct the above fileds
            std::map<std::string,int32_t>           m_properties;  //[Property Name as key,Property' Type]
            //raw functions reference
            std::map<std::string,std::string>       m_functions;   //[Function Name as key, Function Code' HASH(256bit)]
            //raw unit generate raw functions and properties
            std::unordered_map<std::string,std::string> m_account_units; //[unit-height, unit 256bit HASH],all or partial units of this account
         };
        
        class xactiont_t
        {
            uint32_t           action_hash;   //whole action content'xxhash32
            uint16_t           action_type;   //type
            uint16_t           action_size;   //total size for whole action object

            std::string        account_addr;  //account address must embedded network info(e.g. testnet, mainnet)
            std::string        action_name;
            std::string        action_param;  //
            std::string        action_authorization; //it might empty if container already has authorization
        };
        
        //fixed 80 bytes
        class xtransaction_header
        {
            uint64_t          to_account_id;        //xxhash64(account_address)
            uint64_t          from_account_id;      //xxhash64(account_address)
            uint8_t           to_network_id;        //transaction send to target network
            uint8_t           from_network_id;      //where is from transaction
            
            uint16_t          transaction_type;     //transfer,withdraw,deposit etc
            uint16_t          transaction_len;      //max 64KB
            uint16_t          expire_duration;      //seconds(max 18hour), expired when reach (fire_timestamp + expire_duration)
            uint64_t          fire_timestamp;       //GMT
            
            uint32_t          trans_random_nounce;  //random input
            uint32_t          hash_work_proof;      //PoW proof

            uint64_t          last_unit_hight;      //depends transaction type,last_unit_hight force to match the lates one or just be one of recent 4096 unit
            uint64_t          last_unit_hash;       //pair with last_unit_hight
            uint64_t          last_trans_nonce;     //pair with last last_trans_hash
            uint64_t          last_trans_hash;      //point to the xxhash64 of the last valid message(e.g. transaction) ,it must be restrictly match as chain ledger
        };
        
        class xtransaction_t  : public base::xdataobj_t, public xtransaction_header
        {
            xactiont_t        source_action;        //source(sender) 'action
            xactiont_t        target_action;        //target(receiver)'action
            uint256_t         transaction_hash;     //256 digest for signature as safety
            std::string       authorization;        //signature for whole transaction
        };
        
        /////........key transaction
        class xcreate_account_transaction : public xtransaction_t
        {
        };
        class xset_property : public xtransaction_t
        {
        };
        class xtransfer_asset_out : public xtransaction_t
        {
        };
        class xtransfer_asset_in : public xtransaction_t
        {
        };
        /////........
        
        //xmultransaction_t is a group of transaction
        class xmultransaction_t : public base::xdataobj_t
        {
            uint16_t                    expire_duration;      //seconds(max 18hour), expired when reach (fire_timestamp + expire_duration)
            uint64_t                    fire_timestamp;       //GMT
            uint32_t                    random_nounce;        //random input
            uint32_t                    trans_work_proof;     //PoW proof
            std::vector<xtransaction_t> transactions;
            uint256_t                   transactions_digest;  //256 digest for signature for whole multransaction as safety
            std::string                 authorization;        //signature for whole transaction
        };
        
        //all unit/table input + last block ->new block
        //at many case just need syn block header quickly, in otherword xblock_header construct a light chain
        //TODO, we might need add bloom filter to quickly findout whether some account is in some block/unit
        class xblock_header : public base::xdataobj_t
        {
        protected:
            //who, for zone/table block,  m_block_target_id = [network_id][zone index][table_index]
            std::string m_block_owner;          //generate block for who,depends block type & flags

            /*when happen and where*/
            //at when
            uint64_t    m_timestamp;            //time to generate this block
            uint64_t    m_zone_election_round;  //which round of election
            uint64_t    m_shard_election_round; //which round of election
            //at where
            uint64_t    m_producer_xid;          //where happen transaction & generate unit/block,refer XID definition
            
            /* last state*/
            uint64_t    m_prev_block_height;     //current block hight = m_prev_block_height + 1
            uint256_t	m_prev_block_hash;       //the previous unit'hash
            
            /*input new */
            uint16_t    m_input_event_count;    //event could be transaction, unit, table or ...,it could be 0
            uint256_t	m_input_event_root;     //root of merkle tree for events,depends block 'type
            uint16_t    m_input_receipt_count;  //receipt to prove & confirm payment,it could be 0
            uint256_t   m_input_receipt_root;   //root of merkle tree for events,depends block 'type
            
            /*ouput new state after execute transactions*/
            uint64_t    m_random_nonce;         //each block has random nounce
            uint256_t	m_items_root;           //merkle tree for states(unit or table as item etc)
            //so far we not use m_storage_root
            //uint256_t   m_storage_root;       //merkle tree for storage root of every account(modify recently),element is account'property root hash
            
            /*power & stake & credit count*/
            uint64_t    m_count_powers;         //how much power counted to protect blocks from genius block until this block
            uint64_t    m_count_stake;          //how many stake counted to protect blocks from genius block until this block
            uint64_t    m_count_credit;         //how many credtis counted to protect blocks from genius block until this block
            
            uint64_t    m_hash_difficulty;      //Pow Hash difficulty
            uint256_t   m_work_proof;           //PoW proof(might empty,depends on block type & flag),Hash with m_random_nonce
            
            /*orgnaized into block structure*/
            uint8_t		m_block_type;          //
            uint8_t     m_block_status;        //locked ,pending,or other status
            uint8_t     m_block_flags;         //
            uint8_t     m_block_version;       //what is unit version,as compatible design
            uint32_t    m_block_size;          //total unit size
            uint256_t   m_block_header_hash;   //unit digest for  header elements
            uint256_t   m_block_body_hash;     //unit digest for  body elements
            
            /*who produce ,verify & audit*/
            std::string m_block_producer;      //who is produce this table, at PBFT it is leader ' account address
            std::string m_block_auditor;       //who audit this table
            //note:sign(m_block_header_hash+m_block_body_hash)
            std::vector<std::string> m_block_signatures;   //a array of who signature for this block header.
        };
        
        //input all unit'hash under this table
        class xtableblock_t : public xblock_header
        {
            //reference
            uint64_t    m_parent_zone_block_number;    //reference the last zone_block number(height)
            uint256_t   m_parent_zone_block_hash;      //zone block '256 HASH,paired with m_parent_zone_block_number
            uint256_t   m_genesis_table_block_hash;    //genesis table block for m_zone_id.m_table_id
        
        public://state tree, storage tree, unit tree,receipt tree
            //TODO, add Merkle tree / Merkle Particia Tree here
        };
        
        //input all table 'hash under this table
        class xbookblock_t : public xblock_header
        {
            uint256_t   m_genesis_zone_block_hash;          //genesis zone block
        public: //table block tree
            //TODO, add Merkle tree / Merkle Particia Tree here
        };
        
    } //end of namespace of base
};//end of namespace of top

#endif /* xchaincore_hpp */
