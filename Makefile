all:
	gcc -I include -I include/headers -L lib -o main src/*.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer && main.exe 