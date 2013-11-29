#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/wait.h>
#include "deck.h"
#include "dealer.h"

//#############################################################
//#############################################################

int main (int argc, char *argv[])
{
    int numClients = argc - 1;
    fd_set readfds, activefds, tempfds;
    char buffer[BUFFSIZE];
    struct hand *myHand = NULL;
    struct hand *temp;
    struct client clients[numClients];
    struct timeval timeout;
    int clientsPlaying = 0;
    int numRead;
    int i;

    resetDeck();

    //have the dealer get its first 2 cards
    myHand = initCard();
    myHand->next = initCard();

    //have the clients get their first 2 cards
    for (i = 0; i < numClients; i++)
    {
        clients[i].hand = initCard();
        clients[i].hand->next = initCard();
    }

    //initialize the active client list
    FD_ZERO (&activefds);
    FD_ZERO (&tempfds);
    for (i = 1; i < argc; i++)
    {
        int socket = atoi(argv[i]);
        FD_SET (socket, &activefds);
        clients[clientsPlaying].socket = socket;
        clients[clientsPlaying].countdown = 10;
        clients[clientsPlaying].finished = 0;

        //send the initial cards to the client
        temp = clients[clientsPlaying].hand;
        strcpy(buffer, temp->card);
        strcat(buffer, temp->next->card);
        strcat(buffer, myHand->card);
        if (write(socket, buffer, sizeof(buffer)) < 0)
        {
            fprintf(stderr, "ERROR contacting client.\n");
            FD_CLR(socket, &activefds);
        }
   
        clientsPlaying++;
    }

    while (clientsPlaying > 0)
    {
        fflush(stderr);
        //disconnect any clients who have timed out
        for (i = 0; i < numClients; i++)
        {
            if ((clients[i].countdown < 1) && (clients[i].finished == 0))
            {
                fprintf(stderr, "A client has timed out.\n");
                FD_CLR(clients[i].socket, &activefds);
                close(clients[i].socket);
                clients[i].finished = 1;
                clientsPlaying--;
            }
        }

        readfds = activefds;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        numRead = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);

        if (numRead < 0)
        {
            fprintf(stderr, "ERROR with select %d\n", errno);
            continue;
        }

        //keep track of the inactivity for each client
        if (numRead == 0)
        {
            fprintf(stderr, ".");
            for (i = 0; i < numClients; i++)
                if (clients[i].finished == 0)
                    clients[i].countdown--;
            continue;
        }

        //check for the client input
        for (i = 0; i < numClients; i++)
        {
            if (FD_ISSET (clients[i].socket, &readfds) && (clients[i].finished == 0))
            {
                if (read(clients[i].socket, buffer, sizeof(buffer)) < 0)
                {
                    fprintf(stderr, "ERROR reading the client's message.\n");
                    continue;
                }

                //give the player another card
                if (strcasecmp(buffer, "HIT") == 0)
                {
                    clients[i].countdown = 10;
                    struct hand *last = clients[i].hand;
                    while (last->next != NULL)
                        last = last->next;

                    last->next = initCard();
                    write(clients[i].socket, last->next->card, sizeof(last->next->card));
                    
                    //check to see if the client bust
                    if (handValue(clients[i].hand) > 21)
                    {
                        fprintf(stderr, "A client has bust.\n");
                        FD_CLR(clients[i].socket, &activefds);
                        close(clients[i].socket);
                        clients[i].finished = 1;
                        clientsPlaying--;
                    }
                }
                else if (strcasecmp(buffer, "STAND") == 0)
                {
                    //remove the client from ongoing gameplay
                    fprintf(stderr, "A client has chosen to stand.\n");
                    clients[i].finished = 1;
                    FD_CLR(clients[i].socket, &activefds);
                    FD_SET(clients[i].socket, &tempfds);
                    clientsPlaying--;
                }
            }
        }
    }

    activefds = tempfds;

    //calculate the rest of the dealer's hand, first check for soft 17
    if ((getValue(myHand->card) == 1 || getValue(myHand->next->card) == 1) && dealerValue(myHand) == 17)
        myHand->next->next = initCard();


    //keep hitting until 17 or higher
    while (dealerValue(myHand) < 17)
    {
        struct hand *last = myHand;
        while (last->next != NULL)
            last = last->next;
        last->next = initCard();
    }

    //output the rest of the dealers hand to the buffer
    temp = myHand->next;
    strcpy(buffer, temp->card);
    temp = temp->next;
    while (temp != NULL)
    {
        strcat(buffer, temp->card);
        temp = temp->next;
    }

    //send the rest of the dealers hand to the clients
    for (i = 0; i < numClients; i++)
    {
        if (FD_ISSET (clients[i].socket, &activefds))
        {
            write(clients[i].socket, buffer, sizeof(buffer));
            close(clients[i].socket);
        }
    }


    return EXIT_SUCCESS;
}

//#############################################################
//#############################################################

struct hand *initCard()
{
    struct hand *obj = malloc(sizeof(struct hand));
    getCard(obj->card);
    obj->next = NULL;
    return obj;
}

//#############################################################
//#############################################################

int handValue (struct hand *head)
{
    int value = 0;

    while (head != NULL)
    {
        value = value + getValue(head->card);
        head = head->next;
    }
    
    return value;
}

//#############################################################
//#############################################################

int dealerValue (struct hand *head)
{
    int value = 0;

    while (head != NULL)
    {
        value = value + getDealerValue(head->card);
        head = head->next;
    }
    
    return value;
}
