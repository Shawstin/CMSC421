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
#include <sys/epoll.h>
#include "log.h"
#include "protocol.h"
#include "helpers.h"

#define SERVER_PORT "2000"
#define EPOLL_MAX_EVENTS 15

//keeps track of the total likes
int g_total_likes = 0;

//prototype
int process_client_data(struct msg message);


int main(int argc, char **argv)
{
    //Creates the log file for the PrimaryLikesServer
    loginit(argv[0]);

    //File descriptors for the server and client
    int server_fd, client_fd;

    //uses addrinfo with unpecified ip type, tcp, and passive for creating a socket that can be binded
    struct addrinfo *result, *rp;
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    //Initializes the socket with getadderinfo with no ip or hostname specified on the SERVER_PORT with the hints as a flag
    getaddrinfo(NULL, SERVER_PORT, &hints, &result);

    //Iterates through the results linked list until the bind is successful
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        server_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server_fd == -1)
        {
            continue;
        }

        if (bind(server_fd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break;
        }

        close(server_fd);

    }

    //Frees the memory of the liked list used in getaddrinfo
    freeaddrinfo(result);

    //Checks for the case where the bind was not successful
    if (rp == NULL)
    {
        log_message("Failed to bind\n");
        exit(EXIT_FAILURE);
    }


    //initializes an epoll
    int epfd = epoll_create(727);

    //creates an event struct to be used in epoll_ctl
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    //Adds the server file descriptor for the epoll to check for changes on it
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event))
    {
        log_message("Failed to create epoll fd\n");
        exit(EXIT_FAILURE);

    }
    
    //Array of epoll events to store all events on the epft epoll
    struct epoll_event epoll_events[EPOLL_MAX_EVENTS];

    //Starts listening for client connections
    listen(server_fd, 10);

    //epoll_wait() returns how many events the epoll is waiting on
    int num_of_events = epoll_wait(epfd, epoll_events, EPOLL_MAX_EVENTS, -1);
    while(num_of_events > 0)
    {
        //Iterates through the array of events until all are processed
        for (int i = 0; i < num_of_events; i++)
        {
            //Checks for the case where the server_fd has an event
            if (epoll_events[i].data.fd == server_fd)
            {
                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd == -1)
                {
                    log_message("Failed to accept client\n");
                    continue;
                }

                event.data.fd = client_fd;
                //Adds a client fd to the epoll to start looking for changes to the file descriptor
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event))
                {
                    log_message("Failed to create epoll fd\n");
                    exit(EXIT_FAILURE);

                }

            }
            //In the case where there is an event on a client fd it is processed
            else
            {
                if (epoll_events[i].events & (EPOLLERR | EPOLLHUP))
                {
                    close(epoll_events[i].data.fd);
                    continue;
                }
                
                //The server recieves the message from the client
                struct msg message;
                ssize_t recv_bytes = recv_all(epoll_events[i].data.fd, &message, sizeof(message), 0);
                if (recv_bytes == -1)
                {
                    log_message("Recieve failed\n");
                    close(epoll_events[i].data.fd);
                    continue;
                }
                
                //error handling for if no bytes are recieved
                if (recv_bytes == 0)
                {
                    close(epoll_events[i].data.fd);
                    continue;
                }

                //In the case of a successful message recieved the server sends back the success
                if (process_client_data(message) == 1)
                {
                    struct return_msg return_value;
                    return_value.return_code = htonl(1);
                    send_all(epoll_events[i].data.fd, &return_value, sizeof(return_value), 0);
                }
                //In the case where the message recieved is not valid it returns a 0 to the client
                else
                {
                    struct return_msg return_value;
                    return_value.return_code = htonl(0);
                    send_all(epoll_events[i].data.fd, &return_value, sizeof(return_value), 0);
                   
                }




            }


        }

        num_of_events = epoll_wait(epfd, epoll_events, EPOLL_MAX_EVENTS, -1);

    }


    close(server_fd);
    return 0;



}

//When a client sends a message it is checked to see if the message is valid and returns 1 if valid and 0 if invalid
int process_client_data(struct msg message)
{
    message.child_id = ntohl(message.child_id); 
    message.num_likes = ntohl(message.num_likes);

    if (message.num_likes < 0 || message.num_likes > 42)
    {
        return 1;
    }
    //for a valid message the total like counter is incremented
    g_total_likes += message.num_likes;
    log_message("LikeServer%d %d", message.child_id, message.num_likes);
    log_message("Total\t%d", g_total_likes);
    return 0;

    
}
