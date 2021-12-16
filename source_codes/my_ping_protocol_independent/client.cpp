#include <arpa/inet.h>
#include <cassert>
#include <chrono>
#include <climits>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
using namespace std::chrono;

/**
 * @brief Class to monitor transferred and recieved packets.
 */
struct FlowMonitor
{
  int txPackets;
  int rxPackets;
  FlowMonitor() : txPackets(0), rxPackets(0)
  {
  }
};

/**
 * @brief Usage function
 *
 * This is a helper function to print the usage of the program.
 */
void usage()
{
  cout << "Usage: ./client [ -i INTERVAL ] [ -n NUMBER_OF_ECHO MESSAGES ] [ -l "
          "PACKET_SIZE ]"
       << endl;
  cout << "\t";
  cout << " [ -p PORT ] [ -h HOSTNAME ] [ -v HELP ]" << endl;
  exit(0);
}

int main(int argc, char *argv[])
{
  int ch;
  int interval = 1;      // In seconds
  int num_packets = 4;   // Number of echo messages
  int size = 32;         // Size of each packet
  int timeout = 5000000; // in microseconds
  int port = -1;         // Port number
  string hostname;
  int n;
  struct hostent
      *server; // Pointer to a structure of type hostent defined in netdb.h

  // Timeout
  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  int min_rtt = INT_MAX, max_rtt = 0, avg_rtt = 0; // RTT variables

  // Parse command line arguments
  while ((ch = getopt(argc, argv, "i:n:l:h:p:v")) != -1)
  {
    switch (ch)
    {
    case 'i':
      interval = atoi(optarg);
      break;
    case 'n':
      num_packets = atoi(optarg);
      break;
    case 'l':
      size = atoi(optarg);
      break;
    case 'h':
      hostname = optarg;
      server = gethostbyname(hostname.c_str());
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'v':
      usage();
      break;
    }
  }

  // Check if all the required arguments are provided
  if (port == -1)
  {
    cerr << "Please specify port number -p" << endl;
    return -1;
  }
  if (hostname.empty())
  {
    cerr << "Please specify hostname -h" << endl;
    return -1;
  }

  int sockfd;
  struct in6_addr server_addr; // Server address
  struct addrinfo hints;       // Address information
  struct addrinfo *result, *rp;
  char send_message[size], recv_message[size]; // Send and receive buffers
  FlowMonitor flow;                            // Flow monitor

  memset(&hints, 0, sizeof(hints)); // Initialize hints
  hints.ai_family = AF_UNSPEC;      // Allow IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // Datagram socket
  hints.ai_flags = AI_NUMERICSERV;

  n = inet_pton(AF_INET, hostname.c_str(), &server_addr);
  if (n == 1)
  {
    hints.ai_family = AF_INET;
    hints.ai_flags |= AI_NUMERICHOST;
  }
  else
  {
    n = inet_pton(AF_INET6, hostname.c_str(), &server_addr);
    if (n == 1)
    {
      hints.ai_family = AF_INET6;
      hints.ai_flags |= AI_NUMERICHOST;
    }
  }

  n = getaddrinfo(hostname.c_str(), to_string(port).c_str(), &hints,
                  &result); // Get address information
  assert(n == 0 && "getaddrinfo failed");

  // Loop through all the results and connect to the first we can
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    // Create UDP socket
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1)
      continue;

    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
      break; /* Success */

    close(sockfd);
    sockfd = -1;
  }

  freeaddrinfo(result); // No longer needed
  assert(rp != NULL && "Could not connect");
  assert(sockfd >= 0 && "Could not connect");

  std::cout << "Pinging " << hostname << ":" << port << " with " << size
            << " bytes of data:" << endl;

  // Send and recieve echo messages
  for (int i = 0; i < num_packets; i++)
  {
    auto start = high_resolution_clock::now(); // Retrieve the current time

    // Convert message to char array and store it in buffer
    string message = "Ping: " + to_string(i);
    strcpy(send_message, message.c_str());

    // Send echo packet
    n = write(sockfd, send_message, strlen(send_message));

    // Check if the packet was sent successfully
    if (n < 0)
    {
      std::cout << "Error in sending packet" << endl;
      continue;
    }
    else
      flow.txPackets++;

    // Receive echo packet
    n = read(sockfd, recv_message, sizeof(recv_message));

    // Check if the packet was received successfully
    if (n < 0)
    {
      std::cout << "Packet " << i << " lost" << endl;
      continue;
    }
    else
    {
      auto end = high_resolution_clock::now(); // Retrieve the current time
      auto duration = duration_cast<microseconds>(
          end - start); // Calculate the time difference

      // Wait up to timeout for a reply from the server; if no reply is
      // received, the client should assume and print that the packet was lost
      if (duration.count() > timeout)
        std::cout << "Request timed out" << endl;
      else
      {
        flow.rxPackets++;
        std::cout << "Reply from " << hostname << ":" << port
                  << " bytes sent=" << size << " rtt=" << duration.count()
                  << "µs" << endl;

        // Calculate min, max and avg rtt
        min_rtt = min(min_rtt, (int)duration.count());
        max_rtt = max(max_rtt, (int)duration.count());
        avg_rtt += (int)duration.count();
      }
    }
    // Sleep for interval seconds
    sleep(interval);
  }

  // Close socket
  close(sockfd);

  // Display Ping statistics
  int num_lost_packets = flow.txPackets - flow.rxPackets;
  int loss_percent = (1 - (flow.rxPackets / (float)flow.txPackets)) * 100;

  std::cout << endl;
  std::cout << "Ping statistics for " << hostname << ":" << port << ":" << endl;
  std::cout << "\t";
  std::cout << "Packets: Sent = " << flow.txPackets
            << ", Recieved = " << flow.rxPackets
            << ", Lost = " << num_lost_packets << " (" << loss_percent
            << "% loss)" << endl;

  std::cout << "Approximate round trip times in milli-seconds:" << endl;
  std::cout << "\t";
  std::cout << "Minimum = " << min_rtt << "µs, Maximum = " << max_rtt
            << "µs, Average = " << avg_rtt / (float)flow.rxPackets << "µs"
            << endl;
  return 0;
}