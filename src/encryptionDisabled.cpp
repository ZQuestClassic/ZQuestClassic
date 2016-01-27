// This program is free software; you can redistribute it and/or modify it under the terms of the
// modified version 3 of the GNU General Public License. See License.txt for details.

#ifdef USE_ENCRYPTION
    #error "USE_ENCRYPTION is defined; use encryptionEnabled.cpp"
#else

#include "encryption.h"
#include "zdefs.h"
#include "zsys.h"
#include <cstdio>

//
// RETURNS:
//   0 - OK
//   1 - srcfile not opened
//   2 - destfile not opened
//
int encode_file_007(const char *srcfile, const char *destfile, int key2, const char *header, int method)
{
    FILE *src, *dest;
    int c;
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
    
    // encode the data
    while((c=fgetc(src)) != EOF)
    {
        fputc(c, dest);
    }
    
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
    int i, c, size, err;
    
    // open files
    size = file_size_ex_password(srcfile, password);
    
    if(size < 1)
    {
        return 1;
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
        
        fputc(c, dest);
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

