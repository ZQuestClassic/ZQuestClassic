/* GIF Saver
  * by Paul Bartrum
  */

#include <save_gif.h>
#include <allegro.h>

typedef struct
{
    int16_t base;
    uint8_t new_str;
} LZW_STRING;

typedef struct
{
    int32_t pos;
    int32_t bit_pos;
    uint8_t data[255];
} BUFFER;

void clear_speed_buffer(int16_t *speed_buffer);
void dump_buffer(BUFFER *b, PACKFILE *f);
void output(BUFFER *b, int32_t bit_size, int32_t code, PACKFILE *f);

void clear_speed_buffer(int16_t *speed_buffer)
{
//#if defined(ALLEGRO_MACOSX) || defined(_MSC_VER)
    //OSX doesn't like the assembly below.  *shrug*
    //this is basically the same as memset(speed_buffer,-1,256*4096*2)
    //but is filling speed_buffer 4 bytes at a time instead of 1.
    int32_t b=(256*4096/2);
    int32_t *a=(int32_t*)speed_buffer;
    
    while(--b>=0)
    {
        *a=-1;
        a++;
    }
    
    /*
      #else
      // clear speed buffer to -1
      __asm__("0:\n"
              "movl %%ecx, (%%eax)\n"
              "addl %%edx, %%eax\n"
              "decl %%ebx\n"
              "jne 0b\n"
              :
              : "a" (speed_buffer), "b" (256 * 4096 / 2), "c" (-1), "d" (4)
              : "memory");
    #endif
    */
}

void dump_buffer(BUFFER *b, PACKFILE *f)
{
    int32_t size;
    
    size = b->pos;
    
    if(b->bit_pos != 0)
        size ++;
        
    pack_putc(size, f);
    pack_fwrite(b->data, size, f);
}

void output(BUFFER *b, int32_t bit_size, int32_t code, PACKFILE *f)
{
    int32_t shift;
    
    /* pack the code into the buffer */
    shift = b->bit_pos;
    
    for(;;)
    {
        if(shift >= 0)
        {
            if(b->bit_pos != 0)
                b->data[b->pos] = (uint8_t)((code << shift) | b->data[b->pos]);
            else
                b->data[b->pos] = (uint8_t)(code << shift);
        }
        else
        {
            if(b->bit_pos != 0)
                b->data[b->pos] = (uint8_t)((code >> -shift) | b->data[b->pos]);
            else
                b->data[b->pos] = (uint8_t)(code >> -shift);
        }
        
        if(bit_size + shift > 7)
        {
            b->bit_pos = 0;
            b->pos ++;
            shift -= 8;
            
            if(b->pos == 255)
            {
                dump_buffer(b, f);
                b->pos = 0;
                b->bit_pos = 0;
            }
            
            if(bit_size + shift <= 0)
                break;
        }
        else
        {
            b->bit_pos = bit_size + shift;
            break;
        }
    }
}

int32_t save_gif(const char *filename, BITMAP *bmp, const RGB *pal)
{
    PACKFILE *f;
    int32_t i, bpp, bit_size;
    LZW_STRING string_table[4096];
    int32_t prefix;
    int32_t input_pos = 0;
    int32_t c;                                                    /* current character */
    int32_t empty_str;
    BUFFER buffer;
    int16_t *speed_buffer;
    
    f = pack_fopen(filename, F_WRITE);
    
    if(!f)
        return errno;
        
    pack_mputl(0x47494638, f);                                /* GIF8 */
    pack_mputw(0x3761, f);                                    /* 7a */
    pack_iputw(bmp->w, f);                                    /* width */
    pack_iputw(bmp->h, f);                                    /* height */
    pack_putc(215, f);                                        /* packed fields */
    pack_putc(0, f);                                          /* background colour */
    pack_putc(0, f);                                          /* pixel aspect ratio */
    
    /* global colour table */
    for(i = 0; i < 256; i ++)
    {
        pack_putc(pal[i].r << 2, f);
        pack_putc(pal[i].g << 2, f);
        pack_putc(pal[i].b << 2, f);
    }
    
    pack_putc(0x2c, f);                                       /* image separator */
    pack_iputw(0, f);                                         /* x offset */
    pack_iputw(0, f);                                         /* y offset */
    pack_iputw(bmp->w, f);                                    /* width */
    pack_iputw(bmp->h, f);                                    /* height */
    pack_putc(0, f);                                          /* packed fields */
    
    /* Image data starts here */
    bpp = 8;
    pack_putc(bpp, f);                                        /* initial code size */
    
    /* initialize string table */
    for(i = 0; i < 1 << bpp; i ++)
    {
        string_table[i].base = -1;
        string_table[i].new_str = i;
    }
    
    for(; i < (1 << bpp) + 2; i ++)
    {
        string_table[i].base = -1;
        //    string_table[i].new_str = -1;
        string_table[i].new_str = 255;
    }
    
    empty_str = (1 << bpp) + 2;
    
    prefix = -1;
    
    bit_size = bpp + 1;
    
    buffer.pos = 0;
    buffer.bit_pos = 0;
    
    output(&buffer, bit_size, 1 << bpp, f);                   /* clear code */
    
    speed_buffer = (int16_t*)malloc(256 * 4096 * 2);
    clear_speed_buffer(speed_buffer);
    
    for(;;)
    {
        if((c = getpixel(bmp, input_pos % bmp->w, input_pos / bmp->w)) == EOF)
        {
            output(&buffer, bit_size, prefix, f);
            output(&buffer, bit_size, (1 << bpp) + 1, f);         /* end of information */
            dump_buffer(&buffer, f);
            pack_putc(0, f);                                      /* no more data blocks */
            break;
        }
        
        input_pos ++;
        
        if(prefix == -1)
            i = c;
        else
            i = speed_buffer[prefix * 256 + c];
            
        if(i != -1)
        {
            prefix = i;
        }
        else
        {
            /* add prefix + c to string table */
            string_table[empty_str].base = prefix;
            string_table[empty_str].new_str = c;
            
            /*if(prefix < 512) */
            speed_buffer[prefix * 256 + c] = empty_str;
            
            empty_str ++;
            
            /* output code for prefix */
            output(&buffer, bit_size, prefix, f);
            
            if(empty_str == (1 << bit_size) + 1)
                bit_size ++;
                
            /* make sure string table doesn't overflow */
            if(empty_str == 4095)
            {
                output(&buffer, bit_size, 1 << bpp, f);             /* clear code */
                empty_str = (1 << bpp) + 2;
                bit_size = bpp + 1;
                
                clear_speed_buffer(speed_buffer);
            }
            
            /* set prefix to c */
            prefix = c;
        }
    }
    
    free(speed_buffer);
    
    pack_putc(0x3b, f);                                       /* trailer (end of gif) */
    pack_fclose(f);
    return errno;
}

