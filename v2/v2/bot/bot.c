#include <stdlib.h>

#include "bot.h"

static void       restart                 (AI *bot);
static Point      enqueue                 (Buff *buff);
static void       queue                   (Buff *buff, Point attack);
static Point      continue_attack         (AI *bot, Map *map, Boat *p[]);
static Point      new_attack              (AI *bot, Map *map, Boat *p[]);
static Arrow      reverse_arrow           (Arrow a);
static bool      *get_blocked_direction   (AI *bot);
static bool       all_blocked             (const AI *bot);
static bool       are_the_same            (const AI *bot,const Boat *attacked);
static void       goto_initial_point      (AI *bot);

void create_bot(AI *bot, unsigned int difficulty)
{
    bot->buff.count = 0;
    switch (difficulty) {
    case 0:
        bot->level = 1;
        break;
    case 1:
        bot->level = 3;
        break;
    case 2:
        bot->level = 4;
        break;  
    case 3:
        bot->level = 5;
        break;
    case 4:
        bot->level = 7;
        break;
    case 5:
        bot->level = 8;
        break;
    case 6:
        bot->level = 10;
        break;
    }
    restart(bot);
}

Point bot_time(AI *bot, Map *map, Boat *p[])
{
    if (bot->state.searching || bot->level == 1)
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
            int chance = rand() % 10;
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
        bot->spot.current_cor = bot->spot.ghost_cor = enqueue(&bot->buff);
        bot->spot.target = get_boat(p, bot->spot.current_cor, map->info.boats);
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
            if (!*get_blocked_direction(bot)) 
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
        goto_initial_point(bot);
        return continue_attack(bot, map, p);
    }
    Boat *attacked = get_boat(p, attack, map->info.boats);
    if (attacked == NULL) {
        goto_initial_point(bot);
        if (bot->level == 10)
            return continue_attack(bot, map, p);
        if (bot->level == 8) {
            int chance = rand() % 10;
            if (chance <= 5)
                return continue_attack(bot, map, p);
        }
    } else if (are_the_same(bot, attacked)) {
        bot->state.right_axis = true;
    } else {
        if (bot->level >= 5 && bot->buff.count <= BUFF_SIZE)
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
    if (bot->level >= 7) {
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
