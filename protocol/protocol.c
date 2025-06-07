#include "protocol.h"

uint16_t crc16(const void *addr, uint32_t len)
{
    int i;
    uint16_t crc = 0;                     // CRC16-IBM初始异或值
    uint32_t length = len;                // 使用中间变量来存储长度

    while (length > 0)                    // 当剩余长度大于0时继续循环
    {
        crc = crc ^ (*(const char*)addr++ << 8); // 获取一个字节并将其与CRC高字节进行异或操作
        for (i = 0; i < 8; i++)           // 准备对8位进行旋转
        {
            if (crc & 0x8000)             // 如果最高位（b15）是1...
                crc = (crc << 1) ^ POLY;  // 左移一位并与多项式进行异或操作
            else                          // 如果最高位（b15）是0...
                crc <<= 1;                // 只进行左移操作
        }
        length--;                         // 减少剩余长度
    }

    return crc;                           // 返回更新后的CRC值
}


uint8_t checksum(uint8_t * buff, uint8_t len)
{
  uint8_t sum=0,num=0;
  for(num=0;num<len;num++)
  {
    sum=sum+buff[num];
  }
  return sum;
}

// 函数用于解析帧
protocol_t *frame_parse(char *data_ptr, uint32_t len) {
    protocol_t *protocol = NULL;
    uint8_t *header_ptr = NULL;
    uint8_t header_value = 0;
    uint32_t header_index = 0;
    uint32_t length = 0;
    uint16_t crc16_out = 0;

    if(data_ptr == NULL || len == 0) return NULL;
    if(len < FARM_MIN_LEN) return NULL; // 帧头长度加上protocol_t结构体大小

    length = len;
    // 遍历数据，寻找帧头 剩余数据长度大于最小满足长度
    while (length >= FARM_MIN_LEN) {
        // 读取帧头
        header_value = *(const uint8_t *)(data_ptr + header_index);

        // 检查帧头是否匹配
        if (header_value == HEAD) {

            // 找到帧头
            header_ptr = data_ptr + header_index;
            protocol = (protocol_t *)header_ptr;
            if(length < FARM_MIN_LEN + protocol->len)
                return NULL; //数据长度短不满足一帧数据要求
            //crc校验
            crc16_out = crc16(&protocol->head, protocol->len + sizeof(protocol_t));
            //校验成功输出
            if(crc16_out == *((uint16_t*)&protocol->data[protocol->len - 2]))
                return protocol;
            else
                return NULL;
        }

        // 继续查找下一个可能的帧头
        header_index += 1;
        length -= 1;
    }

    // 没有找到帧头
    return NULL;
}

int frame_packed(void *out, const void *in,uint32_t cmd, uint32_t len)
{
    uint8_t* out_ptr = (uint8_t *)out;
    uint8_t* in_ptr = (uint8_t *)in;
    uint16_t crc16_out = 0;
    uint8_t head = HEAD;
    uint8_t tail = TAIL;
    uint32_t index = 0;
    uint32_t data_len = len + 2;//crc16长度

    if(out == NULL) return -1;
    if(in == NULL) return -1;

    memcpy(out_ptr + index, &head, sizeof(head));
    index += sizeof(head);
    memcpy(out_ptr + index, &cmd, sizeof(cmd));
    index += sizeof(cmd);
    memcpy(out_ptr + index, &data_len, sizeof(data_len));
    index += sizeof(len);
    memcpy(out_ptr + index, in_ptr, len);
    index += len;

    crc16_out = crc16(out_ptr, index);
    memcpy(out_ptr + index, crc16_out, sizeof(crc16_out));
    index += sizeof(crc16_out);
    memcpy(out_ptr + index, &tail, sizeof(tail));
    index += sizeof(tail);

    return index;
}

int check_parse_data(const void *in, uint32_t len)
{
    protocol_t *pase_protocl = NULL;
    char send_data_buffer[100];
    char in_buffer[100];
    int send_len = 0;
    
    if(in == NULL) return -1;

    pase_protocl = frame_parse(in,len);
    if(pase_protocl == NULL) return -1;

    switch (pase_protocl->cmd)
    {
        case START_UPLOAD:
            
        break;
    
        default:
        break;
    }

    return 1;

}
