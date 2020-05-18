#include <stdlib.h>
#include <stdbool.h>

#include "battle.h"

#define match_points(p1, p2) (p1.x == p2.x && p1.y == p2.y)

static bool create_boat(Boat *new, Boat *boats, Camp sea[HEIGHT][WIDTH], size_t size, Types type, const char *name);

Boat *get_boat(Boat *boats, Point attack, size_t boats_count)
{
    for (int i = 0; i < boats_count; ++i) 
        for (int j = 0; j < boats[i].size; ++j)
            if (match_points(boats[i].p[j], attack))
                    return &boats[i];
    return NULL;
}


void create_map(Info info, Map *map, Boat boats[MAX_PLAYERS][info.boats])
{
    const Types pattern[5] = {CARRIER, SUBMARINE, DESTROYER,
                                         BATTLESHIP, CRUISER};
    const int pattern_size[5] = {5, 3, 2, 4, 3};

    static const char *names[5] = {"Carrier", "Submarine", "Destroyer", "Battleship",
                                    "Cruiser"};

    for (int i = 0; i < MAX_PLAYERS; ++i) {
        map[i].remaining_boats = 0;
        map[i].HUD.battleships = 0;
        map[i].HUD.cruisers = 0;
        map[i].HUD.carriers = 0;
        map[i].HUD.submarines = 0;
        map[i].HUD.destroyers = 0;
        map[i].info = info;
        for (int j = 0; j < HEIGHT; ++j) 
            for (int k = 0; k < WIDTH; ++k) 
                map[i].sea[j][k] = INACTIVE;
    }

    for (int i = 0; i < info.boats; ++i) {
        Types type = pattern[i % 5];
        int size = pattern_size[type];
        const char *name = names[type];

        for (Player j = 0; j < MAX_PLAYERS; ++j) {
            while (!create_boat(&boats[j][i], boats[j], map[j].sea, size, type, name))
                ;
            update_hud(&map[j].HUD, type, +1);
            ++map[j].remaining_boats;
        }
    }

}

Validity check_valid(Camp sea[HEIGHT][WIDTH], Point attack)
{
    if (attack.x < 0 || attack.y < 0
        || attack.x >= WIDTH || attack.y >= HEIGHT)
        return OUTRANGE;
    else if (sea[attack.y][attack.x] == INACTIVE 
             || sea[attack.y][attack.x] == ACTIVE)
        return VALID;
    else
        return REPEATED;
}

void update_hud(Hud *HUD, Types type, int change)
{
    switch (type) {
    case DESTROYER:
        HUD->destroyers += change;
        break;
    case SUBMARINE:
        HUD->submarines += change;
        break;
    case CRUISER:
        HUD->cruisers += change;
        break;
    case CARRIER:
        HUD->carriers += change;
        break;
    case BATTLESHIP:
        HUD->battleships += change;
        break;
    }
}


static bool create_boat(Boat *new, Boat *boats, Camp sea[HEIGHT][WIDTH], size_t size, Types type, const char *name)
{
    new->p= malloc(size * sizeof(*new->p));
    if (new->p == NULL) {
        free(new);
        exit(EXIT_FAILURE);
    }

    enum {NORTH, SOUTH, EAST, WEST} direction = rand() % NUMBER_DIRECTIONS;
    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;
    
    for (int i = 0; i < size; ++i) {
        if (x >= WIDTH || y >= HEIGHT || x < 0 || y < 0) {
            free(new->p);
            return false;
        }
        if (sea[y][x] == ACTIVE) {
            free(new->p);
            return false;
        }
        new->p[i].x = x;
        new->p[i].y = y;
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
    new->name  = name; 
    new->type = type;
    for (int i = 0; i < size; ++i) 
            sea[new->p[i].y]
               [new->p[i].x] = ACTIVE;
    
    return true;
}
