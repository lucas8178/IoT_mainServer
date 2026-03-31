#include "../../include/server/server.h"
#include "../../include/globalStructures/globalStructures.h"
#include "../../include/database/sensors.h"

/*newGarden Struct allocation*/
newGarden* mallocNewGarden()
{
    newGarden* myGarden = (newGarden*)malloc(sizeof(newGarden));
    myGarden->next = NULL;
    myGarden->previous = NULL;
    return myGarden;
}

/*client Struct allocation*/
client* mallocNewClient()
{
    client* myClient = (client*)malloc(sizeof(client));
    myClient->next = NULL;
    myClient->previous = NULL;
    return myClient;
}

/*Frees the garden allocation*/
newGarden* freeOneGarden(newGarden* toFree)
{
    newGarden* previous = toFree->previous;
    newGarden* next = toFree->next;
    free(toFree);

    if(next == NULL && previous == NULL)
        return NULL;
    else if(previous == NULL)
        return next;
    else
        return previous;

    previous->next = next;
    next->previous = previous;

    return previous;
}

/*Frees the client allocation*/
client* freeOneClient(client* toFree)
{
    client* previous = toFree->previous;
    client* next = toFree->next;
    free(toFree);

    if(next == NULL && previous == NULL)
        return NULL;
    else if(previous == NULL)
        return next;
    else
        return previous;

    previous->next = next;
    next->previous = previous;

    return previous;
}

/*Controls the cursor position of the ReadWin 
 * the data is storaged in socketMessage struct*/
void cursorPosition(socketMessage* myMessage, int y_max, int x_max)
{
    if(myMessage->cursorReadWinY < y_max)
    {
        myMessage->cursorReadWinY += 1;
    } else
    {
	    myMessage->cursorReadWinY = 0;
    }
}

/*Used to show a message in the ReadWin window*/
void showMessageInReadWin(socketMessage* myMessage, char* strMessage)
{   
    int y_max, x_max;
    char** newString= NULL;
    int numberOfStrings = 0;
    getmaxyx(myMessage->readWin, y_max, x_max);

    newString = sliceStringForMessage(strMessage, strlen(strMessage), x_max, &numberOfStrings);

    for(int i = 0; i < numberOfStrings; i++)
    {
        if(myMessage->cursorReadWinY < y_max)
        {
            mvwprintw(myMessage->readWin, myMessage->cursorReadWinY, myMessage->cursorReadWinX, newString[i]);
            wrefresh(myMessage->readWin);
            cursorPosition(myMessage, y_max, x_max);
        } else
        {
            wclear(myMessage->readWin);
            wrefresh(myMessage->readWin);
            mvwprintw(myMessage->readWin, myMessage->cursorReadWinY, myMessage->cursorReadWinX, newString[i]);
            wrefresh(myMessage->readWin);
            cursorPosition(myMessage, y_max, x_max);
        }
    }

    freeMessages(newString, numberOfStrings);
}

/*Used when a message has more characters than one line of the curses window can fit
 * so it splits the message in the need qtd of lines to fit in the window*/
char** sliceStringForMessage(char* currentString, int stringSize, float stringMaxSize, int* newStringSize)
{
    float sizeDiference = stringSize /  stringMaxSize;
    int NumberOfLoops = (int)ceil(sizeDiference);
    char** message = (char**)malloc(NumberOfLoops * sizeof(char*));
    int j = 0, k = 0;

    for(int i = 0; i < NumberOfLoops; i++)
    {
        message[i] = (char*)malloc(stringMaxSize * sizeof(char));
        for(j = 0; j < stringMaxSize-1; j++)
        {
            if(currentString[k] == '\0')
                break;
            message[i][j] = currentString[k];
            k++;
        }
        if(currentString[k] == '\0')
        {
            message[i][j] = '\0';
            break;
        }
        else if(j >= stringMaxSize-1)
        {
            message[i][j] = '\0';
        }
    }

    *newStringSize = NumberOfLoops;
    return message;
}

/*Frees the lines of messages*/
void freeMessages(char** message, int stringSize)
{
    for(int i = 0; i < stringSize; i++)
        free(message[i]);
}

/*Creates the struct for the socket configuration*/
struct addrinfo* congureLocalAddress()
{
    addstr("Configuring local address...\n");
    refresh();
    struct addrinfo hints;
    struct addrinfo* listeningAddress;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int result = getaddrinfo(0, "3001", &hints, &listeningAddress);
    if(result != 0)
    {
        printw("Error: %s\n", gai_strerror(result));
        refresh();
        getch();
        return NULL;
    }
    return listeningAddress;
}

/*binds the configuration to the socket and creates the socket*/
int createMainSocket(struct addrinfo* bind_address)
{
    int socket_listen;

    addstr("Creating file descriptor for the EndPoint");
    refresh();
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if(socket_listen < 0)
    {
        printw("Error: socket() failed. (%d)\n", errno);
        refresh();
        getch();
        return -1;
    }

    addstr("Binding socket to local address...\n");
    refresh();
    if(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        printw("Error: bind failed. (%d)\n", errno);
        refresh();
        getch();
        return -1;
    }

    freeaddrinfo(bind_address);
    return socket_listen;
}

/*a function to be used with pthread, it waits to recive typed messages and send it to the server*/
void* typeMessage(void* arg)
{
    socketMessage* myMessage = (socketMessage*)arg;
    char character = ' ';
    noecho();

    char message[MESSAGESIZE];
    while(1)
    {
        int letterIndex = 0;
        if(myMessage->messageFlag == 0)
        {
            letterIndex = 0;
            while(character != '\n')
            {
                character = wgetch(myMessage->sendWin);

                if((character == 127 || character == 8) && letterIndex > 0)
                {
                    letterIndex--;
                    character = ' ';
                    mvwaddch(myMessage->sendWin, 0, letterIndex, character);
                    wmove(myMessage->sendWin, 0, letterIndex);
                    message[letterIndex] = '\0';
                } else if (letterIndex < COLS)
                {
                    mvwaddch(myMessage->sendWin, 0, letterIndex, character);
                    if(character != '\n')
                    {
                        message[letterIndex] = character;
                    } else 
                        message[letterIndex] = '\0';

                    letterIndex++;
                }
            }
            while(myMessage->messageFlag != 0)
            {
            }
            myMessage->messageFlag = 1;
            strcpy(myMessage->message, message);
            myMessage->messageSize = letterIndex;
            character = ' ';
            myMessage->kindOfMessage = TYPED;
            sendMessage(myMessage);
        }
    }
}

/*Sets the destination of the message recived by the buffer
 * it returns the socket that will recive the message*/
int verifyMessageDestination(socketMessage* myMessage)
{
    int socketMessage;
    if(myMessage->myGarden != NULL && myMessage->myClient != NULL && myMessage->messageId != 0)
    {
        switch(myMessage->kindOfMessage)
        {
            case GARDEN: {
                             client* toLoop = myMessage->myClient;
                             for(toLoop; toLoop->clientId != myMessage->messageId; toLoop = toLoop->next);
                             socketMessage = toLoop->clientSocket;
                             break;
                         }
            case CLIENT: {
                             newGarden* toLoop = myMessage->myGarden;
                             for(toLoop; toLoop->plantId != myMessage->messageId; toLoop = toLoop->next);
                             socketMessage = toLoop->gardenSocket;
                             break;
                         }
            case SERVER: {
                             newGarden* toLoop = myMessage->myGarden;
                             for(toLoop; toLoop->plantId != myMessage->messageId; toLoop = toLoop->next);
                             socketMessage = toLoop->gardenSocket;
                             showMessageInReadWin(myMessage, "Server message");
                             break;
                         }
            default: break;
        } 
    } else if (myMessage->myGarden != NULL && myMessage->messageId != 0)
    {
        switch(myMessage->kindOfMessage)
        {
            case SERVER: {
                             newGarden* toLoop = myMessage->myGarden;
                             for(toLoop; toLoop->plantId != myMessage->messageId; toLoop = toLoop->next);
                             socketMessage = toLoop->gardenSocket;
                             showMessageInReadWin(myMessage, "Server message");
                             break;
                         }
            default: break;
        }
    }

    return socketMessage;
}

/*this function sends the message to the selected socket
 * returns if the action was successful*/
int sendMessageToDestination(socketMessage* myMessage, int socketMessage, fd_set* writes)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    int socketAnswer = 0;
    char buffer[MESSAGESIZE];

    if(select(*myMessage->max_socket+1, 0, writes, 0, &timeout) < 0)
    {
        sprintf(buffer, "Error: select() failed (%d)", errno);
        showMessageInReadWin(myMessage, buffer);
        wgetch(myMessage->readWin);
    }
    int i;

    for(i = 1; i <= *myMessage->max_socket; i++)
    {
        if(myMessage->kindOfMessage != TYPED && myMessage->myGarden != NULL && myMessage->myClient != NULL && myMessage->messageId != 0)
        {
            if(i == socketMessage)
            {
                if(FD_ISSET(i, writes))
                {
                    socketAnswer = send(i, myMessage->message, myMessage->messageSize, 0);
                }
                break;
            }
        } else
        {
            if(FD_ISSET(i, writes))
            {
                socketAnswer = send(i, myMessage->message, myMessage->messageSize, 0);
            }
        }
    }

    return socketAnswer;
}

/*manage the message recived in the buffer of the socketMessage structure*/
void sendMessage(socketMessage* myMessage)
{ 
    int socketMessage;
    int socketAnswer = -1;
    char buffer[MESSAGESIZE];

    if(strcmp(myMessage->message, " "))
    {
        socketMessage = verifyMessageDestination(myMessage);

        wclear(myMessage->sendWin);
        wrefresh(myMessage->sendWin);
        fd_set writes = *myMessage->master;

        while(socketAnswer == -1)
        {  
            socketAnswer = sendMessageToDestination(myMessage, socketMessage, &writes);
        }
        sprintf(buffer, "%s\n", myMessage->message);
        showMessageInReadWin(myMessage, buffer);

        myMessage->messageFlag = 0;
        strcpy(myMessage->message, " ");
        myMessage->messageSize = 0;
        myMessage->kindOfMessage = NONE;
        myMessage->messageId = 0;
        touchwin(myMessage->sendWin);
        wrefresh(myMessage->sendWin);
    }
}

/*Manage a new connection and adds it to our master fd_set*/
int newSocketConnection(socketMessage* myMessage)
{
    char buffer[MESSAGESIZE];
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    int socket_client = accept(myMessage->socket_listen, (struct sockaddr*)&client_address, &client_len);

    if(socket_client < 0)
    {
        sprintf(buffer, "Error: accept() failed. (%d)", errno);
        showMessageInReadWin(myMessage, buffer);
        wgetch(myMessage->readWin);
        return -1;
    }

    FD_SET(socket_client, myMessage->master);
    if(socket_client > *myMessage->max_socket)
        *myMessage->max_socket = socket_client;

    char address_buffer[100];
    getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    sprintf(buffer, "New connection from %s", address_buffer);
    showMessageInReadWin(myMessage, buffer);

    return 0;
}

/*Closes the socket and frees it structure if the connection was closed*/
void clearSocket(socketMessage* myMessage, int i)
{
    int deleted = 0;
    FD_CLR(i, myMessage->master);
    client* toLoop = myMessage->myClient;
    newGarden* gardenToLoop = myMessage->myGarden;
    if(deleted == 0)
    {
        if(toLoop != NULL)
        {
            for(toLoop; toLoop->clientSocket != i && toLoop != NULL; toLoop = toLoop->next)
            {
                if(toLoop != NULL && toLoop->clientSocket == i)
                {
                    myMessage->myClient = freeOneClient(toLoop);
                    showMessageInReadWin(myMessage, "Client disconnected\n");
                    deleted = 1;
                    break;
                }
            }
        }
    }
    if(deleted == 0)
    {
        if(gardenToLoop != NULL)
        {
            for(gardenToLoop; gardenToLoop->gardenSocket != i && gardenToLoop != NULL; gardenToLoop = gardenToLoop->next)
            {
                if(gardenToLoop != NULL && gardenToLoop->gardenSocket == i)
                {
                    myMessage->myGarden = freeOneGarden(gardenToLoop);
                    showMessageInReadWin(myMessage, "Garden disconnected\n");
                    deleted = 1;
                    break;
                }
            }
        }
    }
    close(i);
}

/*recives and manages the messages send to the server*/
int readMessage(socketMessage* myMessage)
{
    char buffer[MESSAGESIZE];
    while(1)
    {
        fd_set reads = *myMessage->master;

        if(select(*myMessage->max_socket+1, &reads, 0, 0, 0) < 0)
        {
            sprintf(buffer, "Error: select() failed. (%d)", errno);
            showMessageInReadWin(myMessage, buffer);
            wgetch(myMessage->readWin);
            return -1;
        }
        int i;

        for(i = 1; i <= *myMessage->max_socket; i++)
        {
            if(FD_ISSET(i, &reads))
            {
                if(i == myMessage->socket_listen)
                {
                    newSocketConnection(myMessage);
                } else
                {
                    char read[1025] = " ";
                    int bytes_received = recv(i, read, 1024, 0);

                    if(bytes_received < 1)
                    {
                        clearSocket(myMessage, i);
                        continue;
                    }

                    serverActions(myMessage, i, read, bytes_received);
                }
                touchwin(myMessage->sendWin);
                wrefresh(myMessage->sendWin);
            }
        }
    }
    return 0;
}

/*manage the socket listening and connections for connections after the whole configuration*/
int listenForConnections(int socket_listen)
{
    addstr("Listening...\n");
    refresh();
    if(listen(socket_listen, 10) < 0)
    {
        printw("Error: listen() failed (%d)\n", errno);
        refresh();
        getch();
        return -1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    int max_socket = socket_listen;

    socketMessage myMessage;
    myMessage.max_socket = &max_socket;
    myMessage.master = &master;
    myMessage.socket_listen = socket_listen;
    myMessage.myGarden = NULL;
    myMessage.messageFlag = 0;
    myMessage.cursorReadWinY = 0;
    myMessage.cursorReadWinX = 0;

    myMessage.sendWin = newwin(LINES/4, COLS, (LINES/4)*3, 0);
    myMessage.readWin = newwin((LINES/4)*3, COLS, 0, 0);
    pthread_t thread1;

    pthread_create(&thread1, NULL, typeMessage, (void*)&myMessage);
    readMessage(&myMessage);


    return 0;
}

/*If an action is connected to a new garden device it decides which action will be taken*/
void newGardenAction(socketMessage* myMessage, cJSON* json, int socket, char* read, int bytes_received)
{
    cJSON* id = cJSON_GetObjectItemCaseSensitive(json, "id");
    cJSON* plantId = cJSON_GetObjectItemCaseSensitive(json, "plantId");
    if(cJSON_IsNumber(id) && id->valueint != 0)
    {
        if(myMessage->myGarden == NULL)
        {
            myMessage->myGarden = mallocNewGarden();
            myMessage->myGarden->ownerId = id->valueint;
            myMessage->myGarden->gardenSocket = socket;
            if(cJSON_IsNumber(plantId) && plantId->valueint != 0)
                myMessage->myGarden->plantId = plantId->valueint;
        } else
        {
            newGarden* toLoop = myMessage->myGarden;
            for(toLoop; toLoop->next != NULL; toLoop = toLoop->next);
            toLoop->next = mallocNewGarden();
            toLoop->next->previous = toLoop;
            toLoop->next->gardenSocket = socket;
            toLoop->next->ownerId = id->valueint;
            if(cJSON_IsNumber(plantId) && plantId->valueint != 0)
                toLoop->next->plantId = plantId->valueint;
        }
        char message[MESSAGESIZE];
        sprintf(message, "%.*s\n", bytes_received, read); 
        while(myMessage->messageFlag != 0)
        {
        }
        myMessage->messageFlag = 1;
        strcpy(myMessage->message, message);
        myMessage->messageSize = bytes_received;
        myMessage->kindOfMessage = GARDEN;
        myMessage->messageId = id->valueint;
        sendMessage(myMessage);
        showMessageInReadWin(myMessage, "a new device is connected.");
    }
}

void saveSensorsData(socketMessage* myMessage, cJSON* json, int socket, char* int)
{
    cJSON* id = cJSON_GetObjectItemCaseSensitive(json, "id");
    cJSON* plantId = cJSON_GetObjectItemCaseSensitive(json, "plantId");
    cJSON* internalTemp = cJSON_GetObjectItemCaseSensitive(json, "internalTemp");
    cJSON* externalTem = cJSON_GetObjectItemCaseSensitive(json, "externalTemp");
    cJSON* moisture = cJSON_GetObjectItemCaseSensitive(json, "moisture");

    //Needs the function to add to the current situation table
}

/*If an action is connected to a garden sensor it decides which action will be taken*/
void gardernSensorsAction(socketMessage* myMessage, cJSON* json, int socket, int bytes_received)
{
            cJSON* id = cJSON_GetObjectItemCaseSensitive(json, "id");
            cJSON* plantId = cJSON_GetObjectItemCaseSensitive(json, "plantId");

            sensors* toGetInfo = mallocSensors();
            toGetInfo->plantId = plantId->valueint;
            searchType searchKind[2] = {ID, END};

            sensors* sensorsInfo = readPlants(toGetInfo, searchKind, searchSensorsQuery);

            char message[MESSAGESIZE];
            sprintf(message, "{\"device\": \"server\", "
                    "\"command\": \"moistureConfig\", "
                    "\"id\": %d, "
                    "\"plantId\": %d, "
                    "\"moistureDry\": %lu, "
                    "\"moistureWet\": %lu}", id->valueint, plantId->valueint, sensorsInfo->minMoisture, sensorsInfo->maxMoisture);

            while(myMessage->messageFlag != 0)
            {
            }
            while(myMessage->messageFlag != 0)
            myMessage->messageFlag = 1;
            strcpy(myMessage->message, message);
            myMessage->messageSize = strlen(message);
            myMessage->kindOfMessage = SERVER;
            myMessage->messageId = plantId->valueint;
            sendMessage(myMessage);
            showMessageInReadWin(myMessage, "The sensors values ware acquired");

            freeSensors(sensorsInfo);
}

/*Manages the actions connected to the garden devices*/
void gardenServerActions(socketMessage* myMessage, cJSON* json, int socket, char* read, int bytes_received)
{
    cJSON* command = cJSON_GetObjectItemCaseSensitive(json, "command");
    if(cJSON_IsString(command) && (command->valuestring != NULL))
    {
        if(!strcmp(command->valuestring, "newGarden") || !strcmp(command->valuestring, "oldGarden"))
        {            
            newGardenAction(myMessage, json, socket, read, bytes_received);
        } else if(!strcmp(command->valuestring, "dataGarden"))
        {
        }
        else if(!strcmp(command->valuestring, "moistureConfig"))
        {
            gardernSensorsAction(myMessage, json, socket, bytes_received);
        }
    }
}


/*If an action is connected to a client device it decides which action will be taken*/
void newClientAction(socketMessage* myMessage, cJSON* json, int socket, char* read, int bytes_received)
{
    cJSON* id = cJSON_GetObjectItemCaseSensitive(json, "id");
    if(myMessage->myClient == NULL)
    {
        myMessage->myClient = mallocNewClient();
        myMessage->myClient->clientSocket = socket;
        if(cJSON_IsNumber(id) && id->valueint != 0)
            myMessage->myClient->clientId = id->valueint;
    } else
    {
        client* toLoop = myMessage->myClient;
        for(toLoop; toLoop->next != NULL; toLoop = toLoop->next);
        toLoop->next = mallocNewClient();
        toLoop->next->previous = toLoop;
        toLoop->next->clientSocket = socket;
        if(cJSON_IsNumber(id) && id->valueint != 0)
            toLoop->next->clientId = id->valueint;
    }


    showMessageInReadWin(myMessage, "a new client is now connected.");

}

/*Sets a new Id to a new plant connected*/
void clientNewIdAction(socketMessage* myMessage, cJSON* json, int socket, char* read, int bytes_received)
{
    cJSON* id = cJSON_GetObjectItemCaseSensitive(json , "id");
    cJSON* plantId = cJSON_GetObjectItemCaseSensitive(json, "plantId");
    newGarden* toLoop = myMessage->myGarden;

    for(toLoop; toLoop->ownerId != id->valueint; toLoop->next);

    toLoop->plantId = plantId->valueint;
    char message[MESSAGESIZE];
    sprintf(message, "%.*s\n", bytes_received, read);
    while(myMessage->messageFlag != 0)
    {
    }
    myMessage->messageFlag = 1;
    strcpy(myMessage->message, message);
    myMessage->messageSize = bytes_received;
    myMessage->kindOfMessage = CLIENT;
    if(cJSON_IsNumber(plantId))
        myMessage->messageId = plantId->valueint;
    sendMessage(myMessage);
    showMessageInReadWin(myMessage, "The new device is now configured.\n");
}

/*Manages the server actions related to clients and garden devices*/
void clientServerActions(socketMessage* myMessage, cJSON* json, int socket, char* read, int bytes_received)
{
    cJSON* command = cJSON_GetObjectItemCaseSensitive(json, "command");

    if(cJSON_IsString(command) && command->valuestring != NULL)
    {
        if(!strcmp(command->valuestring, "newGardenId"))
        { 
            clientNewIdAction(myMessage, json, socket, read, bytes_received);
        } else if(!strcmp(command->valuestring, "newClient"))
        {
            newClientAction(myMessage, json, socket, read, bytes_received);
        }
    }
}

/*manages the actions of the server after it recives messages from gardens and clients that are connected*/
void serverActions(socketMessage* myMessage, int socket, char* read, int bytes_received)
{
    cJSON* json = cJSON_ParseWithLength(read, bytes_received);
    if(json)
    {
        cJSON* device = cJSON_GetObjectItemCaseSensitive(json, "device");
        if(cJSON_IsString(device) && (device->valuestring != NULL))
        {
            if(!strcmp(device->valuestring, "garden"))
            { 
                gardenServerActions(myMessage, json, socket, read, bytes_received);
            } else if(!strcmp(device->valuestring, "client"))
            {
                clientServerActions(myMessage, json, socket, read, bytes_received);
            }
        }

        
    } else
    {
	char buffer[MESSAGESIZE];
        sprintf(buffer, "Received: %.*s\n", bytes_received, read);
	showMessageInReadWin(myMessage, buffer);
    }

    //touchwin(myMessage->sendWin);
    //wrefresh(myMessage->sendWin);

    cJSON_Delete(json);
}
