#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/wait.h>
#include "server.h"

void passwordCheck(int sig);

//#############################################################
//#############################################################

int main (int argc, char *argv[])
{
    fd_set readfds, activefds;
    unsigned short port;
	struct addrinfo hints, *servinfo, *p;
    int S, NS, i;
    struct timeval timeout;
    int totalClients = 0, numRead;
    char portStr[5];

    timeout.tv_usec = 0;

    port = 2269;
    strcpy(portStr, "2269");

    //open socket - INTERNET DOMAIN - TCP
    if ((S = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        ERROR("ERROR: Socket failed to open.\n");

    //Obtain host name & network address
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, portStr, &hints, &servinfo) != 0)
        ERROR("ERROR: Problem getting server address information.\n");

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((S = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
            ERROR("ERROR: Socket failed to open.\n");

        /* Bind socket to port/addr */
        if (bind(S, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(S);
            ERROR("Port failed to open.\n");
            continue;
        }

        break;
    }

    //listen on the new socket
    if (listen(S,MAXPENDING) < 0)
        ERROR("ERROR: Problem while listening.\n");

    //initialize the set of active sockets
    FD_ZERO (&activefds);
    FD_SET (S, &activefds);

    while (1)
    {
		// Block until input arrives on one or more active sockets.
        readfds = activefds;

        //if there is at least one client then start the countdown
        if (totalClients == 0)
            numRead = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        else
            numRead = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);

        if (numRead < 0)
			ERROR("ERROR: Problem with select.\n");
    
        //if the select timed out start the game
        if (numRead == 0)
        {
            startDealer(activefds, totalClients, S);
            totalClients = 0;
            FD_ZERO (&activefds);
            FD_SET (S, &activefds);
        }

        //Service all the sockets with input pending.
        for (i = 0; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET (i, &readfds))
            {
                if (i == S)
                {
                    //create a new socket for the client
					if ((NS = accept(S,NULL,NULL)) < 0)
						ERROR("ERROR: Problem connecting to client.\n");

                    //check to see if the client sends the password
                    int readValue;           
                    int32_t password = 0;  
                    newSocket = NS; //set a global variable so the alarm signal handler can close the fd
                    setAlarm(5, passwordCheck);
                    readValue = read(NS, &password, sizeof(password));

                    //add the new socket to the set if the password is right
                    if (readValue < 0 || !confirmPassword(password))
                    {
                        alarm(0);
                        printf("New Client Disconnected\n");
                        close(NS);
                    }
                    else
                    {
                        alarm(0);
                        FD_SET(NS, &activefds);
                        printf("New Client Connected\n");

                        //create the countdown counter if it is the first client
                        if (totalClients == 0)
                            timeout.tv_sec = 30;
                        totalClients++;
                    }
                }
            }
        }

        //if the maximum clients have been reached then start the game
        if (totalClients == 4)
        {
            startDealer(activefds, totalClients, S);
            totalClients = 0;
            FD_ZERO (&activefds);
            FD_SET (S, &activefds);
        }
    }


    return EXIT_SUCCESS;
}

//#############################################################
//#############################################################

void passwordCheck(int sig)
{
    close(newSocket);
    printf("New client timed out.\n");
    return;
}

//#############################################################
//#############################################################

void reapChildren(int sig) {
	int status;

	while(waitpid(-1,&status,WNOHANG) > 0);
    waitChild = 0;
	signal(SIGCHLD, reapChildren);
}

//#############################################################
//#############################################################

void setAlarm(int sec, void (*handler)(int))
{
    signal(SIGALRM, handler);
    alarm(sec);
}

//#############################################################
//#############################################################

int confirmPassword(int32_t password)
{
    password = ntohl(password);

    if (password == 0xfacebeef)
        return 1;
    else
        return 0;
}

//#############################################################
//#############################################################

void startDealer(fd_set activefds, int numClients, int master)
{
    int i;
    int pid = fork();
    
    signal(SIGCHLD, reapChildren);
    waitChild = 1;

    if (!pid)
    {
        int found = 0;
        char param[numClients + 1][PARAMSIZE];
        char *params[numClients + 2];

        //set the first and last parameters manually
        strcpy(param[0], "./dealer");
        params[0] = param[0];
        params[numClients + 1] = NULL;

        //find the fd to pass as parameters
        for (i = 0; i < FD_SETSIZE && found < numClients; ++i)
        {
            if (i != master && FD_ISSET (i, &activefds))
            {
                found++;
                sprintf(param[found], "%d", i);
                params[found] = param[found];
            }
        }

        //start the dealer process
        if (execvp(params[0], params) == -1)
        {
            fprintf(stderr, "ERROR: The dealer failed to start.\n");
            exit(1);
        }
    }
    else
        while(waitChild == 1);

    printf("Dealer process finished. Accepting new clients.\n");
}
