#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "battle.h"

#define match_points(p1, p2) (p1.x == p2.x && p1.y == p2.y)

Boat *get_boat(Boat *boats, Point attack, int boats_count)
{
    for (int i = 0; i < boats_count; ++i) 
        for (int j = 0; j < boats[i].size; ++j)
            if (match_points(boats[i].p[j], attack))
                    return &boats[i];
    return NULL;
}

Validity check_valid(Camp sea[HEIGHT][WIDTH], Point attack)
{
    if (attack.x < 0 || attack.y < 0
        || attack.x >= WIDTH || attack.y >= HEIGHT)
        return OUTRANGE;
    else if (sea[attack.y][attack.x] == MISSED 
            || sea[attack.y][attack.x] == DESTROYED 
            || sea[attack.y][attack.x] == SUNK)
        return REPEATED;
    else
        return VALID;
}

void initialize_map(Map *map, Info info)
{
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        map[i].remaining_boats = 0;
        map[i].HUD = (Hud){0, 0, 0, 0, 0};
        map[i].info = info;
        for (int j = 0; j < HEIGHT; ++j)
            for (int k = 0; k < WIDTH; ++k)
                map[i].sea[j][k] = INACTIVE;
    }
}

typedef struct BoatMetaInfo {
    unsigned int size;
    Types        type;
    const char  *name;
} BoatMetaInfo;

static bool create_boat(Boat *new, Boat *boats, Camp sea[HEIGHT][WIDTH], const BoatMetaInfo *boat);
static BoatMetaInfo boat_create_meta_info(int i);

static BoatMetaInfo boat_create_meta_info(int i)
{
    static const Types pattern[5] = {CARRIER, SUBMARINE, DESTROYER,
                                         BATTLESHIP, CRUISER};
    static const int pattern_size[5] = {5, 3, 2, 4, 3};
    static const char *names[5] = {"Carrier", "Submarine", "Destroyer", "Battleship",
                                    "Cruiser"};

    int j = i % 5;
    return (BoatMetaInfo){pattern_size[j], pattern[j], names[j]};
}

void create_map(Info info, Map *map, Boat boats[MAX_PLAYERS][info.boats])
{
    for (int i = 0; i < info.boats; ++i) {
        BoatMetaInfo boat = boat_create_meta_info(i);

        for (Player j = 0; j < MAX_PLAYERS; ++j) {
            while (!create_boat(&boats[j][i], boats[j], map[j].sea, &boat))
                ;
            update_hud(&map[j].HUD, boat.type, +1);
            ++map[j].remaining_boats;
        }
    }
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


static bool create_boat(Boat *new, Boat *boats, Camp sea[HEIGHT][WIDTH], const BoatMetaInfo *boat)
{
    new->p= malloc(boat->size * sizeof(*new->p));
    if (new->p == NULL) {
        free(new->p);
        fprintf(stderr, "Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    enum {NORTH, SOUTH, EAST, WEST} direction = rand() % NUMBER_DIRECTIONS;
    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;
    
    for (int i = 0; i < boat->size; ++i) {
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
    new->lifes = boat->size;
    new->size  = boat->size;
    new->name  = boat->name; 
    new->type = boat->type;
    for (int i = 0; i < boat->size; ++i) 
            sea[new->p[i].y]
               [new->p[i].x] = ACTIVE;
    
    return true;
}
