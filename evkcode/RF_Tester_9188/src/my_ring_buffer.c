#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_ring_buffer.h"


static uint8_t is_power_of_two(uint32_t x) {
    return (x != 0) && ((x & (x - 1)) == 0);
}

ring_fifo_t* ring_fifo_init(uint32_t size)
{
    uint8_t* ptr;
    ring_fifo_t* fifo_ptr;

    if (!is_power_of_two(size)) {
        return NULL;  // Not power of two, return error
    }
    ptr = r_malloc(sizeof(ring_fifo_t));
    if (ptr == NULL)
        return NULL;
    fifo_ptr = (ring_fifo_t*)ptr;
    fifo_ptr->in = 0;
    fifo_ptr->out = 0;
    fifo_ptr->size = size;

    ptr = r_malloc(size);
    if (ptr == NULL)
        return NULL;
    fifo_ptr->buffer = ptr;

    return fifo_ptr;
}


void ring_buffer_free(ring_fifo_t* ptr)
{
    free(ptr->buffer);
    free(ptr);
}

fifo_status_t ring_buffer_reset(ring_fifo_t* ptr)
{
    if (!ptr) return FIFO_ERR_PARM;

    ptr->in = 0;
    ptr->out = 0;
    memset(ptr->buffer, 0, ptr->size);
    return FIFO_OK;
}
/*单字节检查缓冲区数据*/
fifo_status_t peek_ring_fifo_char(ring_fifo_t* fifo_ptr, uint8_t* ch)
{
    uint8_t len = 0;

    if (!fifo_ptr || !fifo_ptr->buffer || !ch) {
        return FIFO_ERR_PARM;
    }

    uint32_t used = fifo_ptr->in - fifo_ptr->out;
    
    // Check if buffer is empty
    if (used == 0) {
        return FIFO_ERR_EMPTY;
    }

    *ch = fifo_ptr->buffer[fifo_ptr->out & (fifo_ptr->size - 1)];
    return FIFO_OK;
}
/*单字节读取缓冲区数据*/
fifo_status_t read_ring_fifo_char(ring_fifo_t *fifo_ptr, uint8_t* ch)
{
    // First try to peek the character
    fifo_status_t status = peek_ring_fifo_char(fifo_ptr, ch);
    
    // If peek succeeded, advance the out pointer
    if (status == FIFO_OK) {
        // Use mask operation to handle wrap-around
        fifo_ptr->out++;
    }
    
    return status;
}
/*单字节将数据写入缓冲区*/
fifo_status_t write_ring_fifo_char(ring_fifo_t* fifo_ptr, uint8_t data)
{
    // if (!fifo_ptr || !fifo_ptr->buffer) {
    //     return FIFO_ERR_PARM;
    // }

    uint32_t used = fifo_ptr->in - fifo_ptr->out;
    if (used >= fifo_ptr->size) {
        return FIFO_ERR_FULL;
    }

    *(fifo_ptr->buffer + (fifo_ptr->in & (fifo_ptr->size - 1))) = data;
    fifo_ptr->in++;

    return FIFO_OK;
}
/*获取缓冲区已使用空间大小*/
uint32_t get_ring_buff_used(ring_fifo_t* fifo_ptr)
{
    //not deal with when in < out,
    if (fifo_ptr->in >= fifo_ptr->out) {
        return (fifo_ptr->in - fifo_ptr->out);
    }
    return 0;
}

/*获取缓冲区剩余空间大小*/
uint32_t get_ring_buff_have(ring_fifo_t* fifo_ptr)
{
    return (fifo_ptr->size - get_ring_buff_used(fifo_ptr));
}
/*多字节检查缓冲区数据*/
fifo_status_t peek_ring_fifo(ring_fifo_t* fifo_ptr, uint8_t* buffer, uint32_t len)
{
    // Parameter validation
    if (!fifo_ptr || !fifo_ptr->buffer || !buffer) {
        return FIFO_ERR_PARM;
    }

    // Calculate available data (safe for wrap-around)
    uint32_t used = fifo_ptr->in - fifo_ptr->out;
    if (used == 0) {
        return FIFO_ERR_EMPTY;  // Buffer is empty
    }

    // Adjust request length
    len = (len < used) ? len : used;

    // Calculate first segment length (to buffer end)
    uint32_t out_pos = fifo_ptr->out & (fifo_ptr->size - 1);
    uint32_t first_seg = fifo_ptr->size - out_pos;
    uint32_t copy_len = (len < first_seg) ? len : first_seg;

    // Copy data
    if (copy_len > 0) {
        memcpy(buffer, fifo_ptr->buffer + out_pos, copy_len);
    }
    if (len > copy_len) {
        memcpy(buffer + copy_len, fifo_ptr->buffer, len - copy_len);
    }

    return FIFO_OK;
}
/*多字节读数据*/
fifo_status_t read_ring_fifo(ring_fifo_t *fifo_ptr, uint8_t* buffer, uint32_t len)
{
    fifo_status_t status = peek_ring_fifo(fifo_ptr, buffer, len);
    if (status != FIFO_OK)
        return status;

    fifo_ptr->out += len;

    return FIFO_OK;
}

/*多字节写入缓冲区数据*/
fifo_status_t write_ring_fifo(ring_fifo_t* fifo_ptr, uint8_t* buffer, uint32_t len)
{
    // Parameter validation
    if (!fifo_ptr || !fifo_ptr->buffer || !buffer) {
        return FIFO_ERR_PARM;
    }

    // Calculate free space (safe for wrap-around)
    uint32_t used = fifo_ptr->in - fifo_ptr->out;
    uint32_t free = fifo_ptr->size - used;
    
    // Adjust write length
    if (len > free) {
        return FIFO_ERR_FULL;
    }

    // Calculate first segment length (to buffer end)
    uint32_t in_pos = fifo_ptr->in & (fifo_ptr->size - 1);
    uint32_t first_seg = fifo_ptr->size - in_pos;
    uint32_t copy_len = (len < first_seg) ? len : first_seg;

    // Copy data
    if (copy_len > 0) {
        memcpy(fifo_ptr->buffer + in_pos, buffer, copy_len);
    }
    if (len > copy_len) {
        memcpy(fifo_ptr->buffer, buffer + copy_len, len - copy_len);
    }

    // Update write index
    fifo_ptr->in += len;

    return FIFO_OK;
}