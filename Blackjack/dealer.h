#ifndef DEALER_H
#define DEALER_H

#define BUFFSIZE 10

struct hand
{
    char card[3];
    struct hand *next;
};

struct client
{
    int socket;
    int countdown;
    int finished;
    struct hand *hand;
};

struct hand *initCard();

int handValue (struct hand *head);

int dealerValue (struct hand *head);

#endif
