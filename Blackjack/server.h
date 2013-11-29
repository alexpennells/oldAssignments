#ifndef SERVER_H
#define SERVER_H

#define ERROR(x) do { fprintf(stderr, x); return EXIT_FAILURE; } while(0)

#define NAME       257
#define BUFSIZE    257
#define MAXPENDING 1
#define PARAMSIZE  10

int newSocket;
int waitChild;

void setAlarm(int sec, void (*handler)(int));
int confirmPassword(int32_t password);
void startDealer(fd_set activefds, int numClients, int master);

#endif
