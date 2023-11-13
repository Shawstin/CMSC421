#include <stddef.h>
#include <stdlib.h>
#include <sys/socket.h>

//Makes sure the whole message is recieved and has error handling
ssize_t recv_all(int fd, void* buffer, size_t len, int flags)
{
    size_t bytes_remaining = len;
    while(bytes_remaining > 0)
    {

        ssize_t bytes_recv = recv(fd, &((char*) buffer)[len-bytes_remaining], bytes_remaining, flags);
        if (bytes_recv == -1)
        {
            return -1;
        }
        if (bytes_recv == 0)
        {
            return len - bytes_remaining;
        }
        bytes_remaining -= bytes_recv;
    }
    return len;
}

//Makes sure the whole message is sent and has error handling
ssize_t send_all(int fd, void* buffer, size_t len, int flags)
{
    size_t bytes_remaining = len;
    while(bytes_remaining > 0)
    {

        ssize_t bytes_sent = send(fd, &((char*) buffer)[len-bytes_remaining], bytes_remaining, flags);
        if (bytes_sent == -1)
        {
            return -1;
        }
        bytes_remaining -= bytes_sent;
    }
    return len;
}
