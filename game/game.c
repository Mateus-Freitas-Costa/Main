#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "game.h"
#include "../battle/battle.h"
#include "../bot/bot.h"

static void display_HUD(const Map *map, int current_height);
static void display(const Map *map);
static bool destroyed_boat(Map *map, Boat *boat, Point attack);
static Point input_attack_player(char *self_name);
static Point attack_player(Instance *instance, Map *map, Boat *boats);
static Point bot_response(Instance *instance, Map *map, Boat *boats);
static void create_new_instance(Instance *new_instance, Tag tag, void *self, Player enemy,
                                char *name, Point (*f_atx)(Instance *, Map *, Boat *));
static bool round(Instance *instance, Map *map, int boats_count, Boat boats[MAX_PLAYERS][boats_count]);
static void create_instances(Instance *instances, Info info);
static void destroy_instances(Instance *instances, int boats_count, Boat boats[MAX_PLAYERS][boats_count]);

Instance play_game(Info info)
{
    Boat boats[MAX_PLAYERS][info.boats];
    Instance instances[MAX_PLAYERS];
    Map map[MAX_PLAYERS];
    Player i = PLAYER1;

    create_instances(instances, info);
    create_map(info, map, boats);
    display(map);

    for (;;) {
        bool victory = round(&instances[i], map, info.boats, boats);
        if (victory) {
            destroy_instances(instances, info.boats, boats);
            return instances[i];
        }
        i = instances[i].enemy;
    }

}

static void destroy_instances(Instance *instances, int boats_count, Boat boats[MAX_PLAYERS][boats_count])
{
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (instances[i].tag == CPU)
            free(instances[i].self);
        for (int j = 0; j < boats_count; ++j)
            free(boats[i][j].p);
    }
}

static void create_instances(Instance *instances, Info info)
{
    create_new_instance(&instances[PLAYER1], HUMAN, NULL, OPPONENT, info.namep1, attack_player);

    switch (info.mode) {
    case 1:
        create_new_instance(&instances[OPPONENT], CPU, create_bot(info.difficulty), PLAYER1, info.namep2,bot_response);
        break;
    case 2:
        create_new_instance(&instances[OPPONENT], HUMAN, NULL, PLAYER1, info.namep2, attack_player);
        break;
    }
}

static bool round(Instance *instance, Map *map, int boats_count, Boat boats[MAX_PLAYERS][boats_count])
{
    Point attack = instance->attack_func(instance, map, boats[instance->enemy]);
    char *msgs_attacks[MAX_PLAYERS][3] = {{"You attacked ", "You destroyed ", "You Missed"},
                                          {"The Bot attacked your ", "The bot destroyed your ", "The bot Missed"}};

    char msg_attack[25];
    Boat *attacked = get_boat(boats[instance->enemy], attack, boats_count);

    if (attacked == NULL) {
        sprintf(msg_attack, "%s", msgs_attacks[instance->tag][2]);
        map[instance->enemy].sea[attack.y][attack.x] = MISSED;
    } else {
        int index_msg;
        if (destroyed_boat(&map[instance->enemy], attacked, attack)) {
            index_msg = 1;
            update_hud(&map[instance->enemy].HUD, attacked->type, -1);
        } else
            index_msg = 0;
        sprintf(msg_attack, "%s%s", msgs_attacks[instance->tag][index_msg], attacked->name);
    }

    display(map);
    printf("%s\n", msg_attack);

    return map[instance->enemy].remaining_boats == 0;
}

static Point bot_response(Instance *instance, Map *map, Boat *boats)
{
    puts("Bot time\n");
    sleep(1);
    return bot_time(instance->self, map, boats);
}

static void create_new_instance(Instance *new_instance, Tag tag, void *self, Player enemy,
                                char *name, Point (*f_atx)(Instance *, Map *, Boat *))
{
    new_instance->enemy = enemy;
    new_instance->self = self;
    strcpy(new_instance->name, name);
    new_instance->attack_func = f_atx;
    new_instance->tag = tag;
}

static Point attack_player(Instance *instance, Map *map, Boat *boats)
{
    char *msg_invalid[2] = {"Outside of range", "You cannot attack the same point again"};
    Point attack;
    Validity validity;

    do {
        attack = input_attack_player(instance->name);
        validity = check_valid(map[instance->enemy].sea, attack);
        if (validity != VALID) {
            display(map);
            printf("%s\n", msg_invalid[validity]);
        }
    } while (validity != VALID);
    
    return attack;
}

static Point input_attack_player(char *self_name)
{
    char ipt_str[10];
    Point input;

    printf("%s time, enter a coordinate: ", self_name);
    fgets_(ipt_str, 9, stdin);
    sscanf(ipt_str, "%d -%d", &input.x, &input.y);

    return input;
}

static bool destroyed_boat(Map *map, Boat *boat, Point attack)
{
    map->sea[attack.y][attack.x] = DESTROYED;
    if (--boat->lifes == 0) {
        --map->remaining_boats;
        for (int j = 0; j < boat->size; ++j)
            map->sea[boat->p[j].y][boat->p[j].x] = SUNK;
        return true;
    } else
        return false;
}

static void display(const Map *map)
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
                printf("             ");
                printf("%s\n", map[0].info.namep1);
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
                printf("   %s", map[0].info.namep2);
            }
        }
        display_HUD(map, i);
        putchar('\n');
    } 
}

static void display_HUD(const Map *map, int current_height)
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


char *fgets_(char *str, size_t size, FILE *stream)
{
    int ch;
    unsigned i = 0;

    while ((ch = getc(stream)) != '\n' && ch != EOF)
        if (i < size)
            str[i++] = ch;
    if (i == 0 && ch == EOF)
        return NULL;
    str[i] = '\0';

    return str;
}

void clear_screen(void)
{
#ifdef WIN32
    system("cls");
#elif defined __linux__
    system("clear");
#endif
}
