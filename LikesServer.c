#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "log.h"
#include "protocol.h"
#include "helpers.h"

#define SERVER_NAME "localhost"
#define SERVER_PORT "2000"
#define END_INTERVAL 300

ssize_t recv_all(int, void*, size_t, int);
ssize_t send_all(int, void*, size_t, int);


int main(int argc, char **argv)
{
    //Creates the log file for the LikesServer 
    loginit(argv[0]);

    int server_fd;
    
    //Uses addrinfo with the ip type unsepcified, in tcp mode, and in the state to connect to a socket
    struct addrinfo *result, *rp;
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = 0,
    };

    //uses getaddrinfo to connect to the server socket
    getaddrinfo(SERVER_NAME, SERVER_PORT, &hints, &result);

    //iterates through the results linked list until a successful connection is made
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        server_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(server_fd == -1)
        {
            continue;
        }

        if (connect(server_fd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break;
        }

        close(server_fd);
    }

    //frees the memory of the linked list used in getaddrinfo
    freeaddrinfo(result);

    //Checks for the case where no connection is made
    if (rp == NULL)
    {
        fprintf(stderr, "Failed to connect\n");
        exit(EXIT_FAILURE);
    }

 
    //uses the current nanosecond as a random seed for srand()
	struct timespec tsp;

    clock_gettime(CLOCK_REALTIME, &tsp);
    srand(tsp.tv_nsec);

    struct msg message;

    //gets the likeserver id from the argv[0] and checks for the error case where the args arent correct
    if (sscanf(argv[0], "./LikesServer%d", &message.child_id) != 1)
    {
        fprintf(stderr, "Usage: LikesServer<number>\n");
        exit(EXIT_FAILURE);
    }

    
    message.child_id = htonl(message.child_id);

    //the end time is 5 minutes from the current time
    time_t end_time = time(NULL) + END_INTERVAL;

    //loops until the current time has caught up to the specified end time
    while(time(NULL) < end_time)
    {
        //generates a random number 0-42
        message.num_likes = htonl(rand() % 43);

        //sleeps for 1-5 seconds before sending each message
        sleep(rand() % 5 + 1);

        //sends the message to the server
        send_all(server_fd, &message, sizeof(message), 0);

        struct return_msg rvalue;

        //recieves the response to the message sent as 1 for success and 0 for failure
        ssize_t recv_bytes = recv_all(server_fd, &rvalue, sizeof(rvalue), 0);
        rvalue.return_code = ntohl(rvalue.return_code);
        
   
        log_message("LikesServer %d %d", ntohl(message.num_likes), rvalue.return_code);
    }

}
