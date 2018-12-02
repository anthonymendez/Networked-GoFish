#include <stdio.h>
#include <time.h>
#include "gofish.h"

/*
 * Function: game_start
 * --------------------
 * Called at the start of a new game.
 * Shuffles the deck, resets instances of players, 
 * deals cards to each player, and sets the human player 
 * as the current player.
 */
void game_start(int connfd) {
    reset_player(&user);
    reset_player(&computer);

    sendStringToClient(connfd, "Shuffling deck...\n");
    shuffle();
    deal_player_cards(&user);
    deal_player_cards(&computer);

    /* It is possible to be dealt, at most, one book */
    if(user.book[0] != 0) {
        char *buf = malloc(100 * sizeof(char));
        sprintf(buf, "Player 1 books %s", pR(user.book[0]));
        sendStringToClient(connfd, buf);
        free(buf);
    }
    if(computer.book[0] != 0) {
        char *buf = malloc(100 * sizeof(char));
        sprintf(buf, "Player 2 books %s", pR(computer.book[0]));
        sendStringToClient(connfd, buf);
        free(buf);
    }

    current = &user;
    next_player = &computer;
}

/*
 * Function: game_loop
 * -------------------
 * Called after game_start.
 * Handles each round of game play.
 * 
 * Return: 1 if there is a winner, 0 otherwise
 */
int game_loop(int connfd, rio_t rio) {
    sendStringToClient(connfd, "\n");

    /* Print hand and book statuses */
    sendStringToClient(connfd, "Player 1's Hand - ");
    print_hand(connfd, &user);
    sendStringToClient(connfd, "Player 1's Book - ");
    print_book(connfd, &user);
    sendStringToClient(connfd, "Player 2's Book - ");
    print_book(connfd, &computer);

    struct player* other_player = (current == &user) ? &computer : &user;

    if(game_over(current)) {
        return 1;
    }
    if(game_over(other_player)) { /* Shouldn't happen */
        current = other_player; /* Signify the correct winner */
        return 1;
    }

    /*
    "If a player runs out of cards, then they have to draw a card on their next turn.
    It does not end the game." -Irwin (piazza)
    */
    char r;
    if(current->hand_size > 0) { /* Non-empty hand */
        /* Get rank guess input */
        if(current == &user) { /* User's turn */
            r = user_play(connfd, rio, current);
        } else { /* Computer's turn */
            r = computer_play(connfd, current);
        }
    } else /* Empty hand */
        r = 'X'; /* Invalid rank, so search will always fail and the player will Go Fish */

    /* Handle input */
    if(search(other_player, r) == 0) { /* Go Fish */
        if(r != 'X') { /* Non-empty hand */
            char *buf = malloc(100 * sizeof(char));
            sprintf(buf, "    - Player %d has no %s's\n", ((current == &user) ? 2 : 1), pR(r));
            sendStringToClient(connfd, buf);
            free(buf);
        }
        struct card* fished_card = next_card();
        int next_book_i = 0;
        if (fished_card != NULL) {
            if(current == &user) {
                char *buf = malloc(100 * sizeof(char));
                sprintf(buf, "    - Go Fish, Player 1 draws %s%c\n", pR(fished_card->rank), fished_card->suit);
                sendStringToClient(connfd, buf);
                free(buf);
            } else {
                if (fished_card->rank == r) {
                    char *buf = malloc(100 * sizeof(char));
                    sprintf(buf, "    - Go Fish, Player 2 draws %s%c\n", pR(fished_card->rank), fished_card->suit);
                    sendStringToClient(connfd, buf);
                    free(buf);
                }
                else {
                    char *buf = malloc(100 * sizeof(char));
                    sprintf(buf, "    - Go Fish, Player 2 draws a card\n");
                    sendStringToClient(connfd, buf);
                    free(buf);
                }
            }

           while(current->book[next_book_i] != 0) {
                next_book_i++;
            }
            add_card(current, fished_card);
            if(current->book[next_book_i] != 0) {
                char *buf = malloc(100 * sizeof(char));
                sprintf(buf, "    - Player %d books %s\n", ((current == &user) ? 1 : 2), pR(fished_card->rank));
                sendStringToClient(connfd, buf);
                free(buf);
            }
        }
        /* If a book was added or the asked rank was drawn, play again */
        if(fished_card != NULL && (
            current->book[next_book_i] != 0 || 
            fished_card->rank == r)
          ) {
            next_player = current;
            char *buf = malloc(100 * sizeof(char));
            sprintf(buf, "    - Player %d gets another turn\n", ((current == &user) ? 1 : 2));
            sendStringToClient(connfd, buf);
            free(buf);
        } else { /* Otherwise, switch players' turns */
            next_player = other_player;
            char *buf = malloc(100 * sizeof(char));
            sprintf(buf, "    - Player %d's turn\n", ((next_player == &user) ? 1 : 2));
            sendStringToClient(connfd, buf);
            free(buf);
        }
    } else { /* Transfer cards, play again */
        /* Print the cards of the guessed rank that each player has */
        struct player* print_player = current;
        int i;
        for(i = 0; i < 2; i++) {
            if (current == &computer && print_player == &computer) {
                print_player = (current == &user) ? &computer : &user; /* Switch to other player */
                continue;
            }
            char *buf = malloc(100 * sizeof(char));
            sprintf(buf, "    - Player %d has ", ((print_player == &user) ? 1 : 2));
            sendStringToClient(connfd, buf);
            free(buf);
            int j;
            struct hand* h = print_player->card_list;
            int rank_count = 0;
            for(j = 0; j < print_player->hand_size && h != NULL; j++) {
                if(h->top.rank == r) {
                    if(rank_count++ > 0) {
                        buf = malloc(100 * sizeof(char));
                        sprintf(buf, ", ");
                        sendStringToClient(connfd, buf);
                        free(buf);
                    }
                    buf = malloc(100 * sizeof(char));
                    sprintf(buf, "%s%c", pR(r), h->top.suit);
                    sendStringToClient(connfd, buf);
                    free(buf);
                }

                h = h->next;
            }
            buf = malloc(100 * sizeof(char));
            sprintf(buf, "\n");
            sendStringToClient(connfd, buf);
            free(buf);
            print_player = (current == &user) ? &computer : &user; /* Switch to other player */
        }

        int next_book_i = 0;
        while(current->book[next_book_i] != 0) {
            next_book_i++;
        }
        transfer_cards(other_player, current, r);
        if(current->book[next_book_i] != 0) {
            char *buf = malloc(100 * sizeof(char));
            sprintf(buf, "    - Player %d books %s\n", ((current == &user) ? 1 : 2), pR(r));
            sendStringToClient(connfd, buf);
            free(buf);
        }

        /* If a book was added, play again */
        if(current->book[next_book_i] != 0) {
            next_player = current;
            char *buf = malloc(100 * sizeof(char));
            sprintf(buf, "    - Player %d gets another turn\n", ((current == &user) ? 1 : 2));
            sendStringToClient(connfd, buf);
            free(buf);
        } else { /* Otherwise, switch players' turns */
            next_player = other_player;
            char *buf = malloc(100 * sizeof(char));
            sprintf(buf, "    - Player %d's turn\n", ((next_player == &user) ? 1 : 2));
            sendStringToClient(connfd, buf);
            free(buf);
        }
    }
    return 0;
}

/*
 * Function: game_end
 * ------------------
 * Called after someone wins in 
 * GoFish from game_loop.
 * Declares the winner and asks the human 
 * if s/he wants to play again.
 * If Y is entered, go to game_start.
 * Else if N is entered, end game and close program.
 * 
 * Return: 1 to play again, 0 to exit
 */
int game_end(int connfd, rio_t rio) {
    struct player* other_player = (current == &user) ? &computer : &user;
    
    /* Count books of loser */
    int count = 0;
    while(other_player->book[count] != 0 && count < 6) { /* Last slot (index 6) will always be empty since they lost */
        count++;
    }
    if(current == &user) { /* User won :D */
        char *buf = malloc(100 * sizeof(char));
        sprintf(buf, "Player 1 Wins! 7-%d\n", count);
        sendStringToClient(connfd, buf);
        free(buf);
    } else {
        char *buf = malloc(100 * sizeof(char));
        sprintf(buf, "Player 2 Wins! 7-%d\n", count);
        sendStringToClient(connfd, buf);
        free(buf);
    }

    char yn[3];
    int tryAgain = 0;
    do {
        if(tryAgain) {
            char *buf = malloc(100 * sizeof(char));
            sprintf(buf, "Error - must enter \"Y\" or \"N\"");
            sendStringToClient(connfd, buf);
            free(buf);
        }
        char *buf = malloc(100 * sizeof(char));
        sprintf(buf, "\nDo you want to play again [Y/N]: ");
        sendStringToClient(connfd, buf);
        free(buf);
        /* TODO: Replace with wait to listen from client */
        sendStringToClient(connfd, DO_NOT_PRINT);
        Rio_readlineb(&rio, yn, 3);
        tryAgain = 1;

        if(yn[1] != '\0')
            continue;

        if(yn[0] == 'Y')
            return 1;
        else if(yn[0] == 'N')
            return 0;
        else
            continue;
    }while(1);
}

/*
 * Function: pR
 * ------------
 * The name is short for printableRank.
 * Formats a rank for output.
 * Specifically, 'T' returns "10" 
 * and all other input is "unchanged".
 * 
 * r: the rank to format
 * 
 * Return: String representing rank r
 */
const char* pR(char r) {
    if(r == 'T') {
        static char ten[] = "10";
        return ten;
    }
    static char rS[2];
    rS[0] = r;
    rS[1] = '\0';
    return rS;
}

/*
 * Function: print_hand
 * --------------------
 * Prints space-separated 2-character 
 * representations of the cards in 
 * player target's hand followed by a 
 * newline.
 * The rank character precedes the 
 * suit character.
 * 
 * target: the player to print the hand of
 */
void print_hand(int connfd, struct player* target) {
    if(target->hand_size == 0) {
        char *buf = malloc(100 * sizeof(char));
        sprintf(buf, "\n");
        sendStringToClient(connfd, buf);
        free(buf);
        return;
    }

    struct hand* h = target->card_list;
    char *buf = malloc(100 * sizeof(char));
    sprintf(buf, "%s%c", pR(h->top.rank), h->top.suit);
    sendStringToClient(connfd, buf);
    free(buf);

    int i;
    for(i = 1; i < target->hand_size; i++) {
        h = h->next;
        buf = malloc(100 * sizeof(char));
        sprintf(buf, "%s%c", pR(h->top.rank), h->top.suit);
        sendStringToClient(connfd, buf);
        free(buf);
    }

    buf = malloc(100 * sizeof(char));
    sprintf(buf, "\n");
    sendStringToClient(connfd, buf);
    free(buf);
}

/*
 * Function: print_book
 * --------------------
 * Prints space-separated 
 * representations of the ranks of the 
 * books in player target's hand 
 * followed by a newline.
 * 
 * target: the player to print the book of
 */
void print_book(int connfd, struct player* target) {
    if(target == NULL || target->book == NULL || target->book[0] == '\0' || target->book[0] == 0) {
        char *buf = malloc(100 * sizeof(char));
        sprintf(buf, "\n");
        sendStringToClient(connfd, buf);
        free(buf);
        return;
    }

    char *buf = malloc(100 * sizeof(char));
    sprintf(buf, "%s", pR(target->book[0]));
    sendStringToClient(connfd, buf);
    free(buf);

    int i = 1;
    while(i < 7 && target->book[i] != '\0' && target->book[i] != 0) {
        buf = malloc(100 * sizeof(char));
        sprintf(buf, " %s", pR(target->book[i++]));
        sendStringToClient(connfd, buf);
        free(buf);
    }

    buf = malloc(100 * sizeof(char));
    sprintf(buf, "\n");
    sendStringToClient(connfd, buf);
    free(buf);
}
