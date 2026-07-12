#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
RingBuffer* ring_buffer_create(size_t capacity) {
    RingBuffer* rb = (RingBuffer*)malloc(sizeof(RingBuffer));
    rb->buffer = (float*)calloc(capacity, sizeof(float));
    rb->capacity = capacity;
    atomic_init(&rb->read_pos, 0);
    atomic_init(&rb->write_pos, 0);
    return rb;
}
void ring_buffer_destroy(RingBuffer* rb) { if(rb){ free(rb->buffer); free(rb); } }
int ring_buffer_write(RingBuffer* rb, const float* data, size_t n) {
    size_t w = atomic_load_explicit(&rb->write_pos, memory_order_relaxed);
    size_t r = atomic_load_explicit(&rb->read_pos, memory_order_acquire);
    size_t avail = (r > w) ? (r - w - 1) : (rb->capacity - w + r - 1);
    if(n > avail) n = avail;
    if(n == 0) return 0;
    size_t f = rb->capacity - w;
    if(n <= f) memcpy(&rb->buffer[w], data, n*sizeof(float));
    else { memcpy(&rb->buffer[w], data, f*sizeof(float)); memcpy(&rb->buffer[0], data+f, (n-f)*sizeof(float)); }
    atomic_store_explicit(&rb->write_pos, (w+n)%rb->capacity, memory_order_release);
    return (int)n;
}
int ring_buffer_read(RingBuffer* rb, float* data, size_t n) {
    size_t r = atomic_load_explicit(&rb->read_pos, memory_order_relaxed);
    size_t w = atomic_load_explicit(&rb->write_pos, memory_order_acquire);
    size_t avail = (w >= r) ? (w-r) : (rb->capacity - r + w);
    if(n > avail) {
        if(avail > 0) {
            size_t f = rb->capacity - r;
            if(avail <= f) memcpy(data, &rb->buffer[r], avail*sizeof(float));
            else { memcpy(data, &rb->buffer[r], f*sizeof(float)); memcpy(data+f, &rb->buffer[0], (avail-f)*sizeof(float)); }
            memset(data+avail, 0, (n-avail)*sizeof(float));
            atomic_store_explicit(&rb->read_pos, (r+avail)%rb->capacity, memory_order_release);
        } else memset(data, 0, n*sizeof(float));
        return (int)avail;
    }
    size_t f = rb->capacity - r;
    if(n <= f) memcpy(data, &rb->buffer[r], n*sizeof(float));
    else { memcpy(data, &rb->buffer[r], f*sizeof(float)); memcpy(data+f, &rb->buffer[0], (n-f)*sizeof(float)); }
    atomic_store_explicit(&rb->read_pos, (r+n)%rb->capacity, memory_order_release);
    return (int)n;
}
