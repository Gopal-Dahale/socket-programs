#include <arpa/inet.h>
#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LINE 1024
using namespace std;

// UDP echo server application
int main(int argc, char *argv[])
{
  // Check command line arguments
  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }
  int port = atoi(argv[1]); // First arg:  local port

  int sockfd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addrlen; // Length of addresses
  int n;
  char buffer[1024]; // Message buffer

  // Create socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  assert((sockfd >= 0) && "socket() failed");

  // Initialize server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind socket to the server address
  int bind_result =
      bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  assert(bind_result >= 0 && "bind() failed");

  printf("\nServer Started ...\n");

  while (1)
  {
    cout << "\n";
    addrlen = sizeof(client_addr); // Length of addresses

    // Receive message from client
    n = recvfrom(sockfd, buffer, MAX_LINE, 0, (struct sockaddr *)&client_addr,
                 &addrlen);
    assert((n >= 0) && "recvfrom() failed");

    cout << "Connection from client " << inet_ntoa(client_addr.sin_addr) << ":"
         << ntohs(client_addr.sin_port) << endl;

    string message(buffer);
    cout << "Client's Message: " << message << endl;

    // Send message back to client
    n = sendto(sockfd, buffer, n, 0, (struct sockaddr *)&client_addr, addrlen);
    assert((n >= 0) && "sendto() failed");
  }
  return 0;
}
