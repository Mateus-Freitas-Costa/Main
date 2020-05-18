#ifndef GAME_H
#define GAME_H

#include <stdio.h>

#include "../main/extra.h"
#include "../battle/battle.h"

Instance play_game(Info info);
void clear_screen(void);
char *fgets_            (char *str, size_t size, FILE *stream);

#endif /* GAME_H */
