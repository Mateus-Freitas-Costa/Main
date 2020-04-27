#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "game.h"

void display_winner(Winner winner);

int main(void)
{
    Info info;
    puts("This is a battleship game");
    printf("Enter the number of boats you wanna to play with (3-8): ");
    scanf("%d", &info.boats);
    if (info.boats > 8 || info.boats < 3) {
        puts("Invalid number of boats");
        return 1;
    }
    printf("Enter the mode, 1 to play against the machine, 2 to play with another player: ");
    scanf("%d", &info.mode);

    if (info.mode == 1) {
        printf("Enter a difficulty (0-5): ");
        scanf("%d", &info.difficulty);
        if (info.difficulty < 0 || info.difficulty > 5) {
            puts("invalid difficulty");
            return 2;
        }
    } 
    
    Winner winner;
    srand((unsigned) time(NULL));
    switch (info.mode) {
    case 1:
        winner = bot_game(info);
        break;
    case 2:
        winner = two_players_game(info);
        break;
    default:
        puts("Invalid mode");
        return 3;
    }
    
    display_winner(winner);
    return 0;
}

void display_winner(Winner winner)
{
    switch (winner) {
    case PLAYER_ONE:
        puts("Player one wins");
        break;
    case PLAYER_TWO:
        puts("Player two wins");
        break;
    case BOT:
        puts("BOT wins");
        break;
    }
}
