#include <arpa/inet.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define MAX_LINE 1024
using namespace std;
using namespace std::chrono;

/**
 * @brief Class to monitor flow.
 *
 * Monitors transmitted and received packets.
 * Stores throughputs, average delay and transferred bytes
 * per second.
 */
class FlowMonitor
{
public:
  int txPackets;
  int rxPackets;
  std::vector<double> throughputs; // Bytes per second
  std::vector<double> avg_delays;  // Average delay in microseconds
  std::vector<double> transfer;    // Bytes
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
  cout << "my_iperf: parameter error - must either be a client (-c) or server "
          "(-s)"
       << endl
       << endl;
  cout << "Usage: ./my_iperf [-s|-c host] [options]" << endl << endl;
  cout << "Server or Client:" << endl;
  cout << "\t"
       << "-p,  #         server port to listen on/connect to (default: 8000)"
       << endl;
  cout << "\t"
       << "-i,  #         time between transfer of each packet (in "
          "microseconds) (default: 1s)"
       << endl;
  cout << "Server specific:" << endl;
  cout << "\t"
       << "-s,            run in server mode" << endl;
  cout << "Client specific:" << endl;
  cout << "\t";
  cout << "-c, <host>     run in client mode, connecting to <host>" << endl;
  cout << "\t"
       << "-t,  #         time in seconds to transmit for (default 10 secs)"
       << endl;
  cout << "\t"
       << "-l,  #         length of each message" << endl;
  exit(0);
}

/**
 * @brief Function to write throughputs and avg delays values to a file.
 * @param flow Monitor object.
 */
void write_to_file(FlowMonitor &flow)
{
  // Write throughputs and avg_delays to throughputs.txt and avg_delays.txt
  ofstream throughput_file, avg_delay_file;

  throughput_file.open("throughputs.txt");
  avg_delay_file.open("avg_delays.txt");

  for (int i = 0; i < flow.throughputs.size(); i++)
    throughput_file << flow.throughputs[i] << endl;

  for (int i = 0; i < flow.avg_delays.size(); i++)
    avg_delay_file << flow.avg_delays[i] << endl;

  throughput_file.close();
  avg_delay_file.close();
}

/**
 * @brief Function to print header.
 */
void print_header()
{
  cout << setw(12) << "Interval" << setw(20) << "Transfer" << setw(15)
       << "Bandwidth" << setw(15) << "Avg Delay" << endl;
}

/**
 * @brief Function to print the values.
 * @param s Start time of the invterval.
 * @param e End time of the interval.
 * @param transfer Transferred bits/bytes.
 * @param bandwidth Bandwidth in bits or bytes per second.
 * @param avg_delay Average delay in micro or milli seconds.
 * @param transfer_unit Unit of transfer.
 * @param bandwidth_unit Unit of bandwidth.
 * @param avg_delay_unit Unit of avg_delay.
 * @param precision Precision of the values.
 */
void print_data(int s, int e, double transfer, double bandwidth,
                double avg_delay, string transfer_unit = "bits",
                string bandwidth_unit = "bits/s", string avg_delay_unit = "µs",
                int precision = 0)
{
  transfer_unit = " " + transfer_unit;
  bandwidth_unit = " " + bandwidth_unit;
  avg_delay_unit = " " + avg_delay_unit;

  cout << setw(3) << s << "-" << e << setw(10) << "sec" << setw(10) << fixed
       << setprecision(precision) << transfer << transfer_unit << setw(12)
       << fixed << setprecision(precision) << bandwidth << bandwidth_unit
       << setw(12) << fixed << setprecision(precision) << avg_delay
       << avg_delay_unit << endl;
}

/**
 * @brief Divider line
 */
void divider()
{
  cout << "--------------------------------------------------------------------"
          "----------------"
       << endl;
}

int main(int argc, char **argv)
{
  int ch;
  bool is_server = false; // Is server or client
  string host;
  int port = 8000;        // Port to listen on/connect to
  int interval = 1000000; // in microseconds
  int time = 10;          // Time in seconds to transmit for
  int size = 32;          // Size of each packet
  int timeout = 5000000;  // in microseconds
  struct hostent *server;

  // Parse command line arguments
  while ((ch = getopt(argc, argv, "t:l:i:p:sc:")) != -1)
  {
    switch (ch)
    {
    case 't':
      time = atoi(optarg);
      break;
    case 'l':
      size = atoi(optarg);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'i':
      interval = atoi(optarg);
      break;
    case 's':
      is_server = true;
      break;
    case 'c':
      is_server = false;
      host = optarg;
      server = gethostbyname(host.c_str());
      break;
    }
  }
  if (argc == 1)
    usage();

  // Server Mode
  if (is_server)
  {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen; // Length of addresses
    int n;
    char buffer[1024]; // Buffer for data

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

    cout << "\nServer Listening on " << port << endl;

    while (1)
    {
      addrlen = sizeof(client_addr); // Length of addresses

      // Receive message from client
      n = recvfrom(sockfd, buffer, MAX_LINE, 0, (struct sockaddr *)&client_addr,
                   &addrlen);
      assert((n >= 0) && "recvfrom() failed");

      cout << "Connection from client " << inet_ntoa(client_addr.sin_addr)
           << ":" << ntohs(client_addr.sin_port) << endl;

      string message(buffer);
      cout << "Client's Message: " << message << endl;

      // Send message back to client
      n = sendto(sockfd, buffer, n, 0, (struct sockaddr *)&client_addr,
                 addrlen);
      assert((n >= 0) && "sendto() failed");
    }
  }
  // Client Mode
  else
  {
    int sockfd;
    struct sockaddr_in server_addr;              // Server address
    socklen_t addrlen;                           // Length of address
    char send_message[size], recv_message[size]; // Buffer for data
    int n;
    FlowMonitor flow; // Flow monitor object

    // Timeout
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    assert((sockfd >= 0) && "socket() failed");

    // Initialize server address
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,
          server->h_length);

    auto time_start = high_resolution_clock::now(); // Start time
    auto time_end = time_start;                     // End time
    auto duration = duration_cast<seconds>(
        time_end - time_start); // Calculate the time difference

    int msg_index = 0;    // Index of the message
    int i = 0;            // Index of the interval
    double avg_delay = 0; // In microseconds

    print_header();

    while (i < time)
    {
      auto rtt_start =
          high_resolution_clock::now(); // Retrieve the current time
      addrlen = sizeof(server_addr);    // Length of address

      // Convert message to char array and store it in buffer
      string message = "Ping: " + to_string(msg_index);
      strcpy(send_message, message.c_str());
      msg_index++; // Increment the message index

      // Send echo packet
      n = sendto(sockfd, send_message, sizeof(send_message), 0,
                 (struct sockaddr *)&server_addr, addrlen);
      if (n < 0)
      {
        std::cout << "Error in sending packet" << endl;
        continue;
      }
      else
        flow.txPackets++;

      // Receive echo packet
      n = recvfrom(sockfd, recv_message, sizeof(recv_message), 0,
                   (struct sockaddr *)&server_addr, &addrlen);

      if (n < 0)
      {
        std::cout << "Packet " << i << " lost" << endl;
        continue;
      }
      else
      {
        auto rtt_end =
            high_resolution_clock::now(); // Retrieve the current time
        auto rtt = duration_cast<microseconds>(
            rtt_end - rtt_start); // Calculate the time difference

        // Wait up to timeout for a reply from the server; if no reply is
        // received, the client should assume and print that the packet was lost
        if (rtt.count() > timeout)
          std::cout << "Request timed out" << endl;
        else
        {
          flow.rxPackets++;
          avg_delay += rtt.count();
        }
      }
      // Sleep for interval microseconds
      usleep(interval);

      duration =
          duration_cast<seconds>(high_resolution_clock::now() -
                                 time_end); // Calculate the time difference

      // Display data after every 1 second
      if (duration.count() >= 1)
      {
        i++;
        int throughput = 8 * flow.rxPackets * size; // Per second
        time_end = high_resolution_clock::now();    // End time
        avg_delay = avg_delay / flow.rxPackets;     // Average delay

        flow.throughputs.push_back(throughput / 8.0); // Store throughput value
        flow.avg_delays.push_back(avg_delay);         // Store avg delay value

        double transferred_mega_bytes = (flow.txPackets * size) / 1000000.0;
        flow.transfer.push_back(
            transferred_mega_bytes); // Store transferred mega bytes

        print_data(i - 1, i, flow.rxPackets, throughput, avg_delay);

        // Reset values
        flow.txPackets = 0;
        flow.rxPackets = 0;
      }
    }
    // Close socket
    close(sockfd);

    divider();
    print_header();

    // Calcluate avg throughput over the interval
    double avg_throughput =
        accumulate(flow.throughputs.begin(), flow.throughputs.end(), 0.0) /
        (double)flow.throughputs.size();
    avg_throughput = avg_throughput / 1000000.0; // Mega Bytes

    // Calcluate avg delay over the interval
    double avg_avg_delay =
        accumulate(flow.avg_delays.begin(), flow.avg_delays.end(), 0.0) /
        flow.avg_delays.size();

    // Calcluate avg transferred mega bytes over the interval
    double avg_transfer =
        accumulate(flow.transfer.begin(), flow.transfer.end(), 0.0) /
        (double)flow.transfer.size();

    // Print the final data
    print_data(0, 10, avg_transfer, avg_throughput, avg_avg_delay, "MB", "MBps",
               "µs", 2);

    cout << endl << "my_iperf done" << endl << endl;

    write_to_file(flow); // Write to file
  }

  return 0;
}