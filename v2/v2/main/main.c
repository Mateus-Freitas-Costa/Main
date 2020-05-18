#include <stdio.h>

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "../game/game.h"
#include "../battle/battle.h"

void display_winner(Instance winner, Info info);

int main(void)
{
#ifdef  __linux__
    system("clear");
#elif define WIN32
    system("cls");
#endif

    Info info;
    char input[4];

    puts("This is a battleship game");

    printf("Enter the number of boats you wanna to play with (3-8): ");
    fgets_(input, 3, stdin);
    sscanf(input, "%d", &info.boats);

    if (info.boats > 8 || info.boats < 3) {
        puts("Invalid number of boats");
        return 1;
    }

    printf("Enter the mode, 1 to play against the machine, 2 to play with another player: ");
    fgets_(input, 3, stdin);
    sscanf(input, "%d", &info.mode);

    printf("Enter name for the first player: ");
    fgets_(info.namep1, NAME_LEN, stdin);

    if (info.mode == 1) {
        printf("Enter a difficulty (0-6):\n");
        puts("0 - Very easy");
        puts("1 - Easy");
        puts("2 - Medium");
        puts("3 - Hard");
        puts("4 - Very hard");
        puts("5 - Insane");
        puts("6 - Impossible");

	    fgets_(input, 3, stdin);
        sscanf(input, "%d", &info.difficulty);
        if (info.difficulty < 0 || info.difficulty > 6) {
            puts("invalid difficulty");
            return 2;
        }
        strcpy(info.namep2, "BOT");
    } else {
        printf("Enter name for the second player :");
        fgets_(info.namep2, NAME_LEN, stdin);
    }
    
    Instance winner;

    srand((unsigned) time(NULL));
    switch (info.mode) {
    case 1:
    case 2:
        winner = play_game(info); // alterar aqui é claro
        break;
    default:
        puts("Invalid mode");
        return 3;
    }
    
    display_winner(winner, info);
    return 0;
}

void display_winner(Instance winner, Info info)
{
    int chance;

    switch (winner.tag) {
    case HUMAN:
        printf("%s wins\n", winner.name);
        if (info.mode == 1) {
            switch (info.difficulty) {
            case 0:
                puts("You cannot defeat the masters, just the very easy one");
                break;
            case 1: 
                puts("So you think you are good enough? Try again");
                break;
            case 2: 
                puts("Oh noooooo...");
                break;
            case 3:
                puts("I will have my revenge");
                break;
            case 4:
                puts("I will never forgive you");
                break;
            case 5:
                puts("This is not possible, how can I have loose for a simple man?"
                      "the MASTER you obliterate you");
                break;
            case 6:
                puts("Some bug has ocurred\a");
                break;
            }
        }
        break;
    case CPU:
        chance = rand() % 5;
        if (info.difficulty < 6) {
            switch (chance) {
            case 0:
                puts("BOT: You cannot beat me");
                break;
            case 1:
                puts("BOT: I am the winner again");
                break;
            case 2:
                puts("BOT: You are a great noob");
                break;
            case 3:
                puts("BOT: I am the best player in the world");
                break;
            case 4:
                puts("BOT: The bots of Rodguard will never lose");
                break;
            } 
        } else 
            puts("You thought I was joking? It's impossible defeat me");
        break;
    }
}
