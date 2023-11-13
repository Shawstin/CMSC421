#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include "log.h"

#define LIKE_SERVER_COUNT 10
#define FILE_NAME_LENGTH 50

//prototype for the helper function that starts a likesserver
void run_likes_server(int servernum); 

int main(int argc, char **argv)
{
    //Creates the file for the log to write to
    loginit(argv[0]);

    //Stores the PIDs for each fork
    pid_t like_server_pids[LIKE_SERVER_COUNT];

    //Forks into child processes then stores the pids of each fork
    for(int i = 0; i < LIKE_SERVER_COUNT; i++)
    {
        pid_t pid;
        if((pid = fork()) > 0)
        {
            like_server_pids[i] = pid;
        }
        else
        {
            //waits a second before creating the child likesserver
            sleep(1);
            run_likes_server(i);
            exit(EXIT_FAILURE);
        }
    }

    
    //waits for each child process to die then logs it
    for(int i = 0; i < LIKE_SERVER_COUNT; i++)
    {
        int status;
        pid_t pid = wait(&status);

        if(WIFEXITED(status))
        {
            for(int j = 0; j < LIKE_SERVER_COUNT; j++)
            {
                if(pid == like_server_pids[j])
                {
                    log_message("Child%d Ended", j);
                }
            }
        }
        
    }

    return 0;
}

//Helper function to create the likeservers and logs the creation of them
void run_likes_server(int servernum)
{
    char buffer[FILE_NAME_LENGTH];
    sprintf(buffer, "./LikesServer%d", servernum);
    log_message("Child%d Started", servernum);
    execl("./LikesServer", buffer, NULL);
    
}
