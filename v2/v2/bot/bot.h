#ifndef BOT_H
#define BOT_H

#define BUFF_SIZE (NUMBER_DIRECTIONS - 1)
#define MAX_TRIES  20

#include "../battle/battle.h"

typedef enum {UP, RIGHT, DOWN, LEFT} Arrow;

typedef struct {
    Point          previous[BUFF_SIZE];
    unsigned int   count;
} Buff;

typedef struct {
    bool up;
    bool down;      
    bool left;
    bool right;
} Directions;

typedef struct {
    int            level;
    Buff           buff;
    struct {
        bool           searching;
        Directions     blocked;
        bool           right_axis;
        Arrow          arrow;
        int            count_tries;
    } state;

    struct {
        Point          current_cor;
        Point          ghost_cor;
        Boat          *target;
    } spot;
    
} AI;

Point bot_time(AI *bot, Map *map, Boat *boats);
AI *create_bot    (unsigned int difficulty);

#endif /* BOT_H */
