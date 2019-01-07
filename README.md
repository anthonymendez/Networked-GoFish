# Networked GoFish
## Assignment 5

## Authors
* Anthony Mendez - anthonymende@umass.edu
* Joshua Howell - jhowell@umass.edu

## Summary
The GoFish code interacts with the server to keep the player updated, and handle input coming from the player. The flow of the code is largly the same as in the original GoFish assignment. There is added overhead to establish the network connection and transmit data during the game. Additionally, the server blocks while waiting for input from the network instead of the terminal. The client opens the connection to the server, prints data received from the server, and sends user input when prompted. The server drives the GoFish code by opening the connection to the client, sending game updates to the client, and passing data from the client to the GoFish code.

## Solution Approach
First, we added GoFish files to compile with server files in Makefile. We then converted print statements into calls to a function to send data to the client. We adopted the strategy of performing all logic/processing on the server and simply displaying results and prompts to the client. Therefore, the GoFish code is almost exclusively running on the server. The client gets updated with the current game state and responds to prompts (card choice and whether to play another game or not).

## Problems Encountered
1. Prompting the player for input did not work at first.
2. The program would get stuck after the first iteration of the game.
3. The printout of the cards in the player's hand did not separate each card.

## Solutions
1. The client now checks received data for a special character. When receiving this character, the client will wait for user input and send it back to the server (instead of printing the special character).
2. The code for the prompt at the end of the game was largly rewritten, similar to the prompt at the beginning of each turn.
3. We reworked the server code to include a space separator when concatenating cards in the player's hand to send.
