#ifndef DECK_H
#define DECK_H

#define DECKSIZE 52
#define SUIT 1
#define FACE 0
#define END 2

/*
FUNCTION NAME: resetDeck
PURPOSE: initializes a new deck. Must be called
        before the deck is used
*/

void resetDeck();

/*
FUNCTION NAME: getCard
PURPOSE: gets a random card from the deck and
        removes that card from the deck saving
        the card to 'saveTo'
*/

void getCard(char *saveTo);

/*
FUNCTION NAME: getValue
PURPOSE: Given a card's character string, returns the
        value of it in blackjack
*/

int getValue(char *card);

/*
FUNCTION NAME: getDealerValue
PURPOSE: Same as getValue but now the 
        ace has a value of 11
*/

int getDealerValue(char *card);

#endif
