#include "xvnet_worker.h"
#include "xbasic/xns_macro.h"
#include "xchaininit/xinit.h"
#include "xdata/xchain_param.h"
#include "xmock_biz_obj.h"
#include "xconsensus/xconsensus_adapter.h"
#include <atomic>
static std::atomic<int> s_atomic_suc(0);
static std::atomic<int> s_atomic_fail(0);

uint32_t test_pbft_callback(top::consensus::xbiz_callback_obj callback_obj) {
    if (callback_obj.m_result == 0) {
        s_atomic_suc++;
    }
    else {
        s_atomic_fail++;
    }
    if (((uint32_t)s_atomic_suc + (uint32_t)s_atomic_fail) % 100 == 0)
        xinfo("consensus result[%d:%d]", (uint32_t)s_atomic_suc, (uint32_t)s_atomic_fail);
    return 0;
}

int main(int argc, char * argv[]) {
    std::cout << "test pbft" << std::endl;
    string configfile = argv[1];

    top::data::xchain_param config;
    if (!top::parse_params(configfile, &config)) {
        std::cout << "config invalid!!!! please check config file:" << configfile << " or parameters!";
        return -1;
    }

    xinit_log(config.cfg_log_path.c_str(), true, true);
    xset_log_level((enum_xlog_level)config.cfg_log_level);
    xinfo("===xtopchain start here===");

    top::consensus::performance::xvnetwork_mgr mgr(config);

    std::shared_ptr<top::vnetwork::xvnetwork_driver_face_t > vnet_driver = mgr.get_vnetwork_driver();

    assert(nullptr != vnet_driver);

    if (vnet_driver->type() != top::vnetwork::xvnode_type_t::consensus) {
        std::cout << "not consensus node, wait forever" << endl;
        ::sleep(100000000);
    }
    uint64_t test_num = atoll(argv[2]);
    std::cout << "test pbft num " << test_num << std::endl;
    std::cout << "sleep wait" << std::endl;
    ::sleep(10);
    std::cout << "sleep await" << std::endl;
    top::consensus::xconsensus_object_params params;
    params.m_net_host = vnet_driver.get();
    params.m_state_machine_type = top::consensus::BT_shard_alone_consensus;
    params.m_least_node_num = 2;

    top::consensus::xconsensus_object_id obj_id;
    top::consensus::consensus_adapter::get_instance()->create_consensus_object(params, obj_id);
    top::consensus::consensus_adapter::get_instance()->register_biz_type_notify_handler(obj_id, 12, test_pbft_callback);

    xinfo("[push consensus %d start]", test_num);
    for (uint64_t i = 0; i < test_num; i++) {
        std::shared_ptr<top::consensus::xconsensus_object_face> biz_obj = std::make_shared<top::consensus::performance::mock_xconsensus_object>(i);
        assert(nullptr != biz_obj);
        top::consensus::consensus_adapter::get_instance()->start_consensus(obj_id, biz_obj);
    }
    xinfo("[push consensus %d end]", test_num);
    std::cout << "push " << test_num << " consensus requests " << std::endl;
    ::sleep(100000000);
    return 0;
}
