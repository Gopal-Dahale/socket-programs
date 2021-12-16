#include <arpa/inet.h>
#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX_LINE 1024

using namespace std;

/**
 * @brief Display client's IP address and port number
 * @param client_addr sockaddr struct
 */
void display_address(struct sockaddr *client_addr)
{
  void *addr;                    // IP address
  char buffer[INET6_ADDRSTRLEN]; // Buffer for address conversion
  in_port_t port;                // Port number

  // Check if IP version is IPv4
  if (client_addr->sa_family == AF_INET)
  {
    struct sockaddr_in *ipv4 =
        (struct sockaddr_in *)client_addr; // Cast to IPv4 address
    addr = &(ipv4->sin_addr);              // Get IP address
    port = ntohs(ipv4->sin_port);          // Get port number
  }
  // Check if IP version is IPv6
  else if (client_addr->sa_family == AF_INET6)
  {
    struct sockaddr_in6 *ipv6 =
        (struct sockaddr_in6 *)client_addr; // Cast to IPv6 address
    addr = &(ipv6->sin6_addr);              // Get IP address
    port = ntohs(ipv6->sin6_port);          // Get port number
  }
  else
    fprintf(stderr, "Unsupported address family\n"); // Error

  // Convert IP address to a string and print it
  auto n = inet_ntop(client_addr->sa_family, addr, buffer, sizeof(buffer));
  if (n == NULL)
    cout << "Invalid Address" << endl;
  else
    cout << "Client Address: " << buffer << ":" << port << endl;
}

// UDP echo server application
int main(int argc, char *argv[])
{
  // Check command line arguments
  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  int port = atoi(argv[1]); // First arg: port number

  int sockfd;
  socklen_t addrlen;                   // Length of client address
  struct sockaddr_in6 server_addr;     // Server address
  struct sockaddr_storage client_addr; // Client address
  int n;
  char buffer[MAX_LINE]; // Buffer for echo string

  // Create socket
  sockfd = socket(AF_INET6, SOCK_STREAM, 0);
  assert((sockfd >= 0) && "socket() failed");

  // Initialize server address
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = htons(port);
  server_addr.sin6_addr = in6addr_any;

  // Bind socket to the server address
  n = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  assert((n >= 0) && "bind() failed");

  listen(sockfd, 5); // Listen for client connection requests
  printf("\nServer Started ...\n");

  while (1)
  {
    cout << "\n";
    addrlen = sizeof(client_addr); // Length of client address
    int newsockfd = accept(sockfd, (struct sockaddr *)&client_addr,
                           &addrlen); // Accept connection
    assert((newsockfd >= 0) && "accept() failed");

    printf("\nNew Connection from client ");
    display_address((struct sockaddr *)&client_addr); // Display client address

    bzero(buffer, 256);

    // Receive message from client
    n = read(newsockfd, buffer, MAX_LINE);
    assert((n >= 0) && "read() failed");

    string message(buffer);
    cout << "Client's Message: " << message << endl;

    // Send message back to client
    n = write(newsockfd, buffer, MAX_LINE);
    assert((n >= 0) && "write() failed");
  }
  return 0;
}
