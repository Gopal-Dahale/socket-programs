#include <algorithm>
#include <argp.h>
#include <arpa/inet.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

/**
 * @brief Structure for move. Stores the row and column number of the move.
 */
struct Move
{
  int row;
  int col;
};

using namespace std;

bool isMovesLeft(vector<vector<int>> &game_board)
{
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      if (game_board[i][j] == -1)
        return true;
  return false;
}

int evaluate_board(vector<vector<int>> &game_board)
{
  // Rows
  for (int i = 0; i < 3; i++)
  {
    if (game_board[i][0] == game_board[i][1] &&
        game_board[i][1] == game_board[i][2])
    {
      if (game_board[i][0] == 1)
        return +10;
      else if (game_board[i][0] == 0)
        return -10;
    }
  }

  // Columns
  for (int j = 0; j < 3; j++)
  {
    if (game_board[0][j] == game_board[1][j] &&
        game_board[1][j] == game_board[2][j])
    {
      if (game_board[0][j] == 1)
        return +10;

      else if (game_board[0][j] == 0)
        return -10;
    }
  }

  // Diagnols
  if (game_board[0][0] == game_board[1][1] &&
      game_board[1][1] == game_board[2][2])
  {
    if (game_board[0][0] == 1)
      return +10;
    else if (game_board[0][0] == 0)
      return -10;
  }

  if (game_board[0][2] == game_board[1][1] &&
      game_board[1][1] == game_board[2][0])
  {
    if (game_board[0][2] == 1)
      return +10;
    else if (game_board[0][2] == 0)
      return -10;
  }

  return 0;
}

int minimax(vector<vector<int>> &game_board, int depth, bool is_max)
{
  int board_score = evaluate_board(game_board);

  if (board_score == 10)
    return board_score;

  if (board_score == -10)
    return board_score;

  if (isMovesLeft(game_board) == false)
    return 0;

  if (is_max)
  {
    int best = -1000;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        if (game_board[i][j] == -1)
        {
          game_board[i][j] = 1;
          best = max(best, minimax(game_board, depth + 1, !is_max));
          game_board[i][j] = -1;
        }
      }
    }
    return best;
  }
  else
  {
    int best = 1000;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        if (game_board[i][j] == -1)
        {
          game_board[i][j] = 0;
          best = min(best, minimax(game_board, depth + 1, !is_max));
          game_board[i][j] = -1;
        }
      }
    }
    return best;
  }
}

/**
 * @brief Function to get the best move for the Server.
 * @param game_board The current state of the game.
 * @return The best move for the Server.
 */
Move find_best_move(vector<vector<int>> &game_board)
{
  int best_val = -1000;
  Move best_move;
  best_move.row = -1;
  best_move.col = -1;

  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      if (game_board[i][j] == -1) // Empty celll
      {
        game_board[i][j] = 1; // server makes move
        int move_val = minimax(game_board, 0, false);
        game_board[i][j] = -1; // Undo the move
        if (move_val > best_val)
        {
          best_move.row = i;
          best_move.col = j;
          best_val = move_val;
        }
      }
    }
  }

  return best_move;
}

/**
 * @brief Function to handle client request
 * @param p_client The client socket.
 */
void *handle_clients(void *p_client)
{

  vector<vector<int>> game_board{{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}};
  int sockfd = *((int *)p_client);
  free(p_client);

  int n;
  int client_num = 0, server_num = 0; // client's and server's chosen number
  while (1)
  {
    // Receive message from client
    n = read(sockfd, &client_num, sizeof(client_num));
    assert((n >= 0) && "read() failed");

    game_board[client_num / 3][client_num % 3] = 0; // Client makes move

    Move best_move = find_best_move(game_board);    // Server makes move
    server_num = best_move.row * 3 + best_move.col; // Server's move

    if (server_num < 0 && server_num > 8)
      return nullptr;

    game_board[server_num / 3][server_num % 3] = 1; // Server makes move
    cout << "Server chose: " << server_num << endl;

    // Send message to client
    n = write(sockfd, &server_num, sizeof(server_num));
    assert((n >= 0) && "write() failed");
  }

  return nullptr;
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, port;
  socklen_t clilen;

  struct sockaddr_in server_addr, cli_addr;
  int n;
  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  // Create a TCP socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  assert((sockfd >= 0) && "socket() failed");

  // Initialize server address
  bzero((char *)&server_addr, sizeof(server_addr));
  port = atoi(argv[1]);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind socket to the server address
  n = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  assert((n >= 0) && "bind() failed");

  listen(sockfd, 5); // Listen for connections

  while (1)
  {
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    assert((newsockfd >= 0) && "accept() failed");

    printf("\n New Connection from client %s:%d: \n ",
           inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

    // Create threads for multiple clients
    pthread_t thread;
    int *p_client = new int;
    *p_client = newsockfd;
    pthread_create(&thread, NULL, handle_clients, p_client);
  }
  return 0;
}