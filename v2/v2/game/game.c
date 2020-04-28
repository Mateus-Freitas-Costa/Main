#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "game.h"
#include "../battle/battle.h"
#include "../bot/bot.h"


static bool destroyed_boat   (Map *map, Boat *p, Point attack);
static void display          (const Map map[]);
static void display_HUD      (const Map map[], const int current_height);
static bool check_quit       (const Point input);
static bool response         (Point attack, Map map[], Boat *p[], const Player enemy);
static void bot_response     (Map map[], const Point attack, Boat *p[]);
static void destroy_boats    (Boat *p1[], Boat *p2[], size_t boats);


Winner bot_game(const Info info)
{
    // Those pointers are the most stupid thing you ever have seen
    Boat *p1[info.boats];
    Boat *p2[info.boats];
    Map   map[MAX_PLAYERS];
    AI    bot;
    create_map(p1, p2, map, info);
    create_bot(&bot, info.difficulty);
    display(map);
    Point attack;
    for (;;) {
        printf("Player one time, enter a coordinate (x-y): ");
        scanf("%d -%d", &attack.x, &attack.y);
        if (check_quit(attack)) {
            puts("You quit the game");
            destroy_boats(p1, p2, info.boats);
            exit(EXIT_SUCCESS);
        }
        if (!response(attack, map, p2, OPPONENT)) 
            continue;
        if (map[OPPONENT].remaining_boats == 0)
            return PLAYER_ONE;

        puts("BOT time");
        sleep(1);
        attack = bot_time(&bot, &map[PLAYER1], p1);
        bot_response(map, attack, p1);
        if (map[PLAYER1].remaining_boats == 0)
            return BOT;
    }
    

    destroy_boats(p1, p2, info.boats);
}


Winner two_players_game(const Info info)
{
    Boat *p1[info.boats];
    Boat *p2[info.boats];
    Map map[MAX_PLAYERS];

    create_map(p1, p2, map, info);
    display(map);
    Point attack;
    for (;;) {
        for (;;) {
            printf("Player one time, enter a coordinate (x-y): ");
            scanf("%d -%d", &attack.x, &attack.y);
            if (check_quit(attack)) {
                puts("You quit the game");
                destroy_boats(p1, p2, info.boats);
                exit(EXIT_SUCCESS);
            }
            if (!response(attack, map, p2, OPPONENT)) 
                continue;
            if (map[OPPONENT].remaining_boats == 0)
                return PLAYER_ONE;
            break;
        }
        for (;;) {
            printf("Player two time, enter a coordinate (x-y): ");
            scanf("%d -%d", &attack.x, &attack.y);
            if (check_quit(attack)) {
                puts("You quit the game");
                destroy_boats(p1, p2, info.boats);
                exit(EXIT_SUCCESS);
            }
            if (!response(attack, map, p1, PLAYER1)) 
                continue;
            if (map[PLAYER1].remaining_boats == 0)
                return PLAYER_TWO;
            break;
        }
    }
    destroy_boats(p1, p2, info.boats);
}

static bool response(Point attack, Map map[], Boat *p[], const Player enemy)
{
    Validity result;
    if ((result = check_valid(attack, map[enemy].sea)) == OUTRANGE) {
        display(map);
        puts("Outside of Range");
        return false;
    } 
    if (result == REPEATED) {
        display(map);
        puts("You cannot attack the same point again");
        return false;
    }
    if (map[enemy].sea[attack.y][attack.x] == INACTIVE) {
        map[enemy].sea[attack.y][attack.x] = MISSED;
        display(map);
        puts("You missed");
        return true;
    }
    Boat *attacked = get_boat(p, attack, map->info.boats);
    int index = get_coordinate_location(attacked, attack);
    attacked->coordinates[index].active = false;
    if (destroyed_boat(&map[enemy], attacked, attack)) {
        update_HUD(&map[enemy].HUD, attacked->type, -1);
        display(map);
        printf("You destroyed %s\n", attacked->name);
    } else {
        display(map);
        printf("You attacked %s\n", attacked->name);
    }
    return true;
}

static void bot_response(Map map[], const Point attack, Boat *p[])
{
    if (map[PLAYER1].sea[attack.y][attack.x] == INACTIVE) {
        map[PLAYER1].sea[attack.y][attack.x] = MISSED;
        display(map);
        puts("The BOT missed");
        return;
    }
    Boat *attacked = get_boat(p, attack, map[PLAYER1].info.boats);
    int index = get_coordinate_location(attacked, attack);
    attacked->coordinates[index].active = false;
    if (destroyed_boat(&map[PLAYER1], attacked, attack)) {
        update_HUD(&map[PLAYER1].HUD, attacked->type, -1);
        display(map);
        printf("The BOT destroyed your %s\n", attacked->name);
    } else {
        display(map);
        printf("The BOT attacked your %s\n", attacked->name);
    }
}

static bool destroyed_boat(Map *map, Boat *p, Point attack)
{
    --p->lifes;
    map->sea[attack.y][attack.x] = DESTROYED;
    if (p->lifes == 0) {
        --map->remaining_boats;
        for (int i = 0; i < p->size; ++i)   
            map->sea[p->coordinates[i].p.y]
                    [p->coordinates[i].p.x] = SUNK;
        return true;
    } 
    return false;
}

static void destroy_boats(Boat *p1[], Boat *p2[], const size_t boats)
{
    for (int i = 0; i < boats; ++i) {
        if (p1[i] != NULL) {
            free(p1[i]->coordinates);
            free(p1[i]);
        }
        if (p2[i] != NULL) {
            free(p2[i]->coordinates);
            free(p2[i]);
        }
    }
}



static bool check_quit(const Point input)
{
    return input.y == -1 || input.x == -1;
}

static void display(const Map map[])
{ 
    clear_screen();
    puts("--------------------------------------------------------------------------");
    puts("                                Battleship Game                           ");
    puts("--------------------------------------------------------------------------");
    
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < MAX_PLAYERS; ++j) {
            if (i == 0 && j == PLAYER1) {
                printf("\n     ");
                printf("0  1  2  3  4  5  6  7");
                printf("             ");
                printf("0  1  2  3  4  5  6  7");
                printf("\n     ");
                printf("_  _  _  _  _  _  _  _");
                printf("             ");
                printf("_  _  _  _  _  _  _  _");
                printf("                                   ");
                printf("          ");
                printf("Player One\n");
            }
            for (int k = 0; k < WIDTH; ++k) {
                if (k == 0)
                    printf("%-2d-|", i);
                switch (map[j].sea[i][k]) {
                case INACTIVE:
                case ACTIVE:
                    printf("   ");
                    break;
                case DESTROYED:
                    printf(" X ");
                    break;
                case MISSED:
                    printf(" * ");
                    break;
                case SUNK:
                    printf(" # ");
                    break;
                }
            }
            printf("       ");
            if (i == 4 && j == OPPONENT) {
                printf("                   "
                        "                  ");
                if (map[0].info.mode == 1)
                    printf("    BOT");
                else 
                    printf("Player two");
            }
        }
        display_HUD(map, i);
        putchar('\n');
    } 
}

static void display_HUD(const Map *map, const int current_height)
{
    Player current_player;
    if (current_height < HEIGHT / MAX_PLAYERS)
        current_player = PLAYER1;
    else 
        current_player = OPPONENT;

    printf("                              ");
    switch (current_height % 5) {
    case 0:
        printf("Destroyers: %u  |  Cruisers: %u", map[current_player].HUD.destroyers, 
                map[current_player].HUD.cruisers);
        break;
    case 1:
        printf("Submarines: %u  |  Battleships: %u ", map[current_player].HUD.submarines,
                map[current_player].HUD.battleships);
        break;
    case 2:
        printf("Carriers: %u    |", map[current_player].HUD.carriers);
        break;
    }
}

void clear_screen(void)
{
    #ifdef __linux__ 
    system("clear");
    #elif defined WIN32
    system("cls");
    #endif
}
