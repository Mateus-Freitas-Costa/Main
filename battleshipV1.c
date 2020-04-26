#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define WIDTH              8
#define HEIGHT             8
#define NUMBER_BOATS       7
#define NUMBER_DIRECTIONS  4
#define MAX_PLAYERS        2

typedef enum Types       {DESTROYER, SUBMARINE, CRUISER ,
                          BATTLESHIP, CARRIER}                    Types;
typedef enum Camp        {INACTIVE, ACTIVE, DESTROYED, MISSED}    Camp;
typedef enum Player      {PLAYER1, PLAYER2}                       Player;
typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Hud {
    int cruisers;
    int destroyers;
    int battleships;
    int carriers;
    int submarines;
} Hud;


typedef struct Map {
    Camp  sea[HEIGHT][WIDTH];
    int   remaining_boats;
    Hud   HUD;
} Map;

typedef struct Boat {
    int   size;
    Types  type;
    int   lifes;   
    char *name;
    struct {
        bool  active; 
        Point p;
    } *coordinates;
} Boat;

Boat  *create_boat   (Types type, int size, Camp sea[HEIGHT][WIDTH]);
void   create_map    (Boat *p1[], Boat *p2[], Map map[]);
void   destroy_boats (Boat *p1[], Boat *p2[]);
Player play_game     (Boat *p1[], Boat *p2[], Map map[]);
void   display_result(Player winner);
bool   attack_boat   (Point attack, Map *map, Boat *p[], Player enemy);
bool   match_points  (Point p1, Point p2);
char  *get_name      (Types type);
void   display       (Map map[]);
int    get_boat      (Boat *p[], Point attack);
void   clear_screen  (void);
void   calculate_HUD (Hud *HUD, Types type);
void   display_HUD   (Map *map, int current_height); 
void   update_HUD    (Hud *HUD, Types type);
void   check_quit    (Point attack, Boat *p1[], Boat *p2[]);

int main(void)
{
    srand((unsigned) time(NULL));

    Map map[MAX_PLAYERS];
    Boat *p1[NUMBER_BOATS];
    Boat *p2[NUMBER_BOATS];

    create_map(p1, p2, map);
    Player winner = play_game(p1, p2, map);
    display_result(winner);

    destroy_boats(p1, p2);
}

void create_map(Boat *p1[], Boat *p2[], Map map[])
{
    const Types pattern[NUMBER_BOATS] = {DESTROYER, DESTROYER, SUBMARINE, CRUISER,
                                         BATTLESHIP, SUBMARINE, CARRIER};
    const int pattern_size[5] = {2, 3, 3, 4, 5};

    for (int i = 0; i < MAX_PLAYERS; ++i) {
        map[i].remaining_boats = 0;
        map[i].HUD.battleships = 0;
        map[i].HUD.cruisers = 0;
        map[i].HUD.carriers = 0;
        map[i].HUD.submarines = 0;
        map[i].HUD.destroyers = 0;
        for (int j = 0; j < HEIGHT; ++j) {
            for (int k = 0; k < WIDTH; ++k) {
                map[i].sea[j][k] = INACTIVE;
            } 
        }
    }
    for (int i = 0; i < NUMBER_BOATS; ++i) {
        Types type = pattern[i];
        int size = pattern_size[type];
        while ((p1[i] = create_boat(type, size, map[PLAYER1].sea)) == NULL)
            continue;
        calculate_HUD(&map[PLAYER1].HUD, type);
        ++map[PLAYER1].remaining_boats;

        while ((p2[i] = create_boat(type, size, map[PLAYER2].sea)) == NULL)
            continue;
        calculate_HUD(&map[PLAYER2].HUD, type);
        ++map[PLAYER2].remaining_boats;
    }
}

void calculate_HUD(Hud *HUD, Types type)
{
    switch (type) {
    case DESTROYER:
        ++HUD->destroyers;
        break;
    case SUBMARINE:
        ++HUD->submarines;
        break;
    case CRUISER:
        ++HUD->cruisers;
        break;
    case BATTLESHIP:
        ++HUD->battleships;
        break;
    case CARRIER:
        ++HUD->carriers;
        break;
    }
}


Boat *create_boat(Types type, int size, Camp sea[HEIGHT][WIDTH])
{
    Boat *new = malloc(sizeof(*new));
    if (new == NULL) 
        exit(EXIT_FAILURE);
    new->coordinates = malloc(size * sizeof(*new->coordinates));
    if (new->coordinates == NULL) {
        free(new);
        exit(EXIT_FAILURE);
    }
    enum {NORTH, SOUTH, EAST, WEST} direction = rand() % NUMBER_DIRECTIONS;
    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;
    for (int i = 0; i < size; ++i) {
        if (x >= WIDTH || y >= HEIGHT || x < 0 || y < 0) {
            free(new->coordinates);
            free(new);
            return NULL;
        }
        if (sea[y][x] == ACTIVE) {
            free(new->coordinates);
            free(new);
            return NULL;
        }
        new->coordinates[i].p.x = x;
        new->coordinates[i].p.y = y;
        new->coordinates[i].active = true;
        switch (direction) {
        case NORTH:
            ++y;
            break;
        case EAST:
            ++x;
            break;
        case SOUTH:
            --y;
            break;
        case WEST:
            --x;
            break;
        }
    }
    new->lifes = size;
    new->size  = size;
    new->name  = get_name(type); 
    new->type = type;
    for (int i = 0; i < size; ++i) 
            sea[new->coordinates[i].p.y]
               [new->coordinates[i].p.x] = ACTIVE;
    
    return new;
}

Player play_game(Boat *p1[], Boat *p2[], Map map[])
{
    Point attack;
    int   result;
    display(map);
    for (;;) {
        do {
            puts("\nPlayer one time, enter a coordinate (x-y)");
            scanf("%d -%d", &attack.x, &attack.y);
            check_quit(attack, p1, p2);
            result = attack_boat(attack, map, p2, PLAYER2);
        } while (!result);
        if (map[PLAYER2].remaining_boats == 0) 
            return PLAYER1;

        do {
            puts("\nPlayer two time, enter a coordinate (x-y)");
            scanf("%d -%d", &attack.x, &attack.y);
            check_quit(attack, p1, p2);
            result = attack_boat(attack, map, p1, PLAYER1);
        } while (!result);
        if (map[PLAYER1].remaining_boats == 0) 
            return PLAYER2;
        
    }
}

void check_quit(Point attack, Boat *p1[], Boat *p2[])
{
    if (attack.y == -1 || attack.x == -1) {
        puts("You quit the game");
        destroy_boats(p1, p2);
        exit(EXIT_SUCCESS);
    }
}

bool attack_boat(Point attack, Map *map, Boat *p[], Player enemy)
{
    if (attack.x >= WIDTH || attack.x < 0 || 
    attack.y >= HEIGHT || attack.y < 0) {
        display(map);
        puts("\nOutside of range");
        return false;
    }
    if (map[enemy].sea[attack.y][attack.x] == DESTROYED || 
    map[enemy].sea[attack.y][attack.x] == MISSED) {
        display(map);
        puts("\nYou cannot attack the same point again");
        return false;
    } 
    if (map[enemy].sea[attack.y][attack.x] == INACTIVE) {
        map[enemy].sea[attack.y][attack.x] = MISSED;
        display(map);
        puts("\nYou missed");
        return true;
    }
    
    int attacked = get_boat(p, attack);
    
    --p[attacked]->lifes;
    map[enemy].sea[attack.y][attack.x] = DESTROYED;
    if (p[attacked]->lifes == 0) {
        update_HUD(&map[enemy].HUD, p[attacked]->type);
        display(map);
        printf("\nYou destroyed %s\n", p[attacked]->name);
        --map[enemy].remaining_boats;
    } else {
        display(map);
        printf("\nYou attacked %s\n", p[attacked]->name);
    } 
    return true;
}

void update_HUD(Hud *HUD, Types type)
{
    switch (type) {
    case DESTROYER:
        --HUD->destroyers;
        break;
    case SUBMARINE:
        --HUD->submarines;
        break;
    case CRUISER:
        --HUD->cruisers;
        break;
    case CARRIER:
        --HUD->carriers;
        break;
    case BATTLESHIP:
        --HUD->battleships;
        break;
    }
}

int get_boat(Boat *p[], Point attack)
{
     for (int i = 0; i < NUMBER_BOATS; ++i) 
        for (int j = 0; j < p[i]->size; ++j) 
            if (match_points(attack, p[i]->coordinates[j].p)) { 
                p[i]->coordinates[j].active = false;   
                return i;
            }
}

bool match_points(Point p1, Point p2)
{
    return p1.x == p2.x && p1.y == p2.y;
}

void display_result(Player winner)
{ 
    if (winner == PLAYER1) 
        puts("Player 1 wins");
    else 
        puts("Player 2 wins"); 
}

void destroy_boats(Boat *p1[], Boat *p2[])
{
    for (int i = 0; i < NUMBER_BOATS; ++i) {
        if (p1[i] != NULL) {
            if (p1[i]->coordinates != NULL)
               free(p1[i]->coordinates);
            free(p1[i]);
        }
        if (p2[i] != NULL) {
            if (p2[i]->coordinates != NULL)
               free(p2[i]->coordinates);
            free(p2[i]);
        }
    }
}

char  *get_name(Types type)
{ 
    switch (type) {
    case DESTROYER:
        return "Destroyer";
        break;
    case SUBMARINE:
        return "Submarine";
        break;
    case CARRIER:  
        return "Carrier";
        break;
    case BATTLESHIP:
        return "Battleship";
        break;
    case CRUISER:
        return "Cruiser";
        break;
    } 
}

void display(Map map[])
{ 
    clear_screen();
    puts("--------------------------------------------------------------------------");
    puts("                                Battleship Game                            ");
    puts("--------------------------------------------------------------------------");
    
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < MAX_PLAYERS; ++j) {
            if (i == 0 && j == PLAYER1) {
                printf("\n    0  1  2  3  4  5  6  7");
                printf("        ");
                printf("    0  1  2  3  4  5  6  7\n");
                printf("    _  _  _  _  _  _  _  _");
                printf("        ");
                printf("    _  _  _  _  _  _  _  _");
                printf("                                   ");
                printf("          Player One\n");
            }
            for (int k = 0; k < WIDTH; ++k) {
                if (k == 0)
                    printf("%d-|", i);
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
                }
            }
            printf("       ");
            if (i == 4 && j == PLAYER2)
                printf("                        "
                        "             Player two");
        }
        display_HUD(map, i);
        putchar('\n');
    } 
}

void display_HUD(Map *map, int current_height)
{
    Player current_player;
    if (current_height < NUMBER_BOATS / MAX_PLAYERS)
        current_player = PLAYER1;
    else 
        current_player = PLAYER2;

    printf("                              ");
    switch (current_height % 5) {
    case 0:
        printf("Destroyers: %d  |  Cruisers: %d", map[current_player].HUD.destroyers, 
                map[current_player].HUD.cruisers);
        break;
    case 1:
        printf("Submarines: %d  |  Battleships: %d ", map[current_player].HUD.submarines,
                map[current_player].HUD.battleships);
        break;
    case 2:
        printf("Carriers: %d    |", map[current_player].HUD.carriers);
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
