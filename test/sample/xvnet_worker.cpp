#include "xvnet_worker.h"

NS_BEG3(top, consensus, performance)

std::shared_ptr<vnetwork::xvnetwork_driver_face_t>  xvnetwork_mgr::get_vnetwork_driver()
{

    if (nullptr != m_driver) {
        return m_driver;
    }

    m_network_mgr = std::make_shared<components::managers::xnetwork_component_manager_t>(nullptr,
                                                                             network::xnode_id_t{ m_config.account },
                                                                             m_config.node_role_type,
                                                                             common::xnetwork_id_t{ m_config.network_id },  // TODO(bluecl):
                                                                             m_config.msg_port,
                                                                             m_config.dht_port,
                                                                             m_config.ip);
    assert(nullptr != m_network_mgr);

    std::vector<network::xdht_node_t> seeds{
        network::xdht_node_t{ network::xnode_id_t{ u8"T_edge" }, network::xendpoint_t{ m_config.seeds[0], m_config.dht_port } }
    };

    // TODO: bootstrap into p2p network, if the node doesn't pass the capability test, it should
    // TODO: be removed from the p2p network
    m_network_mgr->virtual_host()->bootstrap(seeds);

    vnetwork::xvnetwork_construction_data_t vnet_construction_data{ common::xtop_mainnet_id, vnetwork::xversion_t{ 0 } };
    vnet_construction_data.add_cluster_info(vnetwork::xrec_cluster_address);
    vnet_construction_data.add_cluster_info(vnetwork::xedge_cluster_address);
    vnetwork::xcluster_address_t advance_cluster_addr{ common::xtop_mainnet_id, common::xtop_default_zid, common::xcluster_id_t{ 2 }, vnetwork::xvnode_type_t::advance };
    vnetwork::xcluster_address_t consensus_cluster_addr{ common::xtop_mainnet_id, common::xtop_default_zid, common::xcluster_id_t{ 3 }, common::xgroup_id_t{ 2 }, vnetwork::xvnode_type_t::consensus };
    vnetwork::xcluster_address_t zec_cluster_addr{ common::xtop_mainnet_id, common::xtop_default_zid, common::xtop_zec_cid, vnetwork::xvnode_type_t::zec };
    vnetwork::xcluster_address_t archive_cluster_addr{ common::xtop_mainnet_id, common::xtop_default_zid, common::xtop_archive_cid, vnetwork::xvnode_type_t::archive };
    vnet_construction_data.add_cluster_info(advance_cluster_addr);
    vnet_construction_data.add_cluster_info(archive_cluster_addr);
    vnet_construction_data.add_cluster_info(consensus_cluster_addr);
    vnet_construction_data.add_cluster_info(zec_cluster_addr);
    vnet_construction_data.insert(network::xnode_id_t{ "T_edge" }, vnetwork::xedge_cluster_address);
    vnet_construction_data.insert(network::xnode_id_t{ "T_adv1" }, advance_cluster_addr);
    vnet_construction_data.insert(network::xnode_id_t{ "T_adv2" }, advance_cluster_addr);
    vnet_construction_data.insert(network::xnode_id_t{ "T_con1" }, consensus_cluster_addr);
    vnet_construction_data.insert(network::xnode_id_t{ "T_con2" }, consensus_cluster_addr);
    vnet_construction_data.insert(network::xnode_id_t{ "T_con3" }, consensus_cluster_addr);

    auto new_vnetwork_drivers = m_network_mgr->virtual_host()->build_vnetwork(vnet_construction_data);
    for (auto const & vnet_driver : new_vnetwork_drivers) {
        auto net_comp_mgr = std::dynamic_pointer_cast<components::managers::xnetwork_component_manager_t>(m_network_mgr);
        assert(net_comp_mgr);

        auto net_comp = net_comp_mgr->create_component(std::addressof(vnet_driver));
    }
    assert(1 == new_vnetwork_drivers.size());

    m_driver = new_vnetwork_drivers.at(0);

    assert(nullptr != m_driver);
    
    m_network_mgr->start();

    return m_driver;

}
NS_END3
