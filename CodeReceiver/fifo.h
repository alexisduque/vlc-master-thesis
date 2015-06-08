/**
  ******************************************************************************
  * File Name          : fifo.
  * Date               : 05/04/2015 10:49:20
  * Description        : This file provides code for the configuration
  *                      of the FIFO ABC Buffer.
  ******************************************************************************
  */

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t *bufptr;    // pointer to the buffer array
    size_t bufsize;    // size of buffer
    size_t rdidx;    // points to next address to be read
    size_t wridx;    // points to next address to be written
#if(FIFO_LOG_MAX_USAGE == 1)
    size_t max;
#endif
} FIFO_t;

// RESULT Enum
typedef enum
{
    RES_OK,         ///< Function executed successfully
    RES_FAIL,       ///< Function failed to execute properly
    RES_INVALID,    ///< Parameters returned are not valid
    RES_NOTFOUND,   ///< Item requested was not found
    RES_FULL,       ///< Buffer/Memory/other is full
    RES_UNDERRUN,   ///< Buffer underrun has occurred
    RES_OVERRUN,    ///< Buffer overrun has occured
    RES_PARAMERR,   ///< Invalid input parameter
    RES_END,        ///< Reached the end of a buffer
    RES_BUSY,       ///< Device is busy
    RES_CANCEL,     ///< Operation has been cancelled
    RES_UNKNOWN     ///< Unknown Error
} RES_t;


void FIFO_init(FIFO_t *fifo, void *bufptr, size_t bufsize);

RES_t FIFO_write(FIFO_t *fifo, void *src, size_t size);

void FIFO_write_trample(FIFO_t *fifo, void *src, size_t size);

RES_t FIFO_read(FIFO_t *fifo, void *dst, size_t size);

size_t FIFO_read_max(FIFO_t *fifo, void *dst, size_t max_size);

RES_t FIFO_peek(FIFO_t *fifo, void *dst, size_t size);

void FIFO_clear(FIFO_t *fifo);

size_t FIFO_rdcount(FIFO_t *fifo);

size_t FIFO_wrcount(FIFO_t *fifo);

#endif

/*****************************END OF FILE**************************************/
