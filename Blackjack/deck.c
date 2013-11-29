#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "deck.h"

/*#######################################################*/
/*#######################################################*/

//global variables
static char virtualDeck[DECKSIZE][3];
static int cardsLeft;

/*#######################################################*/
/*#######################################################*/

/*
FUNCTION NAME: getFace
PURPOSE: returns the character for the
        face of a card given its position
        in the array
*/

static char getFace(int card);

/*#######################################################*/
/*#######################################################*/

void resetDeck()
{
    int i;
    char suit = 'H';
    char face;

    cardsLeft = DECKSIZE;

    for (i = 0; i < DECKSIZE; i++)
    {
        //set the suit
        if (i == 13)
            suit = 'C';
        else if (i == 26)
            suit = 'S';
        else if (i == 39)
            suit = 'D';

        face = getFace(i);
        virtualDeck[i][SUIT] = suit;
        virtualDeck[i][FACE] = face;
        virtualDeck[i][END] = '\0';
    }
}

/*#######################################################*/
/*#######################################################*/

void getCard(char *saveTo)
{
    if (cardsLeft < 1)
    {
        saveTo = NULL;
        return;
    }

    //generate a random card out of the leftover cards
    srandom(time(NULL));
    int card = random() % cardsLeft;

    strcpy(saveTo, virtualDeck[card]);

    //swap the chosen card with the last card
    char temp[3];
    strcpy(temp, virtualDeck[card]);
    strcpy(virtualDeck[card], virtualDeck[cardsLeft - 1]);
    strcpy(virtualDeck[cardsLeft - 1], temp);
    cardsLeft--;
}

/*#######################################################*/
/*#######################################################*/

static char getFace(int card)
{
    card = card % (DECKSIZE / 4);

    switch (card)
    {
        case 0: return 'A';
        case 1: return '2';
        case 2: return '3';
        case 3: return '4';
        case 4: return '5';
        case 5: return '6';
        case 6: return '7';
        case 7: return '8';
        case 8: return '9';
        case 9: return 'T';
        case 10: return 'J';
        case 11: return 'Q';
        case 12: return 'K';
    }

    return '0';
}

/*#######################################################*/
/*#######################################################*/

int getValue(char *card)
{
    switch (card[0])
    {
        case 'A': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'T': return 10;
        case 'J': return 10;
        case 'Q': return 10;
        case 'K': return 10;
    }

    return 0;
}

/*#######################################################*/
/*#######################################################*/

int getDealerValue(char *card)
{
    switch (card[0])
    {
        case 'A': return 11;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'T': return 10;
        case 'J': return 10;
        case 'Q': return 10;
        case 'K': return 10;
    }

    return 0;
}
