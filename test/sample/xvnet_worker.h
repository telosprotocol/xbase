#pragma once
#include "xbase/xns_macro.h"
#include "xcomponent_administration/xcomponent_managers/xnetwork_component_manager.h"
#include "xdata/xchain_param.h"
#include "xvnetwork/xvnetwork_driver_face.h"

NS_BEG3(top, consensus, performance)

class xvnetwork_mgr {
public:
    xvnetwork_mgr(data::xchain_param config) : m_config(config) {}

    std::shared_ptr<vnetwork::xvnetwork_driver_face_t >
    get_vnetwork_driver();

private:
    std::shared_ptr<components::managers::xnetwork_component_manager_t> m_network_mgr{nullptr};
    data::xchain_param m_config;
    std::shared_ptr<vnetwork::xvnetwork_driver_face_t > m_driver{ nullptr };
};

NS_END3
