#ifndef _CLIENT_H__
#define _CLIENT_H__

#include "Common.hpp"

#define BACKLOG 10


class Client {

private:
  int sockfd;
  struct addrinfo *res;

  
public:
  Client(char *addr, char * port);
  int to_connect();
  int get_sockfd();
  ~Client();
};
#endif
