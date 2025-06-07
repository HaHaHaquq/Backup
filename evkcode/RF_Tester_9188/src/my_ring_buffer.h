#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif
/*注意初始化ringbuffer的buf长度必须为2的幂*/

#include "stdint.h"

/*判断宏，防止数据越界*/
#define min(x,y)    ((x) < (y) ? (x) : (y))
typedef struct
{
    uint8_t* buffer;/*缓冲区数组指针*/
    uint32_t size;/*环形队列使用的数组大小*/
    volatile uint32_t in;/*指示添加到缓冲区buff的标志位*/
    volatile uint32_t out;/*指示从缓冲区取出数据的标志位*/
}ring_fifo_t;

typedef enum {
    FIFO_OK = 0,          // Operation succeeded
    FIFO_ERR_PARM = 1,    // Null pointer error (invalid parameters)
    FIFO_ERR_MEM = 2,     // Memory allocation error
    FIFO_ERR_EMPTY = 3,     // Buffer empty error
    FIFO_ERR_FULL = 4,      // Buffer full error
    FIFO_ERR_INTERNAL = 5 // Internal error (reserved for future use)
} fifo_status_t;

/*内存分配定义函数*/
#define r_malloc    malloc

//size must be power of 2
ring_fifo_t* ring_fifo_init(uint32_t size);
void ring_buffer_free(ring_fifo_t* ptr);
fifo_status_t ring_buffer_reset(ring_fifo_t* ptr);
fifo_status_t peek_ring_fifo_char(ring_fifo_t* fifo_ptr, uint8_t* ch);
fifo_status_t read_ring_fifo_char(ring_fifo_t* fifo_ptr, uint8_t* ch);
fifo_status_t write_ring_fifo_char(ring_fifo_t* fifo_ptr, uint8_t data);
uint32_t get_ring_buff_used(ring_fifo_t* fifo_ptr);
uint32_t get_ring_buff_have(ring_fifo_t* fifo_ptr);
fifo_status_t read_ring_fifo(ring_fifo_t* fifo_ptr, uint8_t* buffer, uint32_t len);
fifo_status_t peek_ring_fifo(ring_fifo_t* fifo_ptr, uint8_t* buffer, uint32_t len);
fifo_status_t write_ring_fifo(ring_fifo_t* fifo_ptr, uint8_t* buffer, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif 