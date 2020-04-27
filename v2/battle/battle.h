#ifndef BATTLE_H
#define BATTLE_H

#include "../main/extra.h"

#include <stdbool.h>

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
    char           *name;
    struct {
        bool  active; 
        Point p;
    } *coordinates;
} Boat;


void     create_map              (Boat *p1[], Boat *p2[], Map map[], const Info info);
Boat    *get_boat                (Boat *p[], const Point attack, size_t boats);
int      get_coordinate_location (const Boat *p, const Point attack);
Validity check_valid             (const Point attack, const Camp sea[][WIDTH]);
void     update_HUD              (Hud *HUD, const Types type, int change);

#endif /* BATTLE_H */