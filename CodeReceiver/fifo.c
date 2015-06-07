/**
  ******************************************************************************
  * File Name          : Src/fifo.c
  * Date               : 05/04/2015 10:49:20
  * Description        : This file provides code for the configuration
  *                      of the FIFO ABD Buffer.
  ******************************************************************************
  */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "fifo.h"

//--------------------------------------------------------------------------------------------------
void FIFO_init(FIFO_t *fifo, void *bufptr, size_t bufsize)
{
    fifo->bufptr = bufptr;
    fifo->bufsize = bufsize;
    fifo->rdidx = 0;
    fifo->wridx = 0;
#if(FIFO_LOG_MAX_USAGE == 1)
    fifo->max = 0;
#endif
}

//--------------------------------------------------------------------------------------------------
RES_t FIFO_write(FIFO_t *fifo, void *src, size_t size)
{
    size_t wrcount;

    if(size > FIFO_wrcount(fifo))
    {
        return(RES_FULL);
    }

    if((wrcount = fifo->bufsize - fifo->wridx) <= size)
    {
        // write operation will wrap around in fifo
        // write first half of fifo
        memcpy(fifo->bufptr+fifo->wridx,src,wrcount);

        //wrap around and continue
        fifo->wridx = 0;
        size -= wrcount;
        src = (uint8_t*)src + wrcount;
    }

    if(size > 0)
    {
        memcpy(fifo->bufptr+fifo->wridx,src,size);
        fifo->wridx += size;
    }

#if(FIFO_LOG_MAX_USAGE == 1)
    wrcount = FIFO_rdcount(fifo);
    if(wrcount > fifo->max)
    {
        fifo->max = wrcount;
    }
#endif

    return(RES_OK);
}

//--------------------------------------------------------------------------------------------------
void FIFO_write_trample(FIFO_t *fifo, void *src, size_t size)
{

    if(size >= fifo->bufsize-1)
    {
        // if writing more than can ever fit in the buffer,
        // only write the latter portion of src buf.

        fifo->wridx = fifo->bufsize-1;
        fifo->rdidx = 0;
        memcpy(fifo->bufptr, src + (size-(fifo->bufsize-1)), fifo->bufsize-1);
    }
    else
    {
        size_t wrcount;
        int overflow = 0;
        if(size > FIFO_wrcount(fifo))
        {
            overflow = 1;
        }

        wrcount = fifo->bufsize - fifo->wridx;
        if(wrcount <= size)
        {
            // write operation will wrap around in fifo
            // write first half of fifo
            memcpy(fifo->bufptr+fifo->wridx, src, wrcount);

            //wrap around and continue
            fifo->wridx = 0;
            size -= wrcount;
            src = (uint8_t*)src + wrcount;
        }

        if(size > 0)
        {
            memcpy(fifo->bufptr+fifo->wridx, src, size);
            fifo->wridx += size;
        }

        // if overflow happens, move read pointer.
        if(overflow)
        {
            if(fifo->wridx == fifo->bufsize-1)
            {
                fifo->rdidx = 0;
            }
            else
            {
                fifo->rdidx = fifo->wridx + 1;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
RES_t FIFO_read(FIFO_t *fifo, void *dst, size_t size)
{
    size_t rdcount;

    if(size > FIFO_rdcount(fifo))
    {
        return(RES_PARAMERR);
    }

    rdcount = fifo->bufsize - fifo->rdidx;
    if(rdcount <= size)
    {
        // read operation will wrap around in fifo
        // read first half of fifo
        if(dst != NULL)
        {
            memcpy(dst, fifo->bufptr + fifo->rdidx, rdcount);
        }
        //wrap around and continue
        fifo->rdidx = 0;
        size -= rdcount;
        dst = (uint8_t*)dst + rdcount;
    }

    if(size > 0)
    {
        if(dst != NULL)
        {
            memcpy(dst, fifo->bufptr + fifo->rdidx, size);
        }
        fifo->rdidx += size;
    }

    return(RES_OK);
}

//--------------------------------------------------------------------------------------------------
size_t FIFO_read_max(FIFO_t *fifo, void *dst, size_t max_size)
{
    size_t rdcount, nbytes;

    rdcount = FIFO_rdcount(fifo);
    if(max_size > rdcount)
    {
        max_size = rdcount;
    }
    nbytes = max_size;

    rdcount = fifo->bufsize - fifo->rdidx;
    if(rdcount <= nbytes)
    {
        // read operation will wrap around in fifo
        // read first half of fifo
        if(dst != NULL)
        {
            memcpy(dst, fifo->bufptr + fifo->rdidx, rdcount);
        }
        //wrap around and continue
        fifo->rdidx = 0;
        nbytes -= rdcount;
        dst = (uint8_t*)dst + rdcount;
    }

    if(nbytes > 0)
    {
        if(dst != NULL)
        {
            memcpy(dst, fifo->bufptr + fifo->rdidx, nbytes);
        }
        fifo->rdidx += nbytes;
    }

    return(max_size);
}

//--------------------------------------------------------------------------------------------------
RES_t FIFO_peek(FIFO_t *fifo, void *dst, size_t size)
{
    size_t rdcount;
    size_t rdidx;

    if(size > FIFO_rdcount(fifo))
    {
        return(RES_PARAMERR);
    }

    rdidx = fifo->rdidx;

    if((rdcount = fifo->bufsize - rdidx) <= size)
    {
        // read operation will wrap around in fifo
        // read first half of fifo
        memcpy(dst,fifo->bufptr+rdidx,rdcount);
        //wrap around and continue
        rdidx = 0;
        size -= rdcount;
        dst = (uint8_t*)dst + rdcount;
    }

    if(size > 0)
    {
        memcpy(dst,fifo->bufptr+rdidx,size);
        rdidx += size;
    }

    return(RES_OK);
}

//--------------------------------------------------------------------------------------------------
void FIFO_clear(FIFO_t *fifo)
{
    fifo->rdidx = 0;
    fifo->wridx = 0;
}

//--------------------------------------------------------------------------------------------------
size_t FIFO_rdcount(FIFO_t *fifo)
{
    size_t wridx,rdidx;
    wridx = fifo->wridx;
    rdidx = fifo->rdidx;

    if(wridx >= rdidx)
    {
        return(wridx-rdidx);
    }
    else
    {
        return((fifo->bufsize-rdidx)+wridx);
    }
}

//--------------------------------------------------------------------------------------------------
size_t FIFO_wrcount(FIFO_t *fifo)
{
    size_t wridx,rdidx;
    wridx = fifo->wridx;
    rdidx = fifo->rdidx;

    if(rdidx >= wridx+1)
    {
        return(rdidx-wridx-1);
    }
    else
    {
        return((fifo->bufsize-wridx)+rdidx-1);
    }
}

///\}

/*****************************END OF FILE**************************************/
