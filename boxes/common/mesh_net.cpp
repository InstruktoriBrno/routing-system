#include <Arduino.h>

#include "logging.hpp"
#include "mesh_net.hpp"
#include "serdes.hpp"
#include <logging.hpp>
#include "esp_mac.h"



static const char *TAG = "mesh_net";

static bool is_root = false;
static mesh_addr_t root_addr;

static esp_netif_t *netif_sta = NULL;


static bool is_mesh_connected_flag = false;
static mesh_addr_t mesh_parent_addr = {0,};
static int mesh_layer = 0;

static mesh_addr_t broadcast_address {
    .addr = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
};


bool am_i_root() {
    return is_root;
}

const mesh_addr_t* root_address() {
    return &root_addr;
}

static void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    mesh_addr_t id = {0,};
    static uint16_t last_layer = 0;

    switch (event_id) {
    case MESH_EVENT_STARTED: {
        esp_mesh_get_id(&id);
        rg_log_i(TAG, "<MESH_EVENT_MESH_STARTED>ID:" MACSTR "", MAC2STR(id.addr));
        is_mesh_connected_flag = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_STOPPED: {
        rg_log_i(TAG, "<MESH_EVENT_STOPPED>");
        is_mesh_connected_flag = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_CHILD_CONNECTED: {
        mesh_event_child_connected_t *child_connected = (mesh_event_child_connected_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_CHILD_CONNECTED>aid:%d, " MACSTR "",
                 child_connected->aid,
                 MAC2STR(child_connected->mac));

    }
    break;
    case MESH_EVENT_CHILD_DISCONNECTED: {
        mesh_event_child_disconnected_t *child_disconnected = (mesh_event_child_disconnected_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_CHILD_DISCONNECTED>aid:%d, " MACSTR "",
                 child_disconnected->aid,
                 MAC2STR(child_disconnected->mac));
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_ADD: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        rg_log_w(TAG, "<MESH_EVENT_ROUTING_TABLE_ADD>add %d, new:%d, layer:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new, mesh_layer);
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_REMOVE: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        rg_log_w(TAG, "<MESH_EVENT_ROUTING_TABLE_REMOVE>remove %d, new:%d, layer:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new, mesh_layer);
    }
    break;
    case MESH_EVENT_NO_PARENT_FOUND: {
        mesh_event_no_parent_found_t *no_parent = (mesh_event_no_parent_found_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_NO_PARENT_FOUND>scan times:%d",
                 no_parent->scan_times);
    }
    /* TODO handler for the failure */
    break;
    case MESH_EVENT_PARENT_CONNECTED: {
        mesh_event_connected_t *connected = (mesh_event_connected_t *)event_data;
        esp_mesh_get_id(&id);
        mesh_layer = connected->self_layer;
        memcpy(&mesh_parent_addr.addr, connected->connected.bssid, 6);
        rg_log_i(TAG,
                 "<MESH_EVENT_PARENT_CONNECTED>layer:%d-->%d, parent:" MACSTR "%s, ID:" MACSTR ", duty:%d",
                 last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "", MAC2STR(id.addr), connected->duty);
        // last_layer = mesh_layer;
        // mesh_connected_indicator(mesh_layer);
        is_mesh_connected_flag = true;
        // if (esp_mesh_is_root()) {
        //     esp_netif_dhcpc_stop(netif_sta);
        //     esp_netif_dhcpc_start(netif_sta);
        // }
        // esp_mesh_comm_p2p_start();
    }
    break;
    case MESH_EVENT_PARENT_DISCONNECTED: {
        mesh_event_disconnected_t *disconnected = (mesh_event_disconnected_t *)event_data;
        rg_log_i(TAG,
                 "<MESH_EVENT_PARENT_DISCONNECTED>reason:%d",
                 disconnected->reason);
        is_mesh_connected_flag = false;
        // mesh_disconnected_indicator();
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_LAYER_CHANGE: {
        mesh_event_layer_change_t *layer_change = (mesh_event_layer_change_t *)event_data;
        mesh_layer = layer_change->new_layer;
        rg_log_i(TAG, "<MESH_EVENT_LAYER_CHANGE>layer:%d-->%d%s",
                 last_layer, mesh_layer,
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "");
        last_layer = mesh_layer;
        // mesh_connected_indicator(mesh_layer);
    }
    break;
    case MESH_EVENT_ROOT_ADDRESS: {
        root_addr = *((mesh_addr_t *)event_data);
        rg_log_i(TAG, "<MESH_EVENT_ROOT_ADDRESS>root address:" MACSTR "",
                 MAC2STR(root_addr.addr));
    }
    break;
    case MESH_EVENT_VOTE_STARTED: {
        mesh_event_vote_started_t *vote_started = (mesh_event_vote_started_t *)event_data;
        rg_log_i(TAG,
                 "<MESH_EVENT_VOTE_STARTED>attempts:%d, reason:%d, rc_addr:" MACSTR "",
                 vote_started->attempts,
                 vote_started->reason,
                 MAC2STR(vote_started->rc_addr.addr));
    }
    break;
    case MESH_EVENT_VOTE_STOPPED: {
        rg_log_i(TAG, "<MESH_EVENT_VOTE_STOPPED>");
        break;
    }
    case MESH_EVENT_ROOT_SWITCH_REQ: {
        mesh_event_root_switch_req_t *switch_req = (mesh_event_root_switch_req_t *)event_data;
        rg_log_i(TAG,
                 "<MESH_EVENT_ROOT_SWITCH_REQ>reason:%d, rc_addr:" MACSTR "",
                 switch_req->reason,
                 MAC2STR( switch_req->rc_addr.addr));
    }
    break;
    case MESH_EVENT_ROOT_SWITCH_ACK: {
        /* new root */
        mesh_layer = esp_mesh_get_layer();
        esp_mesh_get_parent_bssid(&mesh_parent_addr);
        rg_log_i(TAG, "<MESH_EVENT_ROOT_SWITCH_ACK>layer:%d, parent:" MACSTR "", mesh_layer, MAC2STR(mesh_parent_addr.addr));
    }
    break;
    case MESH_EVENT_TODS_STATE: {
        mesh_event_toDS_state_t *toDs_state = (mesh_event_toDS_state_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_TODS_REACHABLE>state:%d", *toDs_state);
    }
    break;
    case MESH_EVENT_ROOT_FIXED: {
        mesh_event_root_fixed_t *root_fixed = (mesh_event_root_fixed_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_ROOT_FIXED>%s",
                 root_fixed->is_fixed ? "fixed" : "not fixed");
    }
    break;
    case MESH_EVENT_ROOT_ASKED_YIELD: {
        mesh_event_root_conflict_t *root_conflict = (mesh_event_root_conflict_t *)event_data;
        rg_log_i(TAG,
                 "<MESH_EVENT_ROOT_ASKED_YIELD>" MACSTR ", rssi:%d, capacity:%d",
                 MAC2STR(root_conflict->addr),
                 root_conflict->rssi,
                 root_conflict->capacity);
    }
    break;
    case MESH_EVENT_CHANNEL_SWITCH: {
        mesh_event_channel_switch_t *channel_switch = (mesh_event_channel_switch_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_CHANNEL_SWITCH>new channel:%d", channel_switch->channel);
    }
    break;
    case MESH_EVENT_SCAN_DONE: {
        mesh_event_scan_done_t *scan_done = (mesh_event_scan_done_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_SCAN_DONE>number:%d",
                 scan_done->number);
    }
    break;
    case MESH_EVENT_NETWORK_STATE: {
        mesh_event_network_state_t *network_state = (mesh_event_network_state_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_NETWORK_STATE>is_rootless:%d",
                 network_state->is_rootless);
    }
    break;
    case MESH_EVENT_STOP_RECONNECTION: {
        rg_log_i(TAG, "<MESH_EVENT_STOP_RECONNECTION>");
    }
    break;
    case MESH_EVENT_FIND_NETWORK: {
        mesh_event_find_network_t *find_network = (mesh_event_find_network_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_FIND_NETWORK>new channel:%d, router BSSID:" MACSTR "",
                 find_network->channel, MAC2STR(find_network->router_bssid));
    }
    break;
    case MESH_EVENT_ROUTER_SWITCH: {
        mesh_event_router_switch_t *router_switch = (mesh_event_router_switch_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_ROUTER_SWITCH>new router:%s, channel:%d, " MACSTR "",
                 router_switch->ssid, router_switch->channel, MAC2STR(router_switch->bssid));
    }
    break;
    case MESH_EVENT_PS_PARENT_DUTY: {
        mesh_event_ps_duty_t *ps_duty = (mesh_event_ps_duty_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_PS_PARENT_DUTY>duty:%d", ps_duty->duty);
    }
    break;
    case MESH_EVENT_PS_CHILD_DUTY: {
        mesh_event_ps_duty_t *ps_duty = (mesh_event_ps_duty_t *)event_data;
        rg_log_i(TAG, "<MESH_EVENT_PS_CHILD_DUTY>cidx:%d, " MACSTR ", duty:%d", ps_duty->child_connected.aid-1,
                MAC2STR(ps_duty->child_connected.mac), ps_duty->duty);
    }
    break;
    default:
        rg_log_i(TAG, "unknown id:%" PRId32 "", event_id);
        break;
    }
}

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    rg_log_i(TAG, "<IP_EVENT_STA_GOT_IP>IP:" IPSTR, IP2STR(&event->ip_info.ip));

}

static void initialize_mesh_network() {
    ESP_ERROR_CHECK(nvs_flash_init());
    /*  tcpip initialization */
    esp_netif_init();
    /*  event initialization */
    esp_event_loop_create_default();
    /*  create network interfaces for mesh (only station instance saved for further manipulation, soft AP instance ignored */
    esp_netif_create_default_wifi_mesh_netifs(&netif_sta, NULL);
    /*  wifi initialization */
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_start());
    /*  mesh initialization */
    ESP_ERROR_CHECK(esp_mesh_init());
    ESP_ERROR_CHECK(esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL));
    /*  set mesh topology */
    ESP_ERROR_CHECK(esp_mesh_set_topology(MESH_TOPO_TREE));
    /*  set mesh max layer according to the topology */
    ESP_ERROR_CHECK(esp_mesh_set_max_layer(8));
    ESP_ERROR_CHECK(esp_mesh_set_vote_percentage(1));
    ESP_ERROR_CHECK(esp_mesh_set_xon_qsize(128));
#ifdef CONFIG_MESH_ENABLE_PS
    /* Enable mesh PS function */
    ESP_ERROR_CHECK(esp_mesh_enable_ps());
    /* better to increase the associate expired time, if a small duty cycle is set. */
    ESP_ERROR_CHECK(esp_mesh_set_ap_assoc_expire(60));
    /* better to increase the announce interval to avoid too much management traffic, if a small duty cycle is set. */
    ESP_ERROR_CHECK(esp_mesh_set_announce_interval(600, 3300));
#else
    /* Disable mesh PS function */
    ESP_ERROR_CHECK(esp_mesh_disable_ps());
    ESP_ERROR_CHECK(esp_mesh_set_ap_assoc_expire(1000));
#endif
    mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
    /* mesh ID */
    memcpy((uint8_t *) &cfg.mesh_id, CONFIG_MESH_ID, 6);
    /* router */
    cfg.channel = CONFIG_MESH_CHANNEL;
    cfg.router.ssid_len = strlen(CONFIG_MESH_ROUTER_SSID);
    memcpy((uint8_t *) &cfg.router.ssid, CONFIG_MESH_ROUTER_SSID, cfg.router.ssid_len);
    memcpy((uint8_t *) &cfg.router.password, CONFIG_MESH_ROUTER_PASSWD,
           strlen(CONFIG_MESH_ROUTER_PASSWD));
    /* mesh softAP */
    ESP_ERROR_CHECK(esp_mesh_set_ap_authmode(WIFI_AUTH_WPA2_PSK));
    cfg.mesh_ap.max_connection = 9;
    cfg.mesh_ap.nonmesh_max_connection = 1;
    memcpy((uint8_t *) &cfg.mesh_ap.password, CONFIG_MESH_AP_PASSWD,
           strlen(CONFIG_MESH_AP_PASSWD));
    ESP_ERROR_CHECK(esp_mesh_set_config(&cfg));

    if (am_i_root()) {
        rg_log_i(TAG, "Forcing root");
        esp_mesh_set_type(MESH_ROOT);
    }
    esp_mesh_fix_root(1);
    esp_mesh_send_block_time(20);

    esp_mesh_set_group_id(&broadcast_address, 1);

    /* mesh start */
    ESP_ERROR_CHECK(esp_mesh_start());
    ESP_ERROR_CHECK(esp_mesh_set_self_organized(true, true));
#ifdef CONFIG_MESH_ENABLE_PS
    /* set the device active duty cycle. (default:10, MESH_PS_DEVICE_DUTY_REQUEST) */
    ESP_ERROR_CHECK(esp_mesh_set_active_duty_cycle(CONFIG_MESH_PS_DEV_DUTY, CONFIG_MESH_PS_DEV_DUTY_TYPE));
    /* set the network active duty cycle. (default:10, -1, MESH_PS_NETWORK_DUTY_APPLIED_ENTIRE) */
    ESP_ERROR_CHECK(esp_mesh_set_network_duty_cycle(CONFIG_MESH_PS_NWK_DUTY, CONFIG_MESH_PS_NWK_DUTY_DURATION, CONFIG_MESH_PS_NWK_DUTY_RULE));
#endif
    // rg_log_i(TAG, "mesh starts successfully, heap:%" PRId32 ", %s<%d>%s, ps:%d",  esp_get_minimum_free_heap_size(),
    //          esp_mesh_is_root_fixed() ? "root fixed" : "root not fixed",
    //          esp_mesh_get_topology(), esp_mesh_get_topology() ? "(chain)":"(tree)", esp_mesh_is_ps_enabled());

}


void initialize_mesh_network_as_peer() {
    is_root = false;
    initialize_mesh_network();
}

void initialize_mesh_network_as_root() {
    is_root = true;
    initialize_mesh_network();
}

uint32_t network_millis() {
    return esp_mesh_get_tsf_time() / 1000;
}

static bool handle_network_error(esp_err_t err) {
    static int fail_count = 0;
    if (err == ESP_OK) {
        fail_count = 0;
        return true;
    }

    if (err == ESP_ERR_MESH_TIMEOUT) {
        fail_count++;
        if (fail_count > 10) {
            rg_log_e(TAG, "Too many timeouts, restarting mesh network");
            esp_mesh_stop();
            initialize_mesh_network();
            fail_count = 0;
        }
    }
    rg_log_e(TAG, "Network error: %s", esp_err_to_name(err));
    return false;
}

static bool send_raw_to_root(tcb::span<uint8_t> message) {
    if (am_i_root()) {
        return false;
    }

    assert(message.size() <= MESH_MTU);

    mesh_data_t data = {
        .data = message.data(),
        .size = uint16_t(message.size()),
        .proto = MESH_PROTO_BIN,
        .tos = MESH_TOS_P2P
    };

    auto ret = esp_mesh_send(nullptr, &data, MESH_DATA_P2P, nullptr, 500);
    return handle_network_error(ret);
}

bool broadcast_raw_message(tcb::span<uint8_t> message) {
    assert(message.size() <= MESH_MTU);

    mesh_data_t data = {
        .data = message.data(),
        .size = uint16_t(message.size()),
        .proto = MESH_PROTO_BIN,
        .tos = MESH_TOS_P2P
    };

    const mesh_opt_t opt = {
        .type = MESH_OPT_SEND_GROUP,
    };

    auto ret = esp_mesh_send(&broadcast_address, &data, MESH_DATA_P2P | MESH_DATA_NONBLOCK, &opt, 1);
    return handle_network_error(ret);
}

void report_box_status(int active_round_id, const Sha256& active_round_hash,
        uint8_t round_download_progress, uint8_t game_state, uint16_t game_time,
        int8_t router_id) {
    assert(!am_i_root());
    if (!is_mesh_connected_flag) {
        rg_log_w(TAG, "Not connected to mesh network, not sending status");
        return;
    }

    mesh_addr_t parent_bssid;
    esp_mesh_get_parent_bssid(&parent_bssid);

    NodeStatusMessage msg = {
        .type = NodeType::Box,
        .parent = MacAddress(parent_bssid),
        .active_round_id = active_round_id,
        .active_round_hash = active_round_hash,
        .round_download_progress = round_download_progress,
        .game_state = game_state,
        .game_time = game_time,
        .router_id = router_id
    };

    static uint8_t tx_buffer[sizeof(msg) + 1];
    SerializationBuffer ser_buffer(tcb::span<uint8_t>(tx_buffer, sizeof(tx_buffer)));
    ser_buffer.push<uint8_t>(NodeStatusMessage::MESSAGE_TYPE);
    msg.serialize(ser_buffer);

    send_raw_to_root(ser_buffer.span());
}

bool send_packet_visit(uint16_t time, std::array<uint8_t, 7> physical_card_id, uint8_t team_id,
                       uint8_t seq_num, uint8_t router_id, std::optional<uint8_t> score)
{
    assert(!am_i_root());
    if (!is_mesh_connected_flag) {
        rg_log_w(TAG, "Not connected to mesh network, not sending packet visit");
        return false;
    }

    PacketVisitMessage msg = {
        .time = time,
        .physical_card_id = physical_card_id,
        .team_id = team_id,
        .seq_num = seq_num,
        .router_id = router_id,
        .score = score
    };

    static uint8_t tx_buffer[sizeof(msg) + 1];
    SerializationBuffer ser_buffer(tcb::span<uint8_t>(tx_buffer, sizeof(tx_buffer)));
    ser_buffer.push<uint8_t>(PacketVisitMessage::MESSAGE_TYPE);
    msg.serialize(ser_buffer);

    return send_raw_to_root(ser_buffer.span());

}

bool send_message(const MacAddress& recipient, tcb::span<uint8_t> message) {
    assert(message.size() <= MESH_MTU);

    mesh_data_t data = {
        .data = message.data(),
        .size = uint16_t(message.size()),
        .proto = MESH_PROTO_BIN,
        .tos = MESH_TOS_DEF
    };

    mesh_addr_t recipient_addr;
    memcpy(recipient_addr.addr, recipient.data(), 6);

    auto ret = esp_mesh_send(&recipient_addr, &data, MESH_DATA_P2P, nullptr, 0);
    return handle_network_error(ret);
}

template <typename Func>
bool for_each_packet_type(Func&&) {
    return false;
}

template <typename Func, typename T, typename... Ts>
bool for_each_packet_type(Func&& func) {
    if (func.template operator()<T>())
        return true;
    return for_each_packet_type<Func, Ts...>(std::forward<Func>(func));
}

void handle_incoming_messages(MessageHandler &handler) {
    esp_mesh_set_group_id(&broadcast_address, 1);

    mesh_addr_t from;
    static uint8_t recv_buffer[MESH_MTU];
    mesh_data_t data {
        .data = recv_buffer,
        .size = sizeof(recv_buffer)
    };
    int flag;
    auto ret = esp_mesh_recv(&from, &data, 0, &flag, nullptr, 0);
    if (ret != ESP_OK) {
        if (ret != ESP_ERR_MESH_TIMEOUT)
            rg_log_e(TAG, "Failed to receive mesh message: %s", esp_err_to_name(ret));
        return;
    }

    if (data.size == 0) {
        return;
    }

    DeserializationBuffer buffer(tcb::span<uint8_t>(data.data, data.size));
    uint8_t message_type;
    buffer.pop(message_type);

    auto handle_message = [&]<typename Message>() {
        if (message_type == Message::MESSAGE_TYPE) {
            auto msg = Message::deserialize(buffer);
            handler(MacAddress(from), msg);
            return true;
        }
        return false;
    };

    bool handled = for_each_packet_type<decltype(handle_message),
        NodeStatusMessage,
        RoundHeaderMessage,
        RouterDefinitionMessage,
        LinkDefinitionMessage,
        PacketDefinitionMessage,
        EventDefinitionMessage,
        PrepareGameMessage,
        GameStateMessage,
        PrepareGameMessage,
        PacketVisitMessage>(std::move(handle_message));

    if (!handled) {
        rg_log_w(TAG, "Received unknown message type: %d", message_type);
    }
}


const MacAddress& my_mac_address() {
    static MacAddress mac;
    if (mac[0] == 0) {
        esp_read_mac(mac.data(), ESP_MAC_WIFI_STA);
    }
    return mac;
}

bool is_mesh_connected() {
    return is_mesh_connected_flag;
}
