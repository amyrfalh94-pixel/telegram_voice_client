#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stddef.h>
#include <stdatomic.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { float *buffer; size_t capacity; atomic_size_t read_pos; atomic_size_t write_pos; } RingBuffer;
RingBuffer* ring_buffer_create(size_t capacity);
void ring_buffer_destroy(RingBuffer* rb);
int ring_buffer_write(RingBuffer* rb, const float* data, size_t n);
int ring_buffer_read(RingBuffer* rb, float* data, size_t n);
#ifdef __cplusplus
}
#endif
#endif
