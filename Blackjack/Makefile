COMPILER = gcc
FLAGS = -Wall -g
DEALERSRC = dealer.c deck.c
SERVERSRC = server.c


blackjack: default

default: all

all: serverComp dealerComp

dealerComp: 
	$(COMPILER) $(FLAGS) $(DEALERSRC) -o dealer

serverComp: 
	$(COMPILER) $(FLAGS) $(SERVERSRC) -o server

clean:
	rm server dealer
