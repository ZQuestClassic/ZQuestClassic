/*
 *      JPGalleg: JPEG image decoding routines for
 *
 *         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\ 
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \ 
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      game programming library.
 *
 *      Version 1.1, by Angelo Mottola, May/June 2000
 *
 *      See README file for instructions on using this package in your own
 *      programs.
 */

#include "zc_alleg.h"
#include "/allegro/include/allegro/internal/aintern.h"
#include <stdio.h>
#include <math.h>

#define FIX_0_298631336  ((long)  2446)
#define FIX_0_390180644  ((long)  3196)
#define FIX_0_541196100  ((long)  4433)
#define FIX_0_765366865  ((long)  6270)
#define FIX_0_899976223  ((long)  7373)
#define FIX_1_175875602  ((long)  9633)
#define FIX_1_501321110  ((long)  12299)
#define FIX_1_847759065  ((long)  15137)
#define FIX_1_961570560  ((long)  16069)
#define FIX_2_053119869  ((long)  16819)
#define FIX_2_562915447  ((long)  20995)
#define FIX_3_072711026  ((long)  25172)

#define NEXTWORD(x)     ((*(x)->data << 8) | (*((x)->data + 1)))
#define DESCALE(x,n)    (((x) + ((long)1 << ((n) - 1))) >> n)

#define SOF0            0xc0
#define SOF1            0xc1
#define DHT             0xc4
#define SOI             0xd8
#define EOI             0xd9
#define SOS             0xda
#define DQT             0xdb
#define DRI             0xdd
#define APP0            0xe0
#define COM             0xfe

#define SOF0_DEFINED    0x01
#define DHT_DEFINED     0x02
#define SOI_DEFINED     0x04
#define SOS_DEFINED     0x08
#define DQT_DEFINED     0x10
#define APP0_DEFINED    0x20
#define DRI_DEFINED     0x40

#define JFIF_OK         0x3F

typedef struct HUFFMAN_NODE
{
  long index;
  int code, length;
} HUFFMAN_NODE;

typedef struct DECODER_DATA
{
  unsigned char *data_start;                                /* buffered JPG data block */
  unsigned char *data;                                      /* pointer to actual data */

  int bits;                                                 /* number of bits available to be read */
  int byte;                                                 /* current byte read */
  int width, height;                                        /* size of the image */

  int components;                                           /* number of image pixel components */
  HUFFMAN_NODE huffman_node[1024];                          /* huffman decoding entries */
  int dequant[128];                                         /* dequantization tables */
  int huffman_y, huffman_cbcr;                              /* for tables handling */
  int samples_y;                                            /* y sampling factor */
  int quant_y, quant_cbcr;                                  /* quantization table indexes */
  int ac_y, ac_cbcr;                                        /* AC y/cbcr table indexes */
  int dc_y, dc_cbcr;                                        /* DC y/cbcr table indexes */
  int flags;                                                /* Various decoding flags */
} DECODER_DATA;

/* next_bit:
 *  Returns next bit from decoding data stream.
 */
static unsigned long next_bit(DECODER_DATA *dec)
{
  //   long seg_len;

  dec->bits--;
  if (dec->bits < 0)
  {
    dec->bits = 7;
    dec->byte = *(++dec->data);
    if (dec->byte == 0xff)
    {
      dec->data++;
    }
  }
  return (long)((dec->byte & (1 << dec->bits)) ? 1 : 0);
}

/* get_bits:
 *  Reads a sequence of bits from the buffered data stream. Used to get the
 *  representation of a number, given its category (by JPEG standard).
 */
static int get_bits(DECODER_DATA *dec, int category)
{
  unsigned int i;
  int result = 0;

  for (i=0; i<category; i++)
    result = (result << 1) | next_bit(dec);

  /* adjust sign of the value by category */
  if (result >= (1 << (category - 1)))
    return result;
  else
    return result - ((1 << category) - 1);
}

/* huffman_decode:
 *  Decodes a huffman encoded value from JPG file.
 */
static int huffman_decode(DECODER_DATA *dec, HUFFMAN_NODE *node)
{
  int i, j, found = -1;
  long val;

  if (*dec->data == 0xff)
    if (*(dec->data + 1) >= 0xd0 && *(dec->data + 1) <= 0xd7)
  {
    val = (1 << dec->bits) - 1;
    if ((dec->byte & val) == val)
      return -2;
  }

  val = 0;
  for (i=1; i<=16; i++)
  {
    val = (val << 1) | next_bit(dec);
    for (j=0; j<256; j++)
    {
      if (node[j].length > i) break;
      if ((node[j].length == i) &&
        (node[j].index == val))
      {
        found = j;
        break;
      }
    }
    if (found > -1) break;
  }
  if (found == -1) return -1;

  return node[found].code;
}

/* zigzag_reorder:
 *  Reorders coefficients following the zigzag scan path.
 */
static void zigzag_reorder(int *coefs)
{
  int i, temp[64],
    scan[64] =
  {
    0, 1, 5, 6,14,15,27,28,
    2, 4, 7,13,16,26,29,42,
    3, 8,12,17,25,30,41,43,
    9,11,18,24,31,40,44,53,
    10,19,23,32,39,45,52,54,
    20,22,33,38,46,51,55,60,
    21,34,37,47,50,56,59,61,
    35,36,48,49,57,58,62,63
  };

  for (i=0; i<64; i++) temp[i] = coefs[scan[i]];
  for (i=0; i<64; i++) coefs[i] = temp[i];
}

/* do_idct:
 *  Calculates the inverse discrete cosine transform; based on code by the
 *  Independent JPEG Group. This method uses integer fixed point math.
 */
static void do_idct(int *data, int *dequant)
{
  long tmp0, tmp1, tmp2, tmp3,
    tmp10, tmp11, tmp12, tmp13,
    z1, z2, z3, z4, z5;
  int *inptr, *wsptr, *dqptr, *outptr;
  int i, workspace[64], temp;

  /* Pass 1 */

  inptr = data;
  dqptr = dequant;
  wsptr = workspace;
  for (i=8; i>0; i--)
  {
    if (inptr[8] == 0 && inptr[16] == 0 && inptr[24] == 0 &&
      inptr[32] == 0 && inptr[40] == 0 && inptr[48] == 0 &&
      inptr[56] == 0)
    {
      /* AC terms all zero */
      temp = (inptr[0] * dqptr[0]) << 2;

      wsptr[0] = temp;
      wsptr[8] = temp;
      wsptr[16] = temp;
      wsptr[24] = temp;
      wsptr[32] = temp;
      wsptr[40] = temp;
      wsptr[48] = temp;
      wsptr[56] = temp;

      inptr++;
      wsptr++;
      dqptr++;
      continue;
    }

    z2 = inptr[16] * dqptr[16];
    z3 = inptr[48] * dqptr[48];
    z1 = (z2 + z3) * FIX_0_541196100;
    tmp2 = z1 + (z3 * (- FIX_1_847759065));
    tmp3 = z1 + (z2 * FIX_0_765366865);
    z2 = inptr[0] * dqptr[0];
    z3 = inptr[32] * dqptr[32];
    tmp0 = (z2 + z3) << 13;
    tmp1 = (z2 - z3) << 13;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = inptr[56] * dqptr[56];
    tmp1 = inptr[40] * dqptr[40];
    tmp2 = inptr[24] * dqptr[24];
    tmp3 = inptr[8] * dqptr[8];
    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = (z3 + z4) * FIX_1_175875602;
    tmp0 *= FIX_0_298631336;
    tmp1 *= FIX_2_053119869;
    tmp2 *= FIX_3_072711026;
    tmp3 *= FIX_1_501321110;
    z1 *= (- FIX_0_899976223);
    z2 *= (- FIX_2_562915447);
    z3 *= (- FIX_1_961570560);
    z4 *= (- FIX_0_390180644);
    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Temp output */
    wsptr[0]  = (int) DESCALE(tmp10 + tmp3, 11);
    wsptr[56] = (int) DESCALE(tmp10 - tmp3, 11);
    wsptr[8]  = (int) DESCALE(tmp11 + tmp2, 11);
    wsptr[48] = (int) DESCALE(tmp11 - tmp2, 11);
    wsptr[16] = (int) DESCALE(tmp12 + tmp1, 11);
    wsptr[40] = (int) DESCALE(tmp12 - tmp1, 11);
    wsptr[24] = (int) DESCALE(tmp13 + tmp0, 11);
    wsptr[32] = (int) DESCALE(tmp13 - tmp0, 11);

    inptr++;
    dqptr++;
    wsptr++;
  }

  /* Pass 2 */

  wsptr = workspace;
  outptr = data;
  for (i=0; i<8; i++)
  {
    z2 = (long) wsptr[2];
    z3 = (long) wsptr[6];
    z1 = (z2 + z3) * FIX_0_541196100;
    tmp2 = z1 + (z3 * (- FIX_1_847759065));
    tmp3 = z1 + (z2 * FIX_0_765366865);
    tmp0 = ((long) wsptr[0] + (long) wsptr[4]) << 13;
    tmp1 = ((long) wsptr[0] - (long) wsptr[4]) << 13;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = (long) wsptr[7];
    tmp1 = (long) wsptr[5];
    tmp2 = (long) wsptr[3];
    tmp3 = (long) wsptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = (z3 + z4) * FIX_1_175875602;

    tmp0 *= FIX_0_298631336;
    tmp1 *= FIX_2_053119869;
    tmp2 *= FIX_3_072711026;
    tmp3 *= FIX_1_501321110;
    z1 *= (- FIX_0_899976223);
    z2 *= (- FIX_2_562915447);
    z3 *= (- FIX_1_961570560);
    z4 *= (- FIX_0_390180644);
    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output */
    outptr[0] = (int) DESCALE(tmp10 + tmp3, 18) + 128;
    outptr[7] = (int) DESCALE(tmp10 - tmp3, 18) + 128;
    outptr[1] = (int) DESCALE(tmp11 + tmp2, 18) + 128;
    outptr[6] = (int) DESCALE(tmp11 - tmp2, 18) + 128;
    outptr[2] = (int) DESCALE(tmp12 + tmp1, 18) + 128;
    outptr[5] = (int) DESCALE(tmp12 - tmp1, 18) + 128;
    outptr[3] = (int) DESCALE(tmp13 + tmp0, 18) + 128;
    outptr[4] = (int) DESCALE(tmp13 - tmp0, 18) + 128;

    outptr += 8;
    wsptr += 8;
  }
}

/* read_dht:
 *  Reads AC/DC huffman table(s) from jpeg data.
 */
static int read_dht(DECODER_DATA *dec)
{
  int bit_len[16];
  int table_class, table_id, i, j;
  unsigned int seg_len, count, val, table, index;

  dec->data++;
  seg_len = NEXTWORD(dec);
  dec->data += 2;
  count = 2;

  do
  {
    table_id = *dec->data & 0x1;
    table_class = (*dec->data & 0x10) >> 4;
    table = (table_class << 9) | (table_id << 8);
    dec->data++;
    count++;

    for (i=0; i<16; i++, count++)
      bit_len[i] = *dec->data++;

    val = 0;
    index = -1;
    for (i=0; i<16; i++)
    {
      for (j=0; j<bit_len[i]; j++)
      {
        index++;
        count++;
        dec->huffman_node[table | index].code = *dec->data++;
        dec->huffman_node[table | index].index = val;
        dec->huffman_node[table | index].length = i + 1;
        val++;
      }
      val <<= 1;
    }
  } while (count < seg_len);

  return 0;
}

/* read_sof0:
 *  Basically reads more image informations.
 */
static int read_sof0(DECODER_DATA *dec)
{
  unsigned char comps;
  int i;

  dec->data += 3;
  if (*dec->data != 8)
    /* Only 8 bits/sample supported */
    return 2;
  dec->data++;

  dec->height = NEXTWORD(dec);
  dec->data += 2;
  dec->width = NEXTWORD(dec);
  dec->data += 2;

  comps = *dec->data++;
  for (i=0; i<comps; i++)
  {
    switch (*dec->data)
    {

      case 1:
        dec->data++;
        dec->samples_y = (*dec->data & 0xf) * (*dec->data >> 4);
        dec->data++;
        dec->quant_y = *dec->data++;
        break;

      case 2:
      case 3:
        /* Skips cbcr samples (we don't care) */
        dec->data += 2;
        dec->quant_cbcr = *dec->data++;
        break;

    }
  }

  return 0;
}

/* read_dqt:
 *  Reads dequantization table(s).
 */
static int read_dqt(DECODER_DATA *dec)
{
  unsigned int seg_len, count;
  int i, qt_num;

  dec->data++;
  seg_len = NEXTWORD(dec);
  dec->data += 2;
  count = 2;

  do
  {
    if ((qt_num = (*dec->data & 0xf)) > 1)
      /* Illegal dequantization table number */
      return 1;
    if (*dec->data & 0xf0)
      /* Only 8 bit dequantization table precision supported */
      return 1;
    dec->data++;
    count++;
    for (i=0; i<64; i++, count++)
      dec->dequant[(qt_num << 6) | i] = *dec->data++;
    zigzag_reorder(dec->dequant + (qt_num << 6));
  } while (count < seg_len);

  return 0;
}

/* read_sos:
 *  Reads final image informations before decoding.
 */
static int read_sos(DECODER_DATA *dec)
{
  int i;

  dec->data += 3;

  dec->components = *dec->data++;
  if ((dec->components != 1) && (dec->components != 3))
    /* Only 1 or 3 components supported */
    return 2;

  for (i=0; i<dec->components; i++)
  {
    switch (*dec->data)
    {

      case 1:
        dec->data++;
        dec->ac_y = *dec->data & 0xf;
        dec->dc_y = *dec->data >> 4;
        dec->data++;
        break;

      case 2:
      case 3:
        dec->data++;
        dec->ac_cbcr = *dec->data & 0xf;
        dec->dc_cbcr = *dec->data >> 4;
        dec->data++;
        break;

      default:
        return 3;
    }
  }
  /* Skip next 3 bytes */
  dec->data += 3;

  return 0;
}

/* read_app0:
 *  Reads JFIF format data segment.
 */
static int read_app0(DECODER_DATA *dec)
{
  int skip;

  dec->data++;

  if (NEXTWORD(dec) < 16)
    /* Segment length too short */
    return 1;
  dec->data += 2;

  if (strcmp(dec->data, "JFIF"))
    /* Invalid JFIF id */
    return 1;
  dec->data += 5;

  if (*dec->data != 1)
    /* Only JFIF version 1.x supported */
    return 1;
  dec->data += 7;

  /* Skips thumbnail */
  skip = (*dec->data) * (*(dec->data + 1));
  dec->data += 2 + (skip * 3);

  return 0;
}

/* decode_block:
 *  Decodes a 8x8 block of pixel components; core algorithm.
 */
static int decode_block(DECODER_DATA *dec, int *dat, int dc, int ac, int qt, int *old_dc)
{
  //   int temp[64];
  int data, num_zeros, num_bits;
  int i, val, coef;
  HUFFMAN_NODE *dc_node, *ac_node;

  dc_node = dec->huffman_node + (dc << 8);
  ac_node = dec->huffman_node + (ac << 8) + 512;

  /* First step: huffman-decode 1 DC coefficient */
  data = huffman_decode(dec, dc_node);
  *old_dc += get_bits(dec, data & 0xf);
  dat[0] = *old_dc;

  coef = 1;
  do
  {
    /* Second step: huffman-decode 63 AC coefficients */
    data = huffman_decode(dec, ac_node);
    if (data == -2) data = 0;
    else if (data == -1)
    {
      /* Bad block */
      for (i=0; i<64; i++) dat[i] = 0;
      for (i=0; i<8; i++) dat[(i<<3)|i] = 255;
      for (i=0; i<8; i++) dat[(i<<3)|(7-i)] = 255;
      return 1;
    }
    num_zeros = data >> 4;
    num_bits = data & 0xf;
    val = get_bits(dec, num_bits);

    /* Third step: run length decoding */
    if ((num_zeros == 0) && (num_bits == 0))                /* end of block */
    {
      for (i=coef; i<64; i++)
        dat[i] = 0;
      break;
    }
    else if ((num_zeros == 15) && (num_bits == 0))          /* 16 zeros RLE */
    {
      for (i=0; i<16; i++)
        dat[coef + i] = 0;
      coef += 16;
    }
    else                                                    /* normal zero run length */
    {
      for (i=0; i<num_zeros && coef<64; i++)
        dat[coef++] = 0;
      if (coef >= 64) break;

      dat[coef++] = val;
    }
  } while (coef < 64);

  /* Fourth step: zig-zag reordering */
  zigzag_reorder(dat);

  /* Fifth step: dequantization and inverse discrete cosine transform */
  do_idct(dat, dec->dequant + (qt << 6));

  return 0;
}

/* putpixel_lc:
 *  Draws a pixel onto a 24bit bitmap, given its luminance and chrominance.
 */
static void putpixel_lc(BITMAP *bmp, int xx, int yy, int y, int cb, int cr)
{
  int r, g, b;

  r = (int)((double)y + 1.402 * ((double)cr - 128.0));
  g = (int)((double)y - 0.34414 * ((double)cb - 128.0) - 0.71414 * ((double)cr - 128.0));
  b = (int)((double)y + 1.772 * ((double)cb - 128.0));

  r = MID(0, r, 255);
  g = MID(0, g, 255);
  b = MID(0, b, 255);

  putpixel(bmp, xx, yy, makecol24(r, g, b));
}

/* load_memory_jpg:
 *  Decodes a jpg image from a memory chunk.
 */
BITMAP *load_memory_jpg(void *data, RGB *pal)
{
  DECODER_DATA dec;
  BITMAP *bmp;
  PALETTE tmppal;
  int y1[64], y2[64], y3[64], y4[64], cb[64], cr[64];
  int dc_y, dc_cb, dc_cr;
  int i, j, x, y, dest_depth, hue;

  memset(&dec, 0, sizeof(dec));

  if (!pal)
    pal = tmppal;

  dec.data_start = (unsigned char *)data;
  dec.data = dec.data_start;

  /*
   *  Scans the jpg data for markers, until the SOS marker is found.
   */
  do
  {
    if (*dec.data++ == 0xff)
    {
      switch (*dec.data)
      {

        case SOF0:
        case SOF1:
          dec.flags |= SOF0_DEFINED;
          if (read_sof0(&dec))
            return NULL;
          break;

        case DHT:
          dec.flags |= DHT_DEFINED;
          read_dht(&dec);
          break;

        case SOI:
          dec.flags |= SOI_DEFINED;
          dec.data++;
          break;

        case SOS:
          dec.flags |= SOS_DEFINED;
          if (read_sos(&dec))
            return NULL;
          break;

        case DQT:
          dec.flags |= DQT_DEFINED;
          if (read_dqt(&dec))
            return NULL;
          break;

        case DRI:
          dec.flags |= DRI_DEFINED;
          dec.data += 5;
          break;

        case APP0:
          dec.flags |= APP0_DEFINED;
          if (read_app0(&dec))
            return NULL;
          break;

        case COM:
          dec.data++;
          i = NEXTWORD(&dec);
          dec.data += i;
          break;

        default:
          return NULL;

      }
    }
    /*
     *  Continue scanning for 10000 bytes (more than enough for a header eh?)
     *  or until all the proper markers have been found.
     */
  } while ((dec.data <= dec.data_start + 10000) && !(dec.flags & SOS_DEFINED));

  /*
   *  Checks if the SOF0, DHT, SOI, SOS, DQT and APP0 markers are all there.
   */
  if ((dec.flags & JFIF_OK) != JFIF_OK)
    return NULL;

  dest_depth = _color_load_depth(24, FALSE);

  bmp = create_bitmap_ex(24, dec.width, dec.height);
  if (!bmp)
    return NULL;

  x = y = 0;
  dc_y = dc_cb = dc_cr = 0;
  dec.bits = 8;
  dec.byte = *dec.data;

  switch(dec.components)
  {

    case 3:
      /*
       *  Image has three components (1 for luminance and 2 for chrominance);
       *  this means it's a colored image.
       */
      switch(dec.samples_y)
      {

        case 4:
          do
          {
            /* 1st case:
             *  chrominance is taken every two pixels (horizontally and
             *  vertically), and luminance for every pixel.
             */
            decode_block(&dec, y1, dec.dc_y, dec.ac_y, dec.quant_y, &dc_y);
            decode_block(&dec, y2, dec.dc_y, dec.ac_y, dec.quant_y, &dc_y);
            decode_block(&dec, y3, dec.dc_y, dec.ac_y, dec.quant_y, &dc_y);
            decode_block(&dec, y4, dec.dc_y, dec.ac_y, dec.quant_y, &dc_y);
            decode_block(&dec, cb, dec.dc_cbcr, dec.ac_cbcr, dec.quant_cbcr, &dc_cb);
            decode_block(&dec, cr, dec.dc_cbcr, dec.ac_cbcr, dec.quant_cbcr, &dc_cr);
            for (i=0; i<8; i++) for (j=0; j<8; j++)
              putpixel_lc(bmp, x + j, y + i,
                  y1[(i << 3) | j],
                  cb[((i >> 1) << 3) | (j >> 1)],
                  cr[((i >> 1) << 3) | (j >> 1)]);
            for (i=0; i<8; i++) for (j=0; j<8; j++)
              putpixel_lc(bmp, x + 8 + j, y + i,
                  y2[(i << 3) | j],
                  cb[((i >> 1) << 3) | ((j >> 1) + 4)],
                  cr[((i >> 1) << 3) | ((j >> 1) + 4)]);
            for (i=0; i<8; i++) for (j=0; j<8; j++)
              putpixel_lc(bmp, x + j, y + 8 + i,
                  y3[(i << 3) | j],
                  cb[(((i >> 1) + 4) << 3) | (j >> 1)],
                  cr[(((i >> 1) + 4) << 3) | (j >> 1)]);
            for (i=0; i<8; i++) for (j=0; j<8; j++)
              putpixel_lc(bmp, x + 8 + j, y + 8 + i,
                  y4[(i << 3) | j],
                  cb[(((i >> 1) + 4) << 3) | ((j >> 1) + 4)],
                  cr[(((i >> 1) + 4) << 3) | ((j >> 1) + 4)]);
            x += 16;
            if (x >= dec.width)
            {
              x = 0;
              y += 16;
              if (dec.flags & DRI_DEFINED)
              {
                dc_y = dc_cb = dc_cr = 0;
                dec.data += 2;
                dec.bits = 0;
              }
            }

          } while (y < dec.height);
          break;

        case 2:
          do
          {
            /* 2nd case:
             *  chrominance is taken every two pixels (only horizontally)
             *  and luminance every pixel.
             */
            decode_block(&dec, y1, dec.dc_y, dec.ac_y, dec.quant_y, &dc_y);
            decode_block(&dec, y2, dec.dc_y, dec.ac_y, dec.quant_y, &dc_y);
            decode_block(&dec, cb, dec.dc_cbcr, dec.ac_cbcr, dec.quant_cbcr, &dc_cb);
            decode_block(&dec, cr, dec.dc_cbcr, dec.ac_cbcr, dec.quant_cbcr, &dc_cr);
            for (i=0; i<8; i++) for (j=0; j<8; j++)
              putpixel_lc(bmp, x + j, y + i,
                  y1[(i << 3) | j],
                  cb[(i << 3) | (j >> 1)],
                  cr[(i << 3) | (j >> 1)]);
            for (i=0; i<8; i++) for (j=0; j<8; j++)
              putpixel_lc(bmp, x + 8 + j, y + i,
                  y2[(i << 3) | j],
                  cb[(i << 3) | ((j >> 1) + 4)],
                  cr[(i << 3) | ((j >> 1) + 4)]);
            x += 16;
            if (x >= dec.width)
            {
              x = 0;
              y += 8;
              if (dec.flags & DRI_DEFINED)
              {
                dc_y = dc_cb = dc_cr = 0;
                dec.data += 2;
                dec.bits = 0;
              }
            }

          } while (y < dec.height);
          break;

        case 1:
          do
          {
            /* 3rd case:
             *  chrominance and luminance are taken every pixel.
             */
            decode_block(&dec, y1, dec.dc_y, dec.ac_y, dec.quant_y, &dc_y);
            decode_block(&dec, cb, dec.dc_cbcr, dec.ac_cbcr, dec.quant_cbcr, &dc_cb);
            decode_block(&dec, cr, dec.dc_cbcr, dec.ac_cbcr, dec.quant_cbcr, &dc_cr);
            for (i=0; i<8; i++) for (j=0; j<8; j++)
              putpixel_lc(bmp, x + j, y + i,
                  y1[(i << 3) | j],
                  cb[(i << 3) | j],
                  cr[(i << 3) | j]);
            x += 8;
            if (x >= dec.width)
            {
              x = 0;
              y += 8;
              if (dec.flags & DRI_DEFINED)
              {
                dc_y = dc_cb = dc_cr = 0;
                dec.data += 2;
                dec.bits = 0;
              }
            }

          } while (y < dec.height);
          break;
      }

    case 1:
      /*
       *  Image has just one component (luminance) taken every pixel; we
       *  have a grayscaled picture.
       */
      do
      {
        decode_block(&dec, y1, dec.dc_y, dec.ac_y, dec.quant_y, &dc_y);
        for (i=0; i<8; i++) for (j=0; j<8; j++)
        {
          hue = MID(0, y1[(i << 3) | j], 255);
          putpixel(bmp, x + j, y + i, makecol24(hue, hue, hue));
        }
        x += 8;
        if (x >= dec.width)
        {
          x = 0;
          y += 8;
          if (dec.flags & DRI_DEFINED)
          {
            dc_y = 0;
            dec.data += 2;
            dec.bits = 0;
          }
        }

      } while (y < dec.height);
      break;
  }

  generate_332_palette(pal);
  if (dest_depth != 24)
    bmp = _fixup_loaded_bitmap(bmp, pal, dest_depth);

  return bmp;
}

/* load_jpg:
 *  Loads a jpg image file (duh!).
 */
BITMAP *load_jpg(const char *filename, RGB *pal)
{
  PACKFILE *f;
  BITMAP *bmp;
  long size;
  unsigned char *data;

  size = file_size(filename);
  if (!size)
    return NULL;

  /*
   *  Loads the whole file in memory.
   */
  data = (unsigned char *)_al_malloc(size);
  if (!data)
    return NULL;

  if (!(f = pack_fopen(filename, F_READ)))
  {
    _al_free(data);
    return NULL;
  }

  pack_fread(data, size, f);
  pack_fclose(f);

  /*
   *  Decodes it.
   */
  bmp = load_memory_jpg(data, pal);

  _al_free(data);

  return bmp;
}
