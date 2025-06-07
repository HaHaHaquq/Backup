#include <stdio.h>
#include <string.h>
#include "platform_api.h"
#include "att_db.h"
#include "gap.h"
#include "btstack_event.h"
#include "btstack_defines.h"
#include "raw_pkt.h"

#include "profile.h"
#include "freq_offset.h"
#include "rf_test.h"
#include "uart_golden_DUT.h"

static void user_msg_handler(uint32_t msg_id, void *data, uint16_t size)
{
    switch (msg_id)
    {
        case USER_Tx_RAW_PKT:
            raw_pkt_tx_test();
            break;
        case USER_Rx_RAW_PKT:
            raw_pkt_rx_test();
            break;
        case USER_SET_CHANNEL:
            // set_rf_channel(data);
            break;
        case USER_SET_RX_PKT:
            raw_packet_set_rx_param();
            break;
        case USER_SET_TX_PKT:
            raw_packet_set_tx_param();
            break;
        case USER_END_TEST:
            tf_test_rx_pkt_end();
            break;
        case USER_MSG_FREQ_OFFSET_BEGIN_RX:
            freq_offset_rx_begin_do();
            break;
        case USER_MSG_FREQ_OFFSET_DONE_RX:
            freq_offset_rx_done();
            // freq_offset_rx_begin_do();
            break;
        case USER_MSG_FREQ_OFFSET_END_RX:
            rf_test_freq_offset_end(*(int8_t *)data);
            break;
        case USER_MSG_FREQ_OFFSET_TUNE:
            DUT_cmd_rf_freq_offset_tune(*(uint8_t *)data);
            break;
    default:
        ;
    }
}

static void user_packet_handler(uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size)
{
    static const bd_addr_t rand_addr = { 0xDA, 0xC3, 0x2B, 0x35, 0x24, 0x3B };
    uint8_t event = hci_event_packet_get_type(packet);
    const btstack_user_msg_t *p_user_msg;
    if (packet_type != HCI_EVENT_PACKET) return;

    switch (event)
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
            break;
        
        freq_test_init();
        raw_pkt_malloc_init();	
        break;


    case BTSTACK_EVENT_USER_MSG:
        p_user_msg = hci_event_packet_get_user_msg(packet);
        user_msg_handler(p_user_msg->msg_id, p_user_msg->data, p_user_msg->len);
        break;

    default:
        break;
    }
}

static btstack_packet_callback_registration_t hci_event_callback_registration;

uint32_t setup_profile(void *data, void *user_data)
{
    platform_printf("setup profile\n");
    // Note: security has not been enabled.
    hci_event_callback_registration.callback = &user_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(&user_packet_handler);
    return 0;
}

