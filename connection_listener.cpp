#include "connection_listener.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


#define CONNECTION_QUEUE_SIZE 20


ConnectionListener::ConnectionListener(const int port)
  : port(port)
{
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        throw ConnectionException("Couldn't create the TCP socket");
    }
    
    sockaddr_in addr;
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    linger linger_options;
    linger_options.l_onoff = 1;
    linger_options.l_linger = 10;
    
    setsockopt(socketfd, SOL_SOCKET, SO_LINGER, &linger_options, sizeof(struct linger));
    
    int reuse_addr_options = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_options, sizeof(reuse_addr_options));
    
    int err = bind(socketfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    if (err < 0) {
        std::stringstream ss;
        ss<<"Couldn't bind the socket on port "<<port;
        throw ConnectionException(ss.str());
    }
    
    err = listen(socketfd, CONNECTION_QUEUE_SIZE);
    if (err < 0) {
        throw ConnectionException("Error transforming to server socket");
    }
}

ConnectionListener::~ConnectionListener()
{
    close(this->socketfd);
}

Connection ConnectionListener::accept()
{
    sockaddr_in client_info;
    socklen_t addrlen = sizeof(client_info);
    int client_connection;

    std::memset(&client_info, 0, sizeof(client_info));

    client_connection = ::accept(this->socketfd, (sockaddr *) &client_info, &addrlen);
    if (client_connection < 0) {
        throw ConnectionException("Error while accepting new client");
    }
    
    return Connection(client_connection, client_info);
}


