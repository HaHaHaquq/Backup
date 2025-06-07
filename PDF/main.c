#include <stdio.h>
#include <string.h>
#include "serial.h"

// 串口初始化
void serial_init(uint32_t baudrate) {
    // 初始化串口硬件，设置波特率
    // 具体实现取决于使用的单片机型号
}

// 发送文件数据
int send_file(const char* filename, uint8_t target) {
    FILE *fp = fopen(filename, "rb");
    if(!fp) return -1;
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // 发送文件头信息
    uint8_t header[20];
    sprintf((char*)header, "SIZE:%ld", size);
    serial_send(header, strlen((char*)header));
    
    // 等待从机应答
    while(!serial_available());
    if(serial_read() != ACK) return -2;
    
    // 发送文件内容
    uint8_t buffer[256];
    while(size > 0) {
        int chunk = size > 256 ? 256 : size;
        fread(buffer, 1, chunk, fp);
        serial_send(buffer, chunk);
        size -= chunk;
    }
    fclose(fp);
    return 0;
}