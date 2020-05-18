#ifndef BATTLE_H
#define BATTLE_H

#include "../main/extra.h"
#include "../game/game.h"

#include <stdbool.h>
#include <stdlib.h>

#define WIDTH              8
#define HEIGHT             8
#define MAX_BOATS          8
#define NUMBER_DIRECTIONS  4
#define MAX_PLAYERS        2

typedef enum  {DESTROYER, SUBMARINE, CRUISER ,
               BATTLESHIP, CARRIER}                    Types;
typedef enum  {INACTIVE, ACTIVE, DESTROYED, MISSED,
               SUNK}                                   Camp;
typedef enum  {PLAYER1, OPPONENT}                      Player;
typedef enum  {OUTRANGE, REPEATED, VALID}              Validity;
typedef enum {HUMAN, CPU} Tag;

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    unsigned int cruisers;
    unsigned int destroyers;
    unsigned int battleships;
    unsigned int carriers;
    unsigned int submarines;
} Hud;

typedef struct {
    Camp           sea[HEIGHT][WIDTH];
    unsigned int   remaining_boats;
    Hud            HUD;
    Info           info;
} Map;

typedef struct {
    size_t          size;
    Types           type;
    unsigned int    lifes;   
    const char     *name;
    Point          *p;
} Boat;

typedef struct Instance {
    Winner name;
    Tag tag;
    Player enemy;
    void *self;
    Point (*attack_func)(struct Instance *, Map *, Boat *);
} Instance;

void     update_HUD              (Hud *HUD, Types type, int change);
Boat      *get_boat(Boat *boats, Point attack, size_t boats_count);
void     create_map(Info info, Map *map, Boat boats[MAX_PLAYERS][info.boats]);
Validity check_valid(Camp sea[HEIGHT][WIDTH], Point attack);

#endif /* BATTLE_H */
