#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>

#include "single_instance.h"

#ifndef UNIX_PATH_MAX
  #define UNIX_PATH_MAX 108
#endif

bool is_only_instance(const char* name)
{
    uint32_t fd;
    sockaddr_un sun;
    uint32_t length;
    fd=socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd==-1)
        return false;
    sun.sun_family=AF_UNIX;
    strncpy(sun.sun_path, name, UNIX_PATH_MAX);
    sun.sun_path[UNIX_PATH_MAX-1]='\0';
    length=strlen(sun.sun_path)+sizeof(sun.sun_family);
    if(connect(fd, (sockaddr*)&sun, length)!=-1)
        // Connect succeeded; there must be another instance running already
        return false;
    unlink(sun.sun_path);
    if(bind(fd, (sockaddr*)&sun, length)==-1)
        return false;
    if(listen(fd, 5)==-1)
        return false;
    return true;
}
