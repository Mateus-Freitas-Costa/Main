#ifndef GAME_H
#define GAME_H

#include <stdio.h>

#include "../main/extra.h"

Winner play_game(Info info);
// not working outside
void clear_screen       (void);
char *fgets_            (char *str, size_t size, FILE *stream);

#endif /* GAME_H */
