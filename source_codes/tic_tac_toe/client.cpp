#include <argp.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using namespace std;

vector<vector<int>> game_board{
    {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}}; // game board

/**
 * @brief Function to get the corresponding character for given integer
 * @param n integer to be converted
 * @return character corresponding to the integer
 */
char get_char(int n)
{
  switch (n)
  {
  case -1:
    return ' ';
    break;
  case 0:
    return 'O';
    break;
  case 1:
    return 'X';
    break;

  default:
    break;
  }
  return ' ';
}

/**
 * @brief Function to print the game board
 */
void board()
{
  cout << endl;
  cout << " " << get_char(game_board[0][0]) << " "
       << "|"
       << " " << get_char(game_board[0][1]) << " "
       << "|"
       << " " << get_char(game_board[0][2]) << " " << endl;
  cout << "---|---|---" << endl;
  cout << " " << get_char(game_board[1][0]) << " "
       << "|"
       << " " << get_char(game_board[1][1]) << " "
       << "|"
       << " " << get_char(game_board[1][2]) << " " << endl;
  cout << "---|---|---" << endl;
  cout << " " << get_char(game_board[2][0]) << " "
       << "|"
       << " " << get_char(game_board[2][1]) << " "
       << "|"
       << " " << get_char(game_board[2][2]) << " " << endl;
  cout << endl;
}

/**
 * @brief Function to check if the game is over
 * @return true if the game is over, false otherwise
 */
bool check_win()
{
  pair<bool, int> ans(false, -1);
  // Rows
  for (int i = 0; i < 3; i++)
    if (game_board[i][0] == game_board[i][1] &&
        game_board[i][0] == game_board[i][2] && game_board[i][0] != -1)
      ans = make_pair(true, game_board[i][0]);

  // Columns
  for (int j = 0; j < 3; j++)
    if (game_board[0][j] == game_board[1][j] &&
        game_board[0][j] == game_board[2][j] && game_board[0][j] != -1)
      ans = make_pair(true, game_board[0][j]);

  // Diagnols
  if (game_board[0][0] == game_board[1][1] &&
      game_board[1][1] == game_board[2][2] && game_board[0][0] != -1)
    ans = make_pair(true, game_board[0][0]);

  if (game_board[0][2] == game_board[1][1] &&
      game_board[1][1] == game_board[2][0] && game_board[0][2] != -1)
    ans = make_pair(true, game_board[0][2]);

  if (ans.first == true)
  {
    string player = ans.second == 0 ? "Client" : "Server";
    cout << player << " won!" << endl;
    return true;
  }
  return false;
}

int main(int argc, char *argv[])
{
  int sockfd, port, n;

  struct sockaddr_in server_addr;
  struct hostent *server;

  if (argc < 3)
  {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }

  port = atoi(argv[2]); // Get port number

  // Create a TCP socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  assert((sockfd >= 0) && "socket() failed");
  server = gethostbyname(argv[1]);
  assert((server != NULL) && "gethostbyname() failed");

  // Initialize server address
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,
        server->h_length);
  server_addr.sin_port = htons(port);

  // Connect to the server
  n = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  assert((n >= 0) && "connect() failed");

  int num = -1;
  int free_cells = 9;
  vector<bool> chosen(9, false); // vector to keep track of chosen cells

  while (1)
  {
    board();

    cout << "Enter number: " << endl;
    cin >> num;

    while (num < 0 || num > 8 || chosen[num] == true)
    {
      cout << "Invalid number!" << endl;
      cin >> num;
    }

    chosen[num] = true;
    game_board[num / 3][num % 3] = 0; // Update game board for client's move
    free_cells--;                     // Decrement free cells

    // Check who won
    if (check_win())
    {
      board();
      break;
    }

    // If free cells are zero and no one has won
    // it means that the game is draw
    if (free_cells == 0)
    {
      cout << "Draw!" << endl;
      break;
    }

    // Send the number to the server
    n = write(sockfd, &num, sizeof(num));
    assert((n >= 0) && "write() failed");

    // Receive the number from the server
    n = read(sockfd, &num, sizeof(num));
    assert((n >= 0) && "read() failed");

    if (num < 0 || num > 8)
      break;

    chosen[num] = true;
    game_board[num / 3][num % 3] = 1; // Update game board for server's move
    free_cells--;                     // Decrement free cells

    // Check who won
    if (check_win())
    {
      board();
      break;
    }
    if (free_cells == 0)
    {
      cout << "Draw!" << endl;
      break;
    }
  }

  return 0;
}