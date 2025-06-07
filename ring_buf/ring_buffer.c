#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ring_buffer.h"


#if 1
ring_fifo_t* ring_fifo_init(uint32_t size)
{
    uint8_t* ptr;
    ring_fifo_t* fifo_ptr;

    ptr = r_malloc(sizeof(ring_fifo_t));
    if (ptr == NULL)
        return 0;
    fifo_ptr = (ring_fifo_t*)ptr;
    fifo_ptr->in = 0;
    fifo_ptr->out = 0;
    fifo_ptr->size = size;

    ptr = r_malloc(size);
    if (ptr == NULL)
        return 0;
    fifo_ptr->buffer = ptr;

    return fifo_ptr;
}
#else
ring_fifo_t ring_fifo_init(uint8_t *buff,uint32_t size)
#endif

void ring_buffer_free(ring_fifo_t* ptr)
{
    free(ptr->buffer);
    free(ptr);
}
/*单字节检查缓冲区数据*/
uint32_t peek_ring_fifo_char(ring_fifo_t* fifo_ptr, uint8_t* ch)
{
    uint8_t len = 0;

    if (!fifo_ptr)
    {
        return 0;
    }
    if (!ch)
    {
        return 0;
    }

    if (fifo_ptr->in - fifo_ptr->out)
    {
        *ch = *(fifo_ptr->buffer + (fifo_ptr->out & (fifo_ptr->size - 1)));
        len = 1;
    }
    else
    {
        return 0;
    }


    return len;
}
/*单字节读取缓冲区数据*/
uint32_t read_ring_fifo_char(ring_fifo_t *fifo_ptr, uint8_t* ch)
{
    uint8_t len = 0;

    if (peek_ring_fifo_char(fifo_ptr, ch) == 0)
        return 0;
    fifo_ptr->out += len;

    return len;
}
/*单字节将数据写入缓冲区*/
uint32_t write_ring_fifo_char(ring_fifo_t* fifo_ptr, uint8_t data)
{
    uint32_t len;

    if (!fifo_ptr)
    {
        return 0;
    }

//    /*缓冲区有空闲空间*/
//    if (fifo_ptr->size - fifo_ptr->in + fifo_ptr->out)
//    {
    /*不管是否有空间直接写入，无用空间数据直接覆盖*/
        *(fifo_ptr->buffer + (fifo_ptr->in & (fifo_ptr->size - 1))) = data;
        len = 1;
        fifo_ptr->in += len;
//    }
//    else
//    {
//        return 0;
//    }

    return len;
}
/*获取缓冲区已使用空间大小*/
uint32_t get_ring_buff_used(ring_fifo_t* fifo_ptr)
{
    // 需要注意in和out的循环关系，当in超过out时，需要减去size来得到正确的差值
    if (fifo_ptr->in >= fifo_ptr->out) {
        return (fifo_ptr->in - fifo_ptr->out);
    } else {
        return (fifo_ptr->size - fifo_ptr->out + fifo_ptr->in);
    }
}
/*获取缓冲区剩余空间大小*/
uint32_t get_ring_buff_have(ring_fifo_t* fifo_ptr)
{
     // 剩余空间 = 总大小 - 已使用空间
    return (fifo_ptr->size - get_ring_buff_used(fifo_ptr));
}
/*多字节检查缓冲区数据*/
uint32_t peek_ring_fifo(ring_fifo_t* fifo_ptr, uint8_t* buffer, uint32_t len)
{
    uint32_t over_len;
    
    if (!fifo_ptr)
    {
        return 0;
    }
    if (!buffer)
    {
        return 0;
    }

    /*判断读取字节长度是否超出已有长度*/
    len = min(len, fifo_ptr->in - fifo_ptr->out);

    /*获取输出输出数据直到缓冲区尾部*/
    over_len = min(len,fifo_ptr->size - (fifo_ptr->out & (fifo_ptr->size - 1)));

    /*拷贝数组直到缓冲区尾*/
    memcpy(buffer, fifo_ptr->buffer + (fifo_ptr->out & (fifo_ptr->size - 1)), over_len);
    /*拷贝剩余数据*/
    memcpy(buffer + over_len, fifo_ptr->buffer, len - over_len);

    return len;
}
/*多字节读数据*/
uint32_t read_ring_fifo(ring_fifo_t *fifo_ptr, uint8_t* buffer, uint32_t len)
{
    if (peek_ring_fifo(fifo_ptr, buffer, len) == 0)
        return 0;

    fifo_ptr->out += len;

    return len;
}

/*多字节写入缓冲区数据*/
uint32_t write_ring_fifo(ring_fifo_t* fifo_ptr, uint8_t* buffer, uint32_t len)
{
    uint32_t over_len;

    if(!fifo_ptr)
    {
        return 0;
    }
    if (!buffer)
    {
        return 0;
    }

    len = min(len, fifo_ptr->size - fifo_ptr->in + fifo_ptr->out);

    /*获取输入字节长度直到缓冲区尾部*/
    over_len = min(len, fifo_ptr->size - (fifo_ptr->in & (fifo_ptr->size - 1)));
    /*拷贝数据直到缓冲区尾部*/
    memcpy(fifo_ptr->buffer + (fifo_ptr->in & (fifo_ptr->size - 1)), buffer, over_len);
    /*从头部拷贝剩余数据*/
    memcpy(fifo_ptr->buffer, buffer + over_len, len - over_len);

    fifo_ptr->in += len;

    return len;
}
