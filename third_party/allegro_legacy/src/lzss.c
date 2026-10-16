/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      LZSS compression routines.
 *
 *      By Shawn Hargreaves.
 *
 *      Original code by Haruhiko Okumura.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro.h"
#include "allegro/internal/aintern.h"


/*
   This compression algorithm is based on the ideas of Lempel and Ziv,
   with the modifications suggested by Storer and Szymanski. The algorithm 
   is based on the use of a ring buffer, which initially contains zeros. 
   We read several characters from the file into the buffer, and then 
   search the buffer for the longest string that matches the characters 
   just read, and output the length and position of the match in the buffer.

   With a buffer size of 4096 bytes, the position can be encoded in 12
   bits. If we represent the match length in four bits, the <position,
   length> pair is two bytes long. If the longest match is no more than
   two characters, then we send just one character without encoding, and
   restart the process with the next letter. We must send one extra bit
   each time to tell the decoder whether we are sending a <position,
   length> pair or an unencoded character, and these flags are stored as
   an eight bit mask every eight items.

   The original implementation used binary trees to speed up the search
   for the longest match; this one uses hash chains (see below).

   Original code by Haruhiko Okumura, 4/6/1989.
   12-2-404 Green Heights, 580 Nagasawa, Yokosuka 239, Japan.

   Modified for use in the Allegro filesystem by Shawn Hargreaves.

   Use, distribute, and modify this code freely.
*/


#define N            4096           /* 4k buffers for LZ compression */
#define F            18             /* upper limit for LZ match length */
#define THRESHOLD    2              /* LZ encode string into pos and length
				       if match size is greater than this */


/* local edit: the packer below replaces the original binary-tree packer.
 * The packer keeps a ring of recent history plus pending lookahead, and finds
 * matches with hash chains over 3-byte prefixes. The emitted bitstream is the
 * classic Okumura format read by lzss_read below: a flags byte, then eight
 * items that are either a literal byte or a (12-bit ring position, 4-bit
 * length-3) pair. Positions are indexes into the decoder's N-byte ring, whose
 * first N-F slots start out as zeros, so the history here begins with N-F
 * zero bytes that matches may also reference.
 */
#define LZSS_HIST_SIZE  16384       /* power of two; holds the N window, the
				       pending lookahead (under F bytes) and a
				       full F_BUF_SIZE chunk, so lzss_write
				       takes each packfile flush in one pass */
#define LZSS_HIST_MASK  (LZSS_HIST_SIZE-1)
#define LZSS_HASH_BITS  15
#define LZSS_HASH_SIZE  (1<<LZSS_HASH_BITS)
#define LZSS_MAX_CHAIN  1024        /* candidates examined per position; keeps
				       output within ~0.2% of an exhaustive
				       search on real quest files */

struct LZSS_PACK_DATA               /* stuff for doing LZ compression */
{
   int pos;                         /* history index of the next byte to encode */
   int end;                         /* one past the last byte appended */
   int ins;                         /* next history index to add to the hash chains */
   int code_buf_ptr;
   unsigned char mask;
   unsigned char code_buf[17];
   unsigned char hist[LZSS_HIST_SIZE];
   int head[LZSS_HASH_SIZE];        /* most recent position per hash, or -1 */
   int prev[LZSS_HIST_SIZE];        /* previous position with the same hash */
};


struct LZSS_UNPACK_DATA             /* for reading LZ files */
{
   int state;                       /* where have we got to? */
   int i, j, k, r, c;
   int flags;
   unsigned char text_buf[N+F-1];   /* ring buffer, with F-1 extra bytes
				       for string comparison */
};



/*** Compression (writing) ***/

/* create_lzss_pack_data:
 *  Creates a PACK_DATA structure.
 */
LZSS_PACK_DATA *create_lzss_pack_data(void)
{
   LZSS_PACK_DATA *dat;
   int c;

   if ((dat = _AL_MALLOC_ATOMIC(sizeof(LZSS_PACK_DATA))) == NULL) {
      *allegro_errno = ENOMEM;
      return NULL;
   }

   /* The decoder's ring starts with N-F zero bytes; mirror them so matches
      can refer to them, exactly as the original tree packer did. */
   for (c=0; c < N - F; c++)
      dat->hist[c] = 0;

   for (c=0; c < LZSS_HASH_SIZE; c++)
      dat->head[c] = -1;

   dat->pos = dat->end = N - F;
   dat->ins = 0;
   dat->code_buf[0] = 0;
   dat->code_buf_ptr = dat->mask = 1;

   return dat;
}



/* free_lzss_pack_data:
 *  Frees an LZSS_PACK_DATA structure.
 */
void free_lzss_pack_data(LZSS_PACK_DATA *dat)
{
   ASSERT(dat);

   _AL_FREE(dat);
}



/* lzss_hash:
 *  Hashes the three history bytes starting at index i.
 */
static unsigned int lzss_hash(LZSS_PACK_DATA *dat, int i)
{
   unsigned int v = ((unsigned int)dat->hist[i & LZSS_HIST_MASK] << 16)
		  | ((unsigned int)dat->hist[(i+1) & LZSS_HIST_MASK] << 8)
		  |  (unsigned int)dat->hist[(i+2) & LZSS_HIST_MASK];
   return (v * 2654435761u) >> (32 - LZSS_HASH_BITS);
}



/* lzss_insert_pending:
 *  Adds every encoded position that has three bytes of history behind it to
 *  the hash chains. Only positions already encoded are added, so a lookup
 *  never returns a position at or after the one being encoded.
 */
static void lzss_insert_pending(LZSS_PACK_DATA *dat)
{
   while (dat->ins < dat->pos && dat->ins + 2 < dat->end) {
      unsigned int h = lzss_hash(dat, dat->ins);
      dat->prev[dat->ins & LZSS_HIST_MASK] = dat->head[h];
      dat->head[h] = dat->ins;
      dat->ins++;
   }
}



/* lzss_flush_code:
 *  Writes the pending group of up to eight items to the file.
 */
static int lzss_flush_code(PACKFILE *file, LZSS_PACK_DATA *dat)
{
   int i;

   if ((file->is_normal_packfile) && (file->normal.passpos) &&
       (file->normal.flags & PACKFILE_FLAG_OLD_CRYPT))
   {
      dat->code_buf[0] ^= *file->normal.passpos;
      file->normal.passpos++;
      if (!*file->normal.passpos)
	 file->normal.passpos = file->normal.passdata;
   }

   for (i=0; i<dat->code_buf_ptr; i++)
      pack_putc(dat->code_buf[i], file);

   dat->code_buf[0] = 0;
   dat->code_buf_ptr = dat->mask = 1;

   return pack_ferror(file) ? EOF : 0;
}



/* lzss_find_match:
 *  Finds the longest match for the lookahead at pos among the hash chain
 *  candidates, up to maxlen bytes. A match may start up to N bytes back,
 *  including the slot the decoder is about to overwrite: it reads the source
 *  byte before writing it, like the original tree packer relied on.
 *  Returns the match length (0 if none) and its history position.
 */
static int lzss_find_match(LZSS_PACK_DATA *dat, int pos, int maxlen, int *best_pos_out)
{
   unsigned char *hist = dat->hist;
   int best_len = 0, best_pos = 0;
   int cand = dat->head[lzss_hash(dat, pos)];
   int chain = LZSS_MAX_CHAIN;
   while ((cand >= 0) && (pos - cand <= N) && (chain-- > 0)) {
      if (hist[(cand + best_len) & LZSS_HIST_MASK] == hist[(pos + best_len) & LZSS_HIST_MASK]) {
	 int len = 0;
	 while ((len < maxlen) &&
		(hist[(cand + len) & LZSS_HIST_MASK] == hist[(pos + len) & LZSS_HIST_MASK]))
	    len++;
	 if (len > best_len) {
	    best_len = len;
	    best_pos = cand;
	    if (len >= maxlen)
	       break;
	 }
      }
      cand = dat->prev[cand & LZSS_HIST_MASK];
   }
   *best_pos_out = best_pos;
   return best_len;
}

/* lzss_write:
 *  Packs size bytes from buf, using the pack information contained in dat.
 *  Input is accepted incrementally: up to F-1 bytes stay pending between
 *  calls until enough lookahead arrives, or until last is set.
 *  Returns 0 on success.
 */
int lzss_write(PACKFILE *file, LZSS_PACK_DATA *dat, int size, unsigned char *buf, int last)
{
   unsigned char *hist = dat->hist;

   for (;;) {
      int final, room, n;

      /* Append as much input as fits while keeping the N-byte window
	 before pos resident. */
      room = LZSS_HIST_SIZE - N - (dat->end - dat->pos);
      n = MIN(size, room);
      while (n-- > 0) {
	 hist[dat->end & LZSS_HIST_MASK] = *(buf++);
	 dat->end++;
	 size--;
      }

      final = last && (size == 0);

      while (dat->pos < dat->end) {
	 int pos = dat->pos;
	 int avail = dat->end - pos;
	 int maxlen, best_len, best_pos;

	 if ((avail < F) && (!final))
	    break;                    /* wait for more lookahead */

	 lzss_insert_pending(dat);

	 maxlen = MIN(avail, F);
	 best_len = 0;
	 best_pos = 0;

	 if (maxlen > THRESHOLD)
	    best_len = lzss_find_match(dat, pos, maxlen, &best_pos);

	 if (best_len <= THRESHOLD) {
	    best_len = 1;              /* not long enough match: send one byte */
	    dat->code_buf[0] |= dat->mask;    /* 'send one byte' flag */
	    dat->code_buf[dat->code_buf_ptr++] = hist[pos & LZSS_HIST_MASK];
	 }
	 else {
	    /* send position and length pair. Note best_len > THRESHOLD */
	    int ring_pos = best_pos & (N - 1);
	    dat->code_buf[dat->code_buf_ptr++] = (unsigned char) ring_pos;
	    dat->code_buf[dat->code_buf_ptr++] = (unsigned char)
					(((ring_pos >> 4) & 0xF0) |
					 (best_len - (THRESHOLD + 1)));
	 }

	 dat->pos += best_len;

	 if ((dat->mask <<= 1) == 0) {         /* eight items: send them */
	    if (lzss_flush_code(file, dat))
	       return EOF;
	 }
      }

      if (size == 0)
	 break;
   }

   if (last && dat->code_buf_ptr > 1) {   /* send remaining code */
      if (lzss_flush_code(file, dat))
	 return EOF;
   }

   return 0;
}



/*** Decompression (reading) ***/

/* create_unpack_data:
 *  Creates an LZSS_UNPACK_DATA structure.
 */
LZSS_UNPACK_DATA *create_lzss_unpack_data(void)
{
   LZSS_UNPACK_DATA *dat;
   int c;

   if ((dat = _AL_MALLOC_ATOMIC(sizeof(LZSS_UNPACK_DATA))) == NULL) {
      *allegro_errno = ENOMEM;
      return NULL;
   }

   for (c=0; c < N - F; c++)
      dat->text_buf[c] = 0;

   dat->state = 0;

   return dat;
}



/* free_lzss_unpack_data:
 *  Frees an LZSS_UNPACK_DATA structure.
 */
void free_lzss_unpack_data(LZSS_UNPACK_DATA *dat)
{
   ASSERT(dat);

   _AL_FREE(dat);
}



/* lzss_read:
 *  Unpacks from dat into buf, until either EOF is reached or s bytes have
 *  been extracted. Returns the number of bytes added to the buffer
 */
int lzss_read(PACKFILE *file, LZSS_UNPACK_DATA *dat, int s, unsigned char *buf)
{
   int i = dat->i;
   int j = dat->j;
   int k = dat->k;
   int r = dat->r;
   int c = dat->c;
   unsigned int flags = dat->flags;
   int size = 0;

   if (dat->state==2)
      goto pos2;
   else
      if (dat->state==1)
	 goto pos1;

   r = N-F;
   flags = 0;

   for (;;) {
      if (((flags >>= 1) & 256) == 0) {
	 if ((c = pack_getc(file)) == EOF)
	    break;
  
	 if ((file->is_normal_packfile) && (file->normal.passpos) &&
	     (file->normal.flags & PACKFILE_FLAG_OLD_CRYPT))
	 {
	    c ^= *file->normal.passpos;
	    file->normal.passpos++;
	    if (!*file->normal.passpos)
	       file->normal.passpos = file->normal.passdata;
	 }

	 flags = c | 0xFF00;        /* uses higher byte to count eight */
      }

      if (flags & 1) {
	 if ((c = pack_getc(file)) == EOF)
	    break;
	 dat->text_buf[r++] = c;
	 r &= (N - 1);
	 *(buf++) = c;
	 if (++size >= s) {
	    dat->state = 1;
	    goto getout;
	 }
	 pos1:
	    ; 
      }
      else {
	 if ((i = pack_getc(file)) == EOF)
	    break;
	 if ((j = pack_getc(file)) == EOF)
	    break;
	 i |= ((j & 0xF0) << 4);
	 j = (j & 0x0F) + THRESHOLD;
	 for (k=0; k <= j; k++) {
	    c = dat->text_buf[(i + k) & (N - 1)];
	    dat->text_buf[r++] = c;
	    r &= (N - 1);
	    *(buf++) = c;
	    if (++size >= s) {
	       dat->state = 2;
	       goto getout;
	    }
	    pos2:
	       ; 
	 }
      }
   }

   dat->state = 0;

   getout:

   dat->i = i;
   dat->j = j;
   dat->k = k;
   dat->r = r;
   dat->c = c;
   dat->flags = flags;

   return size;
}



/* _al_lzss_incomplete_state:
 *  Return non-zero if the previous lzss_read() call was in the middle of
 *  unpacking a sequence of bytes into the supplied buffer, but had to suspend
 *  because the buffer wasn't big enough.
 */
int _al_lzss_incomplete_state(AL_CONST LZSS_UNPACK_DATA *dat)
{
   return dat->state == 2;
}
