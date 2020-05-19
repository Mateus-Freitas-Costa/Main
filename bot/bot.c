#include <stdlib.h>
#include <unistd.h> 
#include <stdio.h>

#include "bot.h"

static void       restart                 (AI *bot);
static Point      enqueue                 (Buff *buff);
static void       queue                   (Buff *buff, Point attack);
static Point      continue_attack         (AI *bot, Map *map, Boat *boats);
static Point      new_attack              (AI *bot, Map *map, Boat *boats);
static Arrow      reverse_arrow           (Arrow a);
static bool      *get_blocked_direction   (AI *bot);
static bool       all_blocked             (const AI *bot);
static bool       are_the_same            (const AI *bot,const Boat *attacked);
static void       goto_initial_point      (AI *bot);

AI *create_bot(unsigned int difficulty)
{
    AI *new_bot = malloc(sizeof(*new_bot));
    const int levels[] = {1, 3, 4, 5, 7, 8, 10};

    new_bot->buff.count = 0;
    new_bot->level = levels[difficulty];
    restart(new_bot);
    return new_bot;
}

Point bot_time(AI *bot, Map *map, Boat *boats)
{
    if (bot->state.searching || bot->level == 1) 
        return new_attack(bot, map, boats);
    else 
        return continue_attack(bot, map, boats);
}

static Point new_attack(AI *bot, Map *map, Boat *boats)
{
    if (bot->buff.count == 0) {
        for (;;) {
            Point attack = {rand() % HEIGHT, rand() % WIDTH};

            if (check_valid(map->sea, attack) == REPEATED) 
                continue;
            bot->spot.target = get_boat(boats, attack, map->info.boats);
            if (bot->spot.target == NULL) {
                int chance = rand() % 10;

                if (chance < bot->level) 
                    continue;
            } else {
                bot->state.searching = false;
                bot->spot.current_cor = bot->spot.ghost_cor = attack;
            }
            return attack;
        }
    } else {
        bot->spot.current_cor = bot->spot.ghost_cor = enqueue(&bot->buff);
        bot->spot.target = get_boat(boats, bot->spot.current_cor, map->info.boats);
        bot->state.searching = false;
        return continue_attack(bot, map, boats);
    }
}

static Point continue_attack(AI *bot, Map *map, Boat *boats)
{
    if (bot->spot.target->lifes == 0 || all_blocked(bot)) {
        restart(bot);
        return new_attack(bot, map, boats);
    }

    if (!bot->state.right_axis) {
        do {
            bot->state.arrow = rand() % NUMBER_DIRECTIONS;
        } while (*get_blocked_direction(bot));
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

    if (check_valid(map->sea, attack) != VALID) {
        goto_initial_point(bot);
        return continue_attack(bot, map, boats);
    }

    Boat *attacked = get_boat(boats, attack, map->info.boats);

    if (attacked == NULL) {
        goto_initial_point(bot);
        if (bot->level == 10)
            return continue_attack(bot, map, boats);
        if (bot->level == 8) {
            int chance = rand() % 10;
            if (chance <= 5)
                return continue_attack(bot, map, boats);
        }
    } else if (are_the_same(bot, attacked)) {
        bot->state.right_axis = true;
    } else {
        if (bot->level >= 5 && bot->buff.count < BUFF_SIZE)
            queue(&bot->buff, attack);
        goto_initial_point(bot);
    }
    return attack;
}

static void goto_initial_point(AI *bot)
{
    *get_blocked_direction(bot) = true;
    if (bot->state.right_axis && bot->level >= 4)
        bot->state.arrow = reverse_arrow(bot->state.arrow);
    bot->spot.current_cor = bot->spot.ghost_cor;
}

static bool are_the_same(const AI *bot, const Boat *attacked)
{
    if (bot->level >= 7) 
        return attacked == bot->spot.target;
    else 
        return attacked->type == bot->spot.target->type;
}

static bool *get_blocked_direction(AI *bot)
{
    switch (bot->state.arrow) {
        case UP:
            return &bot->state.blocked.up;
        case DOWN:
            return &bot->state.blocked.down;
        case RIGHT:
            return &bot->state.blocked.right;
        case LEFT:
            return &bot->state.blocked.left;
    }
}

static Point enqueue(Buff *buff)
{
    Point element = buff->previous[0];

    for (int i = 0; i < buff->count - 1; ++i)
        buff->previous[i] = buff->previous[i + 1];
    --buff->count;
    return element;
}

static void queue(Buff *buff, Point attack)
{
    buff->previous[buff->count++] = attack;
}

static Arrow reverse_arrow(Arrow arrow)
{
    return (arrow + 2) % NUMBER_DIRECTIONS; 
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
    bot->spot.target = NULL;
}

static bool all_blocked(const AI *bot)
{
    return bot->state.blocked.left && bot->state.blocked.right &&
        bot->state.blocked.up && bot->state.blocked.down;
}
