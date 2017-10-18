#ifndef _ENCRYPTION_H_
#define _ENCRYPTION_H_

int encode_file_007(const char *srcfile, const char *destfile, int key, const char *header, int method);
int decode_file_007(const char *srcfile, const char *destfile, const char *header, int method, bool packed, const char *password);

#endif
