#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "game.h"
#include "../battle/battle.h"
#include "../bot/bot.h"

static void display_HUD(const Map *map, int current_height);
static void display(const Map map[]);
static bool destroyed_boat(Map *map, Boat *boat, Point attack);
static Point input_attack_player(char *self_name);
static Point attack_player(Instance *instance, Map *map, Boat *boats);
static void create_player(Instance *new_instance, Tag tag, char *idenfifier, Player enemy, Winner name);
static Point bot_response(Instance *instance, Map *map, Boat *boats);
static bool round(Instance *instance, Map *map, size_t boats_count, Boat boats[MAX_PLAYERS][boats_count]);
static void create_instances(Instance *instances, Info info);
static void destroy_instances(Instance *instances, int boats_count, Boat boats[MAX_PLAYERS][boats_count]);

Winner play_game(Info info)
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
            return instances[i].name;
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
    create_player(&instances[PLAYER1], HUMAN, "Player one", OPPONENT, PLAYER_ONE);

    switch (info.mode) {
    case 1:
        instances[OPPONENT].tag = CPU;
        instances[OPPONENT].name = BOT;
        instances[OPPONENT].attack_func = bot_response;
        instances[OPPONENT].self = create_bot(info.difficulty);
        instances[OPPONENT].enemy = PLAYER1;
        break;
    case 2:
        create_player(&instances[OPPONENT], HUMAN, "Player Two", PLAYER1, PLAYER_TWO);
        break;
    }
}

static bool round(Instance *instance, Map *map, size_t boats_count, Boat boats[MAX_PLAYERS][boats_count])
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
            update_HUD(&map[instance->enemy].HUD, attacked->type, -1);
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

static void create_player(Instance *new_instance, Tag tag, char *idenfifier, Player enemy, Winner name)
{
    new_instance->enemy = enemy;
    new_instance->self = idenfifier;
    new_instance->name = name;
    new_instance->attack_func = attack_player;
    new_instance->tag = HUMAN;
}

static Point attack_player(Instance *instance, Map *map, Boat *boats)
{
    char *msg_invalid[2] = {"Outside of range", "You cannot attack the same point again"};
    Point attack;
    Validity validity;

    do {
        attack = input_attack_player(instance->self);
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

void clear_screen(void)
{
#ifdef __linux__ 
    system("clear");
#elif defined WIN32
    system("cls");
#endif
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
