#include "ano_protocol.h"

uint8_t check_sum(uint8_t *data, uint32_t length) 
{
    uint8_t checksum = 0; // 初始化校验和为0

    // 遍历数据，逐个字节累加到校验和中
    for (uint32_t i = 0; i < length; ++i) {
        checksum += data[i];
    }

    // 返回校验和
    return checksum;
}

void check_sum_add(uint8_t *data, uint32_t length, uint8_t* check_sum_out, uint8_t* check_sum_add_out) 
{
    uint8_t checksum = 0; // 
    uint8_t addcheck = 0; // 

    // 遍历数据，逐个字节累加到校验和中
    for (uint32_t i = 0; i < length; ++i) {
        checksum += data[i];
        addcheck += checksum;
    }
    *check_sum_out = checksum;
    *check_sum_add_out = addcheck;
}
//灵活协议组帧
uint8_t packed_ano(uint8_t *out, uint8_t *in, uint32_t len) 
{
    ano_protocol_t *ano = (ano_protocol_t *)out;

    ano->head = PROTOCOL_HEAD;
    ano->dst_addr = 0;
    ano->src_addr = 0;
    ano->commond = COMMODN_LH;
    ano->data_len = len;
    memcpy(ano->data, in, len);

    check_sum_add(out, PROTICOL_NOEND_SIZE(len), &ano->data[len], &ano->data[len + 1]);

    return PROTICOL_SIZE(len);
}
