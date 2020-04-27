main: main.o game.o battle.o bot.o
	gcc -o main main.o game.o battle.o bot.o
main.o: main.c ../game/game.h
	gcc -c main.c
game.o: ../game/game.c ../game/game.h ../battle/battle.h ../bot/bot.h
	gcc -c ../game/game.c 
battle.o: ../battle/battle.c ../battle/battle.h
	gcc -c ../battle/battle.c
bot.o: ../bot/bot.c ../bot/bot.h 
	gcc -c ../bot/bot.c
