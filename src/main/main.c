#include "../../include/server/server.h"

int main()
{
    initscr();
    struct addrinfo* bind_address = congureLocalAddress();
    if(bind_address == NULL)
    {
        return -1;
    }

    int socket_listen = createMainSocket(bind_address);
    if(socket_listen < 0)
        return -1;

    if(listenForConnections(socket_listen) < 0)
        return -1;

    endwin();
}
