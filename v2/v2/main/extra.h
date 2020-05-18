#ifndef EXTRA_H
#define EXTRA_H

#define NAME_LEN 20

typedef enum {PLAYER_ONE, PLAYER_TWO, BOT} Winner;

typedef struct {
    int  boats;
    int  mode;
    int  difficulty;
    char namep1[NAME_LEN + 1];
    char namep2[NAME_LEN + 2];
} Info;


#endif /* EXTRA_H */
