#ifndef __MAIN_INFLATE_H__
#define __MAIN_INFLATE_H__

#include <stddef.h>
#include <stdint.h>

int inflate_raw(const unsigned char *in, unsigned int inlen,
    unsigned char *out, unsigned int outlen);

/* Point the decoder's Huffman tables at scratch RAM (3 x 64 KiB). */
void inflate_init_tables(void *area);

extern void (*inflate_progress_hook)(unsigned int outpos);
extern unsigned int inflate_in_pos;

#endif /* __MAIN_INFLATE_H__ */
