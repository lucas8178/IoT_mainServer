#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ncurses.h>
#define MESSAGESIZE 1024

typedef enum messageType
{
    TYPED, GARDEN, CLIENT, SERVER, NONE
} messageType;

typedef struct client
{
    int clientSocket;
    int clientId;
    struct client* previous;
    struct client* next;
} client;

typedef struct newGarden
{
    int gardenSocket;
    int plantId;
    int ownerId;
    struct newGarden* previous;
    struct newGarden* next;
} newGarden;


typedef struct socketMessage
{
    int* max_socket;
    fd_set* master;
    int socket_listen;
    int messageFlag;
    int messageSize;
    int messageId;
    char message[MESSAGESIZE];
    messageType kindOfMessage;
    WINDOW* sendWin;
    WINDOW* readWin;
    int cursorReadWinX;
    int cursorReadWinY;
    newGarden* myGarden;
    client* myClient;
} socketMessage;


newGarden* mallocNewGarden();
client* mallocNewClient();
newGarden* freeOneGarden(newGarden* toFree);
client* freeOneClient(client* toFree);
void cursorPosition(socketMessage* myMessage, int y_max, int x_max);
void showMessageInReadWin(socketMessage* myMessage, char* strMessage);
char** sliceStringForMessage(char* currentString, int stringSize, float stringMaxSize, int* newStringSize);
void freeMessages(char** message, int stringSize);

struct addrinfo* congureLocalAddress();
int createMainSocket(struct addrinfo* bind_address);
void* typeMessage(void* arg);
int verifyMessageDestination(socketMessage* myMessage);
int sendMessageToDestination(socketMessage* myMessage, int socketMessage, fd_set* writes);
void sendMessage(socketMessage* myMessage);
int readMessage(socketMessage* myMessage);
int listenForConnections(int socket_listen);
void serverActions(socketMessage* myMessage, int socket, char* read, int bytes_received);
