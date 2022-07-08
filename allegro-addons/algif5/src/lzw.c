#include "algif.h"

static int
read_code (ALLEGRO_FILE * file, char *buf, int *bit_pos, int bit_size)
{
    int i;
    int code = 0;
    int pos = 1;

    for (i = 0; i < bit_size; i++)
    {
        int byte_pos = (*bit_pos >> 3) & 255;

        if (byte_pos == 0)
        {
            int data_len = al_fgetc (file);

            if (data_len == 0)
            {
                //printf ("Fatal. Errorneous GIF stream.\n");
                //abort ();
                return -1;
            }
            al_fread (file, buf + 256 - data_len, data_len);
            byte_pos = 256 - data_len;
            *bit_pos = byte_pos << 3;
        }
        if (buf[byte_pos] & (1 << (*bit_pos & 7)))
            code += pos;
        pos += pos;
        (*bit_pos)++;
    }
    return code;
}

int
LZW_decode (ALLEGRO_FILE * file, ALGIF_BITMAP *bmp)
{
    int orig_bit_size;
    char buf[256];
    int bit_size;
    int bit_pos;
    int clear_marker;
    int end_marker;
    struct
    {
        int prefix;
        int c;
        int len;
    }
    codes[4096];                /* Maximum bit size is 12. */
    int n;
    int i, prev, code, c;
    int out_pos = 0;

    orig_bit_size = al_fgetc (file);
    n = 2 + (1 << orig_bit_size);

    for (i = 0; i < n; i++)
    {
        codes[i].c = i;
        codes[i].len = 0;
    }

    clear_marker = n - 2;
    end_marker = n - 1;

    bit_size = orig_bit_size + 1;

    bit_pos = 0;

    /* Expect to read clear code as first code here. */
    prev = read_code (file, buf, &bit_pos, bit_size);
    //printf("%d %d = %d\n", bit_pos, bit_size, prev);
    if (prev == -1)
        return -1;
    do
    {
        code = read_code (file, buf, &bit_pos, bit_size);
        //printf("%d %d = %d\n", bit_pos, bit_size, code);
        if (code == -1)
            return -1;
        if (code == clear_marker)
        {
            bit_size = orig_bit_size;
            n = 1 << bit_size;
            n += 2;
            bit_size++;
            prev = code;
            continue;
        }

        if (code == end_marker)
            break;

        /* Known code: ok. Else: must be doubled char. */
        if (code < n)
            c = code;
        else
            c = prev;

        /* Output the code. */
        out_pos += codes[c].len;
        i = 0;
        do
        {
            bmp->data[out_pos - i] = codes[c].c;
            if (codes[c].len)
                c = codes[c].prefix;
            else
                break;
            i++;
        }
        while (1);

        out_pos++;

        /* Unknown code -> must be double char. */
        if (code >= n)
        {
            bmp->data[out_pos] = codes[c].c;
            out_pos++;
        }

        /* Except after clear marker, build new code. */
        if (prev != clear_marker)
        {
            codes[n].prefix = prev;
            codes[n].len = codes[prev].len + 1;
            codes[n].c = codes[c].c;
            n++;
        }

        /* Out of bits? Increase. */
        if (n == (1 << bit_size))
        {
            if (bit_size < 12)
                bit_size++;
        }

        prev = code;
    }
    while (1);
    return 0;
}
