#include <stdlib.h>

#include "bot.h"

static void       restart                 (AI *bot);
static Previous   enqueue                 (Buff *buff);
static void       queue                   (Buff *buff, Boat *b, Point attack);
static Point      continue_attack         (AI *bot, Map *map, Boat *p[]);
static Point      new_attack              (AI *bot, Map *map, Boat *p[]);
static Arrow      reverse_arrow           (const Arrow a);
static bool      *get_blocked_direction   (AI *bot);
static bool       all_blocked             (const AI *bot);
static bool       are_the_same            (AI *bot, Boat *attacked);



void create_bot(AI *bot, const unsigned int d)
{
    bot->buff.count = 0;
    bot->level = d;
    bot->spot.target = NULL;
    restart(bot);
}

Point bot_time(AI *bot, Map *map, Boat *p[])
{
    if (bot->state.searching || bot->level == 0)
        return new_attack(bot, map, p);
    else 
        return continue_attack(bot, map, p);
}

static Point new_attack(AI *bot, Map *map, Boat *p[])
{
    if (bot->buff.count == 0) {
        Point attack = {rand() % HEIGHT, rand() % WIDTH};
        if (check_valid(attack, map->sea) != VALID) 
            return new_attack(bot, map, p);
        bot->spot.target = get_boat(p, attack, map->info.boats);
        if (bot->spot.target == NULL) {
            ++bot->state.count_tries;
            if (bot->state.count_tries >= MAX_TRIES) {
                bot->spot.current_cor = bot->spot.ghost_cor = attack;
                bot->state.count_tries = 0;
                return attack;
            }
            int chance = rand() % 5;
            if (chance < bot->level) {
                return new_attack(bot, map, p);
                bot->state.count_tries++;
            }
        }
        if (bot->spot.target != NULL) {
            bot->state.searching = false;
            bot->spot.current_cor = bot->spot.ghost_cor = attack;
        }
        return attack;
    } else {
        Previous previous = enqueue(&bot->buff);
        bot->spot.target = previous.target;
        bot->spot.current_cor = bot->spot.ghost_cor = previous.cor;
        bot->state.searching = false;
        return continue_attack(bot, map, p);
    }
}

static Point continue_attack(AI *bot, Map *map, Boat *p[])
{
    if (bot->spot.target->lifes == 0 || all_blocked(bot)) {
        restart(bot);
        return new_attack(bot, map, p);
    }

    if (!bot->state.right_axis) {
        for (;;) {
            Arrow new_arrow = rand() % NUMBER_DIRECTIONS;
            bot->state.arrow = new_arrow;
            if (*get_blocked_direction(bot)) 
                continue;
            else 
                break;
        }
    }
    
    switch (bot->state.arrow) {
        case UP:
            ++bot->spot.current_cor.y;
            break;
        case DOWN:
            --bot->spot.current_cor.y;
            break;
        case RIGHT:
            ++bot->spot.current_cor.x;
            break;
        case LEFT:
            --bot->spot.current_cor.x;
            break;
    }
    Point attack = bot->spot.current_cor;

    if (check_valid(attack, map->sea) != VALID) {
        bot->state.right_axis = false;
        *get_blocked_direction(bot) = true;
        bot->spot.current_cor = bot->spot.ghost_cor;
        return continue_attack(bot, map, p);
    }
    Boat *attacked = get_boat(p, attack, map->info.boats);
    if (attacked == NULL) {
        *get_blocked_direction(bot) = true;
        if (bot->state.right_axis && bot->level >= 2)
            bot->state.arrow = reverse_arrow(bot->state.arrow);
        bot->spot.current_cor = bot->spot.ghost_cor;
    } else if (are_the_same(bot, attacked)) {
        bot->state.right_axis = true;
    } else {
        bot->state.right_axis = false;
        if (bot->buff.count < bot->level / 2 && bot->buff.count <= BUFF_SIZE)
            queue(&bot->buff, attacked, attack);
        *get_blocked_direction(bot) = true;
        bot->spot.current_cor = bot->spot.ghost_cor;
    }
    return attack;
}

static bool are_the_same(AI *bot, Boat *attacked)
{
    if (bot->level >= 3) {
        return attacked == bot->spot.target;
    } else {
        return attacked->type == bot->spot.target->type;
    }
}

static bool *get_blocked_direction(AI *bot)
{
    switch (bot->state.arrow) {
    case UP:
        return &bot->state.blocked.up;
        break;
    case DOWN:
        return &bot->state.blocked.down;
        break;
    case RIGHT:
        return &bot->state.blocked.right;
        break;
    case LEFT:
        return &bot->state.blocked.left;
        break;
    }
}

static Previous enqueue(Buff *buff)
{
    Previous element = buff->boat[0];
    for (int i = 0; i < buff->count - 1; ++i)
        buff->boat[i] = buff->boat[i + 1];
    --buff->count;
    return element;
}

static void queue(Buff *buff, Boat *attacked, Point attack)
{
    buff->boat[buff->count].target = attacked;
    buff->boat[buff->count++].cor  = attack;
}

static Arrow reverse_arrow(const Arrow arrow)
{
    switch (arrow) {
    case UP:
        return DOWN;
    case DOWN:
        return UP;
    case LEFT:
        return RIGHT;
    case RIGHT:
        return LEFT;
    }
}

static void restart(AI *bot)
{
    bot->state.blocked.down = false;
    bot->state.blocked.up = false;
    bot->state.blocked.right = false;
    bot->state.blocked.left = false;
    bot->state.searching = true;
    bot->state.right_axis = false;
    bot->state.count_tries = 0;
}

static bool all_blocked(const AI *bot)
{
    return bot->state.blocked.left && bot->state.blocked.right &&
    bot->state.blocked.up && bot->state.blocked.down;
}
