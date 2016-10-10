#include "base.h"
#include <string.h>
#include "ringbuf.h"

// 判断x是否是2的次方
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

// 初始化缓冲区
u32 ringbuf_init(ringbuf_t *ring_buf, void *buffer, u32 size)
{
    if (!is_power_of_2(size)) {
        return FALSE;
    }
    
    memset(ring_buf, 0, sizeof(ringbuf_t));
    ring_buf->buffer = buffer;
    ring_buf->size = size;
    ring_buf->in = 0;
    ring_buf->out = 0;
    
    return TRUE;
}

// 缓冲区数据的长度
u32 ringbuf_use_len(const ringbuf_t *ring_buf)
{
    return (ring_buf->in - ring_buf->out);
}

// 判断缓冲区是否满
u32 ringbuf_is_full(const ringbuf_t *ring_buf)
{
    return (ring_buf->in - ring_buf->out == ring_buf->size);
}

// 从缓冲区中取数据
u32 ringbuf_get(ringbuf_t *ring_buf, void *buffer, u32 size)
{
    u32 len = 0;
    
    size = MIN(size, ring_buf->in - ring_buf->out);
    /* first get the data from fifo->out until the end of the buffer */
    len = MIN(size, ring_buf->size - (ring_buf->out & (ring_buf->size - 1)));
    memcpy((u8*)buffer, (u8*)ring_buf->buffer + (ring_buf->out & (ring_buf->size - 1)), len);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy((u8*)buffer + len, (u8*)ring_buf->buffer, size - len);
    ring_buf->out += size;
    
    // buffer中没有数据
    if (ring_buf->in == ring_buf->out) {
        ring_buf->in = ring_buf->out = 0;
    }
    
    return size;
}

// 向缓冲区中存数据
u32 ringbuf_put(ringbuf_t *ring_buf, const void *buffer, u32 size)
{
    u32 len = 0;
    
    size = MIN(size, ring_buf->size - ring_buf->in + ring_buf->out);
    /* first put the data starting from fifo->in to buffer end */
    len  = MIN(size, ring_buf->size - (ring_buf->in & (ring_buf->size - 1)));
    memcpy((u8*)ring_buf->buffer + (ring_buf->in & (ring_buf->size - 1)), (u8*)buffer, len);
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy((u8*)ring_buf->buffer, (u8*)buffer + len, size - len);
    ring_buf->in += size;
    
    return size;
}

// 向缓冲区中存字节数据
u32 ringbuf_putc(ringbuf_t *ring_buf, const u8 c)
{
    if (ringbuf_is_full(ring_buf)) {
        return 0;
    }

    *((u8*)ring_buf->buffer + (ring_buf->in & (ring_buf->size - 1))) = c;
    ring_buf->in++;
    
    return 1;
}
