#include <dlfcn.h>
#include <stdio.h>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern "C"
{
typedef void *(*malloc_function_t) (size_t);
typedef void(*free_function_t) (void *);

static malloc_function_t sysmalloc = NULL;
static free_function_t sysfree = NULL;
static bool has_init = false;
static unsigned long long sendbuf[2];
static int fd;
void initialize() {
    if(!has_init){
        sysmalloc = (malloc_function_t)dlsym(RTLD_NEXT, "malloc");
        sysfree = (free_function_t)dlsym(RTLD_NEXT, "free");
        fd = open("/tmp/fifo_file", O_CREAT|O_WRONLY);
        has_init = true;
    }
}
void* malloc(size_t size) {
    initialize();
    void *result = sysmalloc(size);

    sendbuf[0] = (unsigned long long)result;
    sendbuf[1] = size;
    write(fd, sendbuf, sizeof(sendbuf));
    return result;
}

void free(void *p) {
    initialize();
    sendbuf[0] = (unsigned long long)p;
    sendbuf[1] = 0;
    write(fd, sendbuf, sizeof(sendbuf));
    sysfree(p);
}

}
