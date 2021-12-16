# socket-programs

## Directory structure of source_codes

```
│
├───my_iperf
│       avg_delays.txt
│       my_iperf.cpp
│       plot.py
│       throughputs.txt
│
├───my_ping
│       client.cpp
│       server.cpp
│
├───my_ping_protocol_independent
│       client.cpp
│       server.cpp
│
└───tic_tac_toe
        client.cpp
        server.cpp
```

## Directory structure of screenshots

```
├───my_iperf
│       avg_delay.png
│       client.PNG
│       server.png
│       throughput.png
│       wireshark.png
│
├───my_ping
│       client.PNG
│       server.png
│       wireshark.png
│
├───my_ping_protocol_independent
│       client_ip6-localhost.PNG
│       client_localhost.PNG
│
└───tic_tac_toe
        client.PNG
        server.png
        wireshark.png
```

Screenshots of working of code as well as wireshark packet captures.

g++ version 9.3.0 and Python 3.7.7 has been used.

## my_ping
Compile the client.cpp and server.cpp using

```cpp
g++ client.cpp -o client
g++ server.cpp -o server
```
Run the server and client using the following commands

```cpp
./server 8000
```

8000 specifies the port number. Then on a different terminal run

```cpp
./client -p 8000 -h localhost
```

Port number is specified with -p argument and hostname is specified with -h argument. To get the full argument list use the following command

```cpp
./client -v
```

## my_iperf
Compile the my_iperf.cpp using 

```cpp
g++ my_iperf.cpp -o my_iperf
```

To run in server mode use the following command

```cpp
./my_iperf -s
```

A default port of 8000 is used. To specify the port use the following command

```cpp
./my_iperf -s -p 9000
```

On a different terminal, launch the client with the following command

```cpp
./my_iperf -c localhost
```

To get the full argument list use the following command

```cpp
./my_iperf
```

The data of throughputs and avg delay is stored in throughputs.txt and avg_delays.txt created by my_iperf program during the execution. 

plot.py can be used to plot throughputs and avg delay. Use the following command to plot the data.

```python
python plot.py
```

## tic_tac_toe

A Tic-Tac-Toe game using TCP as transport layer protocol. First the client plays with O and then the server with X and so on. We have used the minmax algorithm on the server side to help the server to choose the best move. The game can be played with multiple clients. We have created threads to handle each client’s request. This ensures that multiple clients can play the game without hindering with other client’s requests.

The client has to enter a number between 0-8 (inclusive). The mapping of these numbers is described in the below matrix

| <!-- --> | <!-- --> | <!-- --> |
|-- | --| --|
| 0 | 1 | 2 | 
| 3 | 4 | 5 |
| 6 | 7 | 8 |

Suppose the client wants to mark at row 1 col 2 (0-based indexing), then the client should type 5 in the console.

Compile the client.cpp and server.cpp using

```cpp
g++ client.cpp -o client
g++ -pthread server.cpp -o server
```

Run the server and client using the following commands

```cpp
./server 8000
```

8000 specifies the port number. Then on a different terminal run

```cpp
./client localhost 8000
```
## my_ping_protocol_independent

To make the server side protocol independent we have used sockaddr storage to store the client’s address. Also, we have used getaddrinfo() function to find the IP address of server in the client side. Therefore, the server can bind both IPv4 and IPv6 IP addresses and the client is capable of resolving and connecting with them.

Compile the client.cpp and server.cpp using

```cpp
g++ client.cpp -o client
g++ server.cpp -o server
```
Run the server and client using the following commands

```cpp
./server 8000
```

8000 specifies the port number. Then on a different terminal run

```cpp
./client -p 8000 -h localhost
```

Port number is specified with -p argument and hostname is specified with -h argument. 

To use IPv6, run the following command

```cpp
./client -p 8000 -h ip6-localhost
```

To get the full argument list use the following command

```cpp
./client -v
```