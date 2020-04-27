#ifndef BOT_H
#define BOT_H

#define BUFF_SIZE (NUMBER_DIRECTIONS - 1)
#define MAX_TRIES  20

#include "../battle/battle.h"

typedef enum {UP, DOWN, RIGHT, LEFT} Arrow;

typedef struct {
    Boat *target;
    Point cor;
} Previous;

typedef struct {
    Previous       boat[BUFF_SIZE];
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

Point bot_time       (AI *machine, Map *map, Boat *p[]);
void  create_bot     (AI *machine, const unsigned int d);

#endif /* BOT_H */
