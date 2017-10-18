#ifndef USE_ENCRYPTION
    #error "USE_ENCRYPTION is not defined; use encryptionDisabled.cpp"
#else

#include "encryption.h"
#include "zdefs.h"
#include "zsys.h"
#include <cstdio>

//#define MASK 0x4C358938
static int seed = 0;
//#define MASK 0x91B2A2D1
//static int seed = 7351962;
static int enc_mask[ENC_METHOD_MAX]= {0x4C358938,0x91B2A2D1,0x4A7C1B87,0xF93941E6,0xFD095E94};
static int pvalue[ENC_METHOD_MAX]= {0x62E9,0x7D14,0x1A82,0x02BB,0xE09C};
static int qvalue[ENC_METHOD_MAX]= {0x3619,0xA26B,0xF03C,0x7B12,0x4E8F};

static int rand_007(int method)
{
    short BX = seed >> 8;
    short CX = (seed & 0xFF) << 8;
    signed char AL = seed >> 24;
    signed char C = AL >> 7;
    signed char D = BX >> 15;
    AL <<= 1;
    BX = (BX << 1) | C;
    CX = (CX << 1) | D;
    CX += seed & 0xFFFF;
    BX += (seed >> 16) + C;
    //  CX += 0x62E9;
    //  BX += 0x3619 + D;
    CX += pvalue[method];
    BX += qvalue[method] + D;
    seed = (BX << 16) + CX;
    return (CX << 16) + BX;
}

void encode_007(byte *buf, dword size, dword key2, word *check1, word *check2, int method)
{
    dword i;
    byte *p;
    
    *check1 = 0;
    *check2 = 0;
    
    p = buf;
    
    for(i=0; i<size; i++)
    {
        *check1 += *p;
        *check2 = (*check2 << 4) + (*check2 >> 12) + *p;
        ++p;
    }
    
    p = buf;
    seed = key2;
    
    for(i=0; i<size; i+=2)
    {
        byte q = rand_007(method);
        *p ^= q;
        ++p;
        
        if(i+1 < size)
        {
            *p += q;
            ++p;
        }
    }
}

bool decode_007(byte *buf, dword size, dword key2, word check1, word check2, int method)
{
    dword i;
    word c1 = 0, c2 = 0;
    byte *p;
    
    p = buf;
    seed = key2;
    
    for(i=0; i<size; i+=2)
    {
        unsigned char q = rand_007(method);
        *p ^= q;
        ++p;
        
        if(i+1 < size)
        {
            *p -= q;
            ++p;
        }
    }
    
    p = buf;
    
    for(i=0; i<size; i++)
    {
        c1 += *p;
        c2 = (c2 << 4) + (c2 >> 12) + *p;
        ++p;
    }
    
    return (c1 == check1) && (c2 == check2);
}

//
// RETURNS:
//   0 - OK
//   1 - srcfile not opened
//   2 - destfile not opened
//
int encode_file_007(const char *srcfile, const char *destfile, int key2, const char *header, int method)
{
    FILE *src, *dest;
    int tog = 0, c, r=0;
    short c1 = 0, c2 = 0;
    
    seed = key2;
    src = fopen(srcfile, "rb");
    
    if(!src)
        return 1;
        
    dest = fopen(destfile, "wb");
    
    if(!dest)
    {
        fclose(src);
        return 2;
    }
    
    
    // write the header
    if(header)
    {
        for(c=0; header[c]; c++)
            fputc(header[c], dest);
    }
    
    // write the key, XORed with MASK
    key2 ^= enc_mask[method];
    fputc(key2>>24, dest);
    fputc((key2>>16)&255, dest);
    fputc((key2>>8)&255, dest);
    fputc(key2&255, dest);
    
    // encode the data
    while((c=fgetc(src)) != EOF)
    {
        c1 += c;
        c2 = (c2 << 4) + (c2 >> 12) + c;
        
        if(tog)
            c += r;
        else
        {
            r = rand_007(method);
            c ^= r;
        }
        
        tog ^= 1;
        
        fputc(c, dest);
    }
    
    // write the checksums
    r = rand_007(method);
    c1 ^= r;
    c2 += r;
    fputc(c1>>8, dest);
    fputc(c1&255, dest);
    fputc(c2>>8, dest);
    fputc(c2&255, dest);
    
    fclose(src);
    fclose(dest);
    return 0;
}

//
// RETURNS:
//   0 - OK
//   1 - srcfile not opened
//   2 - destfile not opened
//   3 - scrfile too small
//   4 - srcfile EOF
//   5 - checksum mismatch
//   6 - header mismatch
//
int decode_file_007(const char *srcfile, const char *destfile, const char *header, int method, bool packed, const char *password)
{
    FILE *normal_src=NULL, *dest=NULL;
    PACKFILE *packed_src=NULL;
    int tog = 0, c, r=0, err;
    long size, i;
    short c1 = 0, c2 = 0, check1, check2;
    
    // open files
    size = file_size_ex_password(srcfile, password);
    
    if(size < 1)
    {
        return 1;
    }
    
    size -= 8;                                                // get actual data size, minus key and checksums
    
    if(size < 1)
    {
        return 3;
    }
    
    if(!packed)
    {
        normal_src = fopen(srcfile, "rb");
        
        if(!normal_src)
        {
            return 1;
        }
    }
    else
    {
        packed_src = pack_fopen_password(srcfile, F_READ_PACKED,password);
        
        if(errno==EDOM)
        {
            packed_src = pack_fopen_password(srcfile, F_READ,password);
        }
        
        if(!packed_src)
        {
            return 1;
        }
    }
    
    dest = fopen(destfile, "wb");
    
    if(!dest)
    {
        if(packed)
        {
            pack_fclose(packed_src);
        }
        else
        {
            fclose(normal_src);
        }
        
        return 2;
    }
    
    // read the header
    err = 4;
    
    if(header)
    {
        for(i=0; header[i]; i++)
        {
            if(packed)
            {
                if((c=pack_getc(packed_src)) == EOF)
                {
                    goto error;
                }
            }
            else
            {
                if((c=fgetc(normal_src)) == EOF)
                {
                    goto error;
                }
            }
            
            if((c&255) != header[i])
            {
                err = 6;
                goto error;
            }
            
            --size;
        }
    }
    
    // read the key
    if(packed)
    {
        if((c=pack_getc(packed_src)) == EOF)
        {
            goto error;
        }
    }
    else
    {
        if((c=fgetc(normal_src)) == EOF)
        {
            goto error;
        }
    }
    
    seed = c << 24;
    
    if(packed)
    {
        if((c=pack_getc(packed_src)) == EOF)
        {
            goto error;
        }
    }
    else
    {
        if((c=fgetc(normal_src)) == EOF)
        {
            goto error;
        }
    }
    
    seed += (c & 255) << 16;
    
    if(packed)
    {
        if((c=pack_getc(packed_src)) == EOF)
        {
            goto error;
        }
    }
    else
    {
        if((c=fgetc(normal_src)) == EOF)
        {
            goto error;
        }
    }
    
    seed += (c & 255) << 8;
    
    if(packed)
    {
        if((c=pack_getc(packed_src)) == EOF)
        {
            goto error;
        }
    }
    else
    {
        if((c=fgetc(normal_src)) == EOF)
        {
            goto error;
        }
    }
    
    seed += c & 255;
    seed ^= enc_mask[method];
    
    // decode the data
    for(i=0; i<size; i++)
    {
        if(packed)
        {
            if((c=pack_getc(packed_src)) == EOF)
            {
                goto error;
            }
        }
        else
        {
            if((c=fgetc(normal_src)) == EOF)
            {
                goto error;
            }
        }
        
        if(tog)
        {
            c -= r;
        }
        else
        {
            r = rand_007(method);
            c ^= r;
        }
        
        tog ^= 1;
        
        c &= 255;
        c1 += c;
        c2 = (c2 << 4) + (c2 >> 12) + c;
        
        fputc(c, dest);
    }
    
    // read checksums
    if(packed)
    {
        if((c=pack_getc(packed_src)) == EOF)
        {
            goto error;
        }
    }
    else
    {
        if((c=fgetc(normal_src)) == EOF)
        {
            goto error;
        }
    }
    
    check1 = c << 8;
    
    if(packed)
    {
        if((c=pack_getc(packed_src)) == EOF)
        {
            goto error;
        }
    }
    else
    {
        if((c=fgetc(normal_src)) == EOF)
        {
            goto error;
        }
    }
    
    check1 += c & 255;
    
    if(packed)
    {
        if((c=pack_getc(packed_src)) == EOF)
        {
            goto error;
        }
    }
    else
    {
        if((c=fgetc(normal_src)) == EOF)
        {
            goto error;
        }
    }
    
    check2 = c << 8;
    
    if(packed)
    {
        if((c=pack_getc(packed_src)) == EOF)
        {
            goto error;
        }
    }
    else
    {
        if((c=fgetc(normal_src)) == EOF)
        {
            goto error;
        }
    }
    
    check2 += c & 255;
    
    // verify checksums
    r = rand_007(method);
    check1 ^= r;
    check2 -= r;
    check1 &= 0xFFFF;
    check2 &= 0xFFFF;
    
    if(check1 != c1 || check2 != c2)
    {
        err = 5;
        goto error;
    }
    
    if(packed)
    {
        pack_fclose(packed_src);
    }
    else
    {
        fclose(normal_src);
    }
    
    fclose(dest);
    return 0;
    
error:

    if(packed)
    {
        pack_fclose(packed_src);
    }
    else
    {
        fclose(normal_src);
    }
    
    fclose(dest);
    delete_file(destfile);
    return err;
}

#endif
