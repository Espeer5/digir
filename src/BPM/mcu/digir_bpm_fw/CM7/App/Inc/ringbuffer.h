//#################################################################################################
//  DIGIR BPM FIRMWARE RINGBUFFER
//
//  VERSION 0.1
//#################################################################################################

// A generic ringbuffer implementation for use across the BPM firmware.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 8/1/26  Edward Speer  Initial revision

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <stdbool.h>

//#################################################################################################
//  RINGBUFFER
//#################################################################################################

// Generate a type and function definitions for a ring buffer with the given type, type name, and
// capacity.
#define RBUFF_DEF(name, type, capacity)                               \
    typedef struct                                                    \
    {                                                                 \
        type data[(capacity)];                                        \
        size_t head;                                                  \
        size_t tail;                                                  \
        size_t count;                                                 \
    } name;                                                           \
                                                                      \
    static inline void name##_init(name *rb)                          \
    {                                                                 \
        rb->head  = 0;                                                \
        rb->tail  = 0;                                                \
        rb->count = 0;                                                \
    }                                                                 \
                                                                      \
    static inline bool name##_empty(name *rb)                         \
    {                                                                 \
        return rb->count == 0;                                        \
    }                                                                 \
                                                                      \
    static inline bool name##_full(name *rb)                          \
    {                                                                 \
        return rb->count == (capacity);                               \
    }                                                                 \
                                                                      \
    static inline bool name##_push(name *rb, type *value, bool force) \
    {                                                                 \
        if (name##_full(rb))                                          \
        {                                                             \
            if (!force)                                               \
            {                                                         \
                return false;                                         \
            }                                                         \
                                                                      \
            rb->data[rb->head] = *value;                              \
            return true;                                              \
        }                                                             \
                                                                      \
        rb->data[rb->head] = *value;                                  \
        rb->head           = (rb->head + 1) % (capacity);             \
        rb->count++;                                                  \
        return true;                                                  \
    }                                                                 \
                                                                      \
    static inline bool name##_pop(name *rb, type *value)              \
    {                                                                 \
        if (name##_empty(rb))                                         \
        {                                                             \
            return false;                                             \
        }                                                             \
                                                                      \
        *value   = rb->data[rb->tail];                                \
        rb->tail = (rb->tail + 1) % (capacity);                       \
        rb->count--;                                                  \
        return true;                                                  \
    }                                                                 \

#endif // #ifndef RINGBUFFER_H

