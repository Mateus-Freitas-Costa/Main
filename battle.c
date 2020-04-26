#include <stdlib.h>
#include <stdbool.h>

#include "battle.h"

static Boat  *create_boat   (const Types type, const int size, Camp sea[HEIGHT][WIDTH]);
static char  *get_name      (const Types type);
static bool   match_points  (const Point p1, const Point p2);

Boat *get_boat(Boat *p[], const Point attack, const size_t boats)
{
     for (int i = 0; i < boats; ++i) 
        for (int j = 0; j < p[i]->size; ++j) 
            if (match_points(attack, p[i]->coordinates[j].p)) 
                return p[i];
    return NULL;
}

int get_coordinate_location(const Boat *p, const Point attack)
{   
    for (int i = 0; i < p->size; ++i) 
        if (match_points(attack, p->coordinates[i].p))
            return i;
}

static bool match_points(const Point p1, const Point p2)
{
    return p1.x == p2.x && p1.y == p2.y;
}

Validity check_valid(const Point attack, const Camp sea[][WIDTH])
{
    if (attack.x >= WIDTH || attack.y >= HEIGHT || 
    attack.x < 0 || attack.y < 0)
        return OUTRANGE;
    if (sea[attack.y][attack.x] == MISSED || 
        sea[attack.y][attack.x] == DESTROYED)
            return REPEATED;
    return VALID;
}

void create_map(Boat *p1[], Boat *p2[], Map map[], const Info info)
{
    const Types pattern[5] = {DESTROYER, SUBMARINE, CRUISER,
                                         BATTLESHIP, CARRIER};
    const int pattern_size[5] = {2, 3, 3, 4, 5};

    for (int i = 0; i < MAX_PLAYERS; ++i) {
        map[i].remaining_boats = 0;
        map[i].HUD.battleships = 0;
        map[i].HUD.cruisers = 0;
        map[i].HUD.carriers = 0;
        map[i].HUD.submarines = 0;
        map[i].HUD.destroyers = 0;
        map[i].info = info;
        for (int j = 0; j < HEIGHT; ++j) {
            for (int k = 0; k < WIDTH; ++k) {
                map[i].sea[j][k] = INACTIVE;
            } 
        }
    }
    for (int i = 0; i < info.boats; ++i) {
        Types type = pattern[i % 5];
        int size = pattern_size[type];
        while ((p1[i] = create_boat(type, size, map[PLAYER1].sea)) == NULL)
            continue;
        update_HUD(&map[PLAYER1].HUD, type, +1);
        ++map[PLAYER1].remaining_boats;

        while ((p2[i] = create_boat(type, size, map[OPPONENT].sea)) == NULL)
            continue;
        update_HUD(&map[OPPONENT].HUD, type, +1);
        ++map[OPPONENT].remaining_boats;
    }

}

void update_HUD(Hud *HUD, const Types type, int change)
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


static Boat *create_boat(const Types type, const int size, Camp sea[HEIGHT][WIDTH])
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

static char  *get_name(const Types type)
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
