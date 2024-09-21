#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

const int WIDTH = 720, HEIGHT = 540;

const float GRAVITY = 0.30;

long int score, highScore;
int lives;
int prices[7] = {70, 25, 40, 25, 10, 10, 50};
int item[9] = {0};
const char itemNames[9][101] = {"x Burger", "x Chicken Ball", "x Double Coffee", "x Lemonade", "x Samosa", "x Chaa", "x Sandwich", "x Bombs", "x Joshims hurt"};
int difficulty;
// bool fullScreen = false;

// game structs
typedef struct
{
    float x, y, dy, dx;
    int h, w, frame;
    bool right, up;
} Man;

typedef struct
{
    float x, y, dy;
    int h, w, frame;
    bool screen;

} Things;

typedef struct
{
    float x, y, dx;
    int h, w, frame;
    bool screen, right;

} Cat;

typedef struct
{
    Man man;
    Things food[10];
    Cat joshim;

    unsigned int time;

    TTF_Font *font;

    Mix_Music *menuMusic, *gameMusic, *minigameMusic;
    Mix_Chunk *soundBytes[5];

    SDL_Surface *scoreSurface, *livesSurface, *textSurface, *highScoreSurface;

    SDL_Renderer *renderer;

} GameObjects;

typedef struct
{
    SDL_Texture *character[5];

    SDL_Texture *burger;
    SDL_Texture *chknball;
    SDL_Texture *dblcoffee;
    SDL_Texture *lemonade;
    SDL_Texture *samosa;
    SDL_Texture *chaa;
    SDL_Texture *sndwch;

    SDL_Texture *bomb1[2];
    SDL_Texture *bomb2[2];
    SDL_Texture *bomb3[2];

    SDL_Texture *josh[2];

    SDL_Texture *hearts[4];

    SDL_Texture *mainScreen;
    SDL_Texture *background;

    SDL_Texture *manualPage;
    SDL_Texture *optionsPage;

    SDL_Texture *itemNo[9], *itemLabels[9], *itemTotal[9], *priceTotal[3];
    SDL_Texture *receiptbg, *taka;

    SDL_Texture *scoreText[2], *livesText;

    SDL_Texture *highScoretext[2];

    SDL_Texture *playText[2], *manualText[2], *exitText[2];
    SDL_Texture *mainMenuText[2], *playAgainText[2];
    SDL_Texture *manualMenuText[2];
    SDL_Texture *resetText[2], *diff1[2], *diff2[2], *diff3[2];
    int hover[10];

} GameTextures;

// minigame structs
typedef struct
{
    float a, b, yspeed, xspeed;
    int w, h, frame;
    bool right;

} Mincat;

typedef struct
{
    float c, d;
    int w, h, move, size;

} MinPlatform;

typedef struct
{
    int ee[42], ff[42];
    int j;
    int targw, targh, targnum;

} MinTarget;

typedef struct
{
    Mincat mincat;
    MinPlatform minplat;
    MinTarget tar;
    SDL_Texture *cat[2], *plat, *targ, *back, *panjim, *heart;
    int time;

} MinGameObjects;

// pages
typedef enum
{
    PAGE_STATUS_MENU,
    PAGE_STATUS_GAME,
    PAGE_STATUS_MINIGAME,
    PAGE_STATUS_MANUAL,
    PAGE_STATUS_OPTIONS,
    PAGE_STATUS_OVER
} PageStatus;

PageStatus page;

/*
0 = burger
1 = chknball
2 = dblcoffee
3 = lemonade
4 = samosa
5 = chaa
6 = sndwch
7-9 = bomb
*/

// player
void loadMan(GameObjects *object)
{
    // man
    object->man.h = 155;
    object->man.w = 100;

    if (difficulty == 8)
        object->man.dx = 3.25;
    if (difficulty == 9)
        object->man.dx = 3;
    if (difficulty == 10)
        object->man.dx = 2.25;

    object->man.dy = 0;
    object->man.up = false;
    object->man.x = ((WIDTH - object->man.w) / 2);
    object->man.y = HEIGHT - object->man.h;
    object->man.frame = 0;
    object->man.right = true;

    // time
    object->time = 0;
}

void manProcess(GameObjects *object)
{
    // time
    object->time++;

    // movement
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A])
    {
        object->man.right = false;
        if (object->man.x > 0)
        {
            object->man.x -= object->man.dx;
        }
        if (object->time % 15 == 0)
        {
            if (object->man.frame == 0)
                object->man.frame = 1;
            else
                object->man.frame = 0;
        }
    }
    else if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D])
    {
        object->man.right = true;
        if (object->man.x < (WIDTH - object->man.w))
        {
            object->man.x += object->man.dx;
        }
        if (object->time % 25 == 0)
        {
            if (object->man.frame == 0)
                object->man.frame = 1;
            else
                object->man.frame = 0;
        }
    }
    else
    {
        if (object->time % 35 == 0)
        {
            if (object->man.frame == 2)
                object->man.frame = 3;
            else
                object->man.frame = 2;
        }
    }

    // jump mechanics
    if (object->man.up)
    {
        object->man.frame = 4;

        object->man.y -= object->man.dy;
        object->man.dy -= GRAVITY;

        if (object->man.y >= (HEIGHT - object->man.h))
        {
            object->man.dy = 0;
            object->man.y = (HEIGHT - object->man.h);
            object->man.up = false;
        }
    }
}

void manJump(GameObjects *object)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP))
        {
            if (object->man.y > 0)
            {
                Mix_PlayChannel(-1, object->soundBytes[3], 0);
                object->man.dy = 2.5;
                object->man.y = HEIGHT - object->man.h - 225;
                object->man.up = true;
            }
        }
    }
}

// food
void loadFood(GameObjects *object, int i)
{
    if (i > 6)
    {
        object->food[i].w = 66;
        object->food[i].h = 96.8;
    }
    else
    {
        object->food[i].w = 60;
        object->food[i].h = 60;
    }
    object->food[i].x = (rand() % (WIDTH - object->food[i].w));
    object->food[i].y = HEIGHT;
    object->food[i].dy = 1 + (i / 3);
    object->food[i].screen = false;
    object->food[i].frame = 0;
}

void foodProcess(GameObjects *object)
{
    for (int i = 0; i < difficulty; i++)
    {
        if (object->food[i].screen)
        {
            object->food[i].y += object->food[i].dy;
        }
        if (object->food[i].y == HEIGHT)
        {
            object->food[i].screen = false;
        }
        if (i > 6)
        {
            if (object->time % 15 == 0 && object->food[i].screen)
            {
                if (object->food[i].frame == 0)
                    object->food[i].frame = 1;
                else
                    object->food[i].frame = 0;
            }
        }
    }
}

bool foodCollision(GameObjects *obj1, GameObjects *obj2, int i)
{
    float x1 = obj1->man.x, y1 = obj1->man.y;
    float w1 = obj1->man.w, h1 = obj1->man.h;

    float x2 = obj2->food[i].x, y2 = obj2->food[i].y;
    float w2 = obj2->food[i].w, h2 = obj2->food[i].h;

    if (x1 < (x2 + w2) && x2 < (x1 + w1) && y1 < (y2 + h2) && y2 < (y1 + h1))
        return true;

    return false;
}

void dropFood(GameObjects *object, int i)
{
    object->food[i].screen = true;
    object->food[i].x = (rand() % (WIDTH - object->food[i].w));
    object->food[i].y = -(object->food[i].h);
}

// joshim
void loadJoshim(GameObjects *object)
{
    object->joshim.h = 84;
    object->joshim.w = 108;
    object->joshim.x = -(object->joshim.w);
    object->joshim.y = HEIGHT;
    object->joshim.frame = 0;
    object->joshim.screen = false;
}

void joshimProcess(GameObjects *object)
{
    if (object->joshim.screen)
    {
        object->joshim.x += object->joshim.dx;
        if (object->time % 17 == 0)
        {
            if (object->joshim.frame == 0)
                object->joshim.frame = 1;
            else
                object->joshim.frame = 0;
        }
    }
    if (object->joshim.x < -(object->joshim.w) || object->joshim.x > WIDTH)
        object->joshim.screen = false;
}

bool joshimCollision(GameObjects *obj1, GameObjects *obj2)
{
    float x1 = obj1->man.x, y1 = obj1->man.y;
    float w1 = obj1->man.w, h1 = obj1->man.h;

    float x2 = obj2->joshim.x, y2 = obj2->joshim.y;
    float w2 = obj2->joshim.w, h2 = obj2->joshim.h;

    if (x1 < (x2 + w2) && x2 < (x1 + w1) && y1 < (y2 + h2) && y2 < (y1 + h1))
        return true;

    return false;
}

void joshimLeft(GameObjects *object)
{
    object->joshim.x = -(object->joshim.w);
    object->joshim.y = HEIGHT - object->joshim.h;
    object->joshim.dx = 2;
    object->joshim.frame = 0;
    object->joshim.right = false;
    object->joshim.screen = true;
}

void joshimRight(GameObjects *object)
{
    object->joshim.x = WIDTH;
    object->joshim.y = HEIGHT - object->joshim.h;
    object->joshim.dx = -2;
    object->joshim.frame = 0;
    object->joshim.right = true;
    object->joshim.screen = true;
}

// music
void loadMusic(GameObjects *object)
{
    object->menuMusic = Mix_LoadMUS("sound/Undertale OST - Hotel.mp3");
    object->gameMusic = Mix_LoadMUS("sound/Undertale OST - Can You Really Call This A Hotel I Didn't Receive A Mint On My Pillow.mp3");
    object->minigameMusic = Mix_LoadMUS("sound/Undertale OST - Ghost Fight.mp3");

    object->soundBytes[0] = Mix_LoadWAV("sound/eatSound.wav");
    object->soundBytes[1] = Mix_LoadWAV("sound/joshimSound.wav");
    object->soundBytes[2] = Mix_LoadWAV("sound/boomSound.wav");
    object->soundBytes[3] = Mix_LoadWAV("sound/jumpSound.wav");
    object->soundBytes[4] = Mix_LoadWAV("sound/gameoverSound.wav");
}

void destroyMusic(GameObjects *object)
{
    Mix_FreeMusic(object->gameMusic);
    Mix_FreeMusic(object->menuMusic);
    Mix_FreeMusic(object->minigameMusic);
    for (int i = 0; i < 5; i++)
    {
        Mix_FreeChunk(object->soundBytes[i]);
    }
    Mix_CloseAudio();
}

// visuals
void destroyTextures(GameTextures *texture)
{
    SDL_DestroyTexture(texture->burger);
    SDL_DestroyTexture(texture->chknball);
    SDL_DestroyTexture(texture->dblcoffee);
    SDL_DestroyTexture(texture->lemonade);
    SDL_DestroyTexture(texture->samosa);
    SDL_DestroyTexture(texture->chaa);
    SDL_DestroyTexture(texture->sndwch);

    SDL_DestroyTexture(texture->mainScreen);
    SDL_DestroyTexture(texture->background);

    SDL_DestroyTexture(texture->manualPage);
    SDL_DestroyTexture(texture->optionsPage);

    for (int i = 0; i < 5; i++)
    {
        SDL_DestroyTexture(texture->character[i]);
    }

    for (int j = 0; j < 2; j++)
    {
        SDL_DestroyTexture(texture->bomb1[j]);
        SDL_DestroyTexture(texture->bomb2[j]);
        SDL_DestroyTexture(texture->bomb3[j]);

        SDL_DestroyTexture(texture->josh[j]);

        SDL_DestroyTexture(texture->playText[j]);
        SDL_DestroyTexture(texture->manualText[j]);
        SDL_DestroyTexture(texture->exitText[j]);

        SDL_DestroyTexture(texture->highScoretext[j]);

        SDL_DestroyTexture(texture->mainMenuText[j]);
        SDL_DestroyTexture(texture->playAgainText[j]);

        SDL_DestroyTexture(texture->manualMenuText[j]);

        SDL_DestroyTexture(texture->resetText[j]);
        SDL_DestroyTexture(texture->diff1[j]);
        SDL_DestroyTexture(texture->diff2[j]);
        SDL_DestroyTexture(texture->diff3[j]);
    }

    for (int k = 0; k < 4; k++)
    {
        SDL_DestroyTexture(texture->hearts[k]);
    }

    SDL_DestroyTexture(texture->receiptbg);
}

void destroyTextTextures(GameTextures *texture)
{
    for (int i = 0; i < 9; i++)
    {
        SDL_DestroyTexture(texture->itemNo[i]);
        SDL_DestroyTexture(texture->itemLabels[i]);
        SDL_DestroyTexture(texture->itemTotal[i]);
    }
    SDL_DestroyTexture(texture->priceTotal[0]);
    SDL_DestroyTexture(texture->priceTotal[1]);
    SDL_DestroyTexture(texture->priceTotal[2]);
    SDL_DestroyTexture(texture->taka);
}

void loadSurfaces(GameObjects *object, GameTextures *texture, SDL_Renderer *renderer)
{
    // buttons
    for (int i = 0; i < 10; i++)
    {
        texture->hover[i] = 0;
    }

    // options page
    SDL_Surface *optionsPageSurface1 = IMG_Load("graphics/options.png");
    if (optionsPageSurface1 == NULL)
    {
        printf("Error: options.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->optionsPage = SDL_CreateTextureFromSurface(renderer, optionsPageSurface1);
    SDL_FreeSurface(optionsPageSurface1);

    // reset
    SDL_Surface *resetSurface1 = IMG_Load("graphics/resettext1.png");
    if (resetSurface1 == NULL)
    {
        printf("Error: resettext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->resetText[0] = SDL_CreateTextureFromSurface(renderer, resetSurface1);
    SDL_FreeSurface(resetSurface1);

    SDL_Surface *resetSurface2 = IMG_Load("graphics/resettext2.png");
    if (resetSurface2 == NULL)
    {
        printf("Error: resettext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->resetText[1] = SDL_CreateTextureFromSurface(renderer, resetSurface2);
    SDL_FreeSurface(resetSurface2);

    // diff 1
    SDL_Surface *diff1Surface1 = IMG_Load("graphics/diff11.png");
    if (diff1Surface1 == NULL)
    {
        printf("Error: diff11.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->diff1[0] = SDL_CreateTextureFromSurface(renderer, diff1Surface1);
    SDL_FreeSurface(diff1Surface1);

    SDL_Surface *diff1Surface2 = IMG_Load("graphics/diff12.png");
    if (diff1Surface2 == NULL)
    {
        printf("Error: diff12.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->diff1[1] = SDL_CreateTextureFromSurface(renderer, diff1Surface2);
    SDL_FreeSurface(diff1Surface2);

    // diff 2
    SDL_Surface *diff2Surface1 = IMG_Load("graphics/diff21.png");
    if (diff2Surface1 == NULL)
    {
        printf("Error: diff21.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->diff2[0] = SDL_CreateTextureFromSurface(renderer, diff2Surface1);
    SDL_FreeSurface(diff2Surface1);

    SDL_Surface *diff2Surface2 = IMG_Load("graphics/diff22.png");
    if (diff2Surface2 == NULL)
    {
        printf("Error: diff22.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->diff2[1] = SDL_CreateTextureFromSurface(renderer, diff2Surface2);
    SDL_FreeSurface(diff2Surface2);

    // diff 3
    SDL_Surface *diff3Surface1 = IMG_Load("graphics/diff31.png");
    if (diff3Surface1 == NULL)
    {
        printf("Error: diff31.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->diff3[0] = SDL_CreateTextureFromSurface(renderer, diff3Surface1);
    SDL_FreeSurface(diff2Surface1);

    SDL_Surface *diff3Surface2 = IMG_Load("graphics/diff32.png");
    if (diff3Surface2 == NULL)
    {
        printf("Error: diff32.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->diff3[1] = SDL_CreateTextureFromSurface(renderer, diff3Surface2);
    SDL_FreeSurface(diff3Surface2);

    // manual pages
    SDL_Surface *manualPageSurface1 = IMG_Load("graphics/instructions.png");
    if (manualPageSurface1 == NULL)
    {
        printf("Error: instructions.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->manualPage = SDL_CreateTextureFromSurface(renderer, manualPageSurface1);
    SDL_FreeSurface(manualPageSurface1);

    // manual menu button
    SDL_Surface *manualMenuSurface1 = IMG_Load("graphics/manualmenutext1.png");
    if (manualMenuSurface1 == NULL)
    {
        printf("Error: manualmenutext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->manualMenuText[0] = SDL_CreateTextureFromSurface(renderer, manualMenuSurface1);
    SDL_FreeSurface(manualMenuSurface1);

    SDL_Surface *manualMenuSurface2 = IMG_Load("graphics/manualmenutext2.png");
    if (manualMenuSurface2 == NULL)
    {
        printf("Error: manualmenutext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->manualMenuText[1] = SDL_CreateTextureFromSurface(renderer, manualMenuSurface2);
    SDL_FreeSurface(manualMenuSurface2);

    // man
    SDL_Surface *charSurface1 = IMG_Load("graphics/character1.png");
    if (charSurface1 == NULL)
    {
        printf("Error: character1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->character[0] = SDL_CreateTextureFromSurface(renderer, charSurface1);
    SDL_FreeSurface(charSurface1);

    SDL_Surface *charSurface2 = IMG_Load("graphics/character2.png");
    if (charSurface2 == NULL)
    {
        printf("Error: character2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->character[1] = SDL_CreateTextureFromSurface(renderer, charSurface2);
    SDL_FreeSurface(charSurface2);

    SDL_Surface *charSurface3 = IMG_Load("graphics/character3.png");
    if (charSurface3 == NULL)
    {
        printf("Error: character3.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->character[2] = SDL_CreateTextureFromSurface(renderer, charSurface3);
    SDL_FreeSurface(charSurface3);

    SDL_Surface *charSurface4 = IMG_Load("graphics/character4.png");
    if (charSurface4 == NULL)
    {
        printf("Error: character4.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->character[3] = SDL_CreateTextureFromSurface(renderer, charSurface4);
    SDL_FreeSurface(charSurface4);

    SDL_Surface *charSurface5 = IMG_Load("graphics/character5.png");
    if (charSurface5 == NULL)
    {
        printf("Error: character5.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->character[4] = SDL_CreateTextureFromSurface(renderer, charSurface5);
    SDL_FreeSurface(charSurface5);

    // food
    SDL_Surface *burgerSurface = IMG_Load("graphics/burger.png");
    if (burgerSurface == NULL)
    {
        printf("Error: burger.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->burger = SDL_CreateTextureFromSurface(renderer, burgerSurface);
    SDL_FreeSurface(burgerSurface);

    SDL_Surface *chknballSurface = IMG_Load("graphics/chknball.png");
    if (chknballSurface == NULL)
    {
        printf("Error: chknball.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->chknball = SDL_CreateTextureFromSurface(renderer, chknballSurface);
    SDL_FreeSurface(chknballSurface);

    SDL_Surface *dblcoffeeSurface = IMG_Load("graphics/dblcoffee.png");
    if (dblcoffeeSurface == NULL)
    {
        printf("Error: dblcoffee.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->dblcoffee = SDL_CreateTextureFromSurface(renderer, dblcoffeeSurface);
    SDL_FreeSurface(dblcoffeeSurface);

    SDL_Surface *lemonadeSurface = IMG_Load("graphics/lemonade.png");
    if (lemonadeSurface == NULL)
    {
        printf("Error: lemonade.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->lemonade = SDL_CreateTextureFromSurface(renderer, lemonadeSurface);
    SDL_FreeSurface(lemonadeSurface);

    SDL_Surface *samosaSurface = IMG_Load("graphics/samosa.png");
    if (samosaSurface == NULL)
    {
        printf("Error: samosa.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->samosa = SDL_CreateTextureFromSurface(renderer, samosaSurface);
    SDL_FreeSurface(samosaSurface);

    SDL_Surface *chaaSurface = IMG_Load("graphics/chaa.png");
    if (chaaSurface == NULL)
    {
        printf("Error: chaa.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->chaa = SDL_CreateTextureFromSurface(renderer, chaaSurface);
    SDL_FreeSurface(chaaSurface);

    SDL_Surface *sndwchSurface = IMG_Load("graphics/sndwch.png");
    if (sndwchSurface == NULL)
    {
        printf("Error: sndwch.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->sndwch = SDL_CreateTextureFromSurface(renderer, sndwchSurface);
    SDL_FreeSurface(sndwchSurface);

    // bombs
    SDL_Surface *bombSurface1 = IMG_Load("graphics/bomb1.png");
    if (bombSurface1 == NULL)
    {
        printf("Error: bomb1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->bomb1[0] = SDL_CreateTextureFromSurface(renderer, bombSurface1);
    SDL_FreeSurface(bombSurface1);

    SDL_Surface *bombSurface2 = IMG_Load("graphics/bomb2.png");
    if (bombSurface2 == NULL)
    {
        printf("Error: bomb2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->bomb1[1] = SDL_CreateTextureFromSurface(renderer, bombSurface2);
    SDL_FreeSurface(bombSurface2);

    SDL_Surface *bombSurface3 = IMG_Load("graphics/bomb1.png");
    if (bombSurface3 == NULL)
    {
        printf("Error: bomb1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->bomb2[0] = SDL_CreateTextureFromSurface(renderer, bombSurface3);
    SDL_FreeSurface(bombSurface3);

    SDL_Surface *bombSurface4 = IMG_Load("graphics/bomb2.png");
    if (bombSurface4 == NULL)
    {
        printf("Error: bomb2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->bomb2[1] = SDL_CreateTextureFromSurface(renderer, bombSurface4);
    SDL_FreeSurface(bombSurface4);

    SDL_Surface *bombSurface5 = IMG_Load("graphics/bomb1.png");
    if (bombSurface5 == NULL)
    {
        printf("Error: bomb1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->bomb3[0] = SDL_CreateTextureFromSurface(renderer, bombSurface5);
    SDL_FreeSurface(bombSurface5);

    SDL_Surface *bombSurface6 = IMG_Load("graphics/bomb2.png");
    if (bombSurface6 == NULL)
    {
        printf("Error: bomb2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->bomb3[1] = SDL_CreateTextureFromSurface(renderer, bombSurface6);
    SDL_FreeSurface(bombSurface6);

    // background
    SDL_Surface *bgSurface = IMG_Load("graphics/background.png");
    if (bgSurface == NULL)
    {
        printf("Error: background.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->background = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    // screen
    SDL_Surface *mainSurface = IMG_Load("graphics/mainScreen.png");
    if (mainSurface == NULL)
    {
        printf("Error: mainScreen.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->mainScreen = SDL_CreateTextureFromSurface(renderer, mainSurface);
    SDL_FreeSurface(mainSurface);

    // joshim
    SDL_Surface *joshimSurface1 = IMG_Load("graphics/joshim1.png");
    if (joshimSurface1 == NULL)
    {
        printf("Error: joshim1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->josh[0] = SDL_CreateTextureFromSurface(renderer, joshimSurface1);
    SDL_FreeSurface(joshimSurface1);

    SDL_Surface *joshimSurface2 = IMG_Load("graphics/joshim2.png");
    if (joshimSurface2 == NULL)
    {
        printf("Error: joshim2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->josh[1] = SDL_CreateTextureFromSurface(renderer, joshimSurface2);
    SDL_FreeSurface(joshimSurface2);

    // receipt
    SDL_Surface *receiptSurface = IMG_Load("graphics/receipt.png");
    if (receiptSurface == NULL)
    {
        printf("Error: receipt.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->receiptbg = SDL_CreateTextureFromSurface(renderer, receiptSurface);
    SDL_FreeSurface(receiptSurface);

    // hearts
    SDL_Surface *livesSurface1 = IMG_Load("graphics/lives0.png");
    if (livesSurface1 == NULL)
    {
        printf("Error: lives0.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->hearts[0] = SDL_CreateTextureFromSurface(renderer, livesSurface1);
    SDL_FreeSurface(livesSurface1);

    SDL_Surface *livesSurface2 = IMG_Load("graphics/lives1.png");
    if (livesSurface2 == NULL)
    {
        printf("Error: lives1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->hearts[1] = SDL_CreateTextureFromSurface(renderer, livesSurface2);
    SDL_FreeSurface(livesSurface2);

    SDL_Surface *livesSurface3 = IMG_Load("graphics/lives2.png");
    if (livesSurface3 == NULL)
    {
        printf("Error: lives2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->hearts[2] = SDL_CreateTextureFromSurface(renderer, livesSurface3);
    SDL_FreeSurface(livesSurface3);

    SDL_Surface *livesSurface4 = IMG_Load("graphics/lives3.png");
    if (livesSurface4 == NULL)
    {
        printf("Error: lives3.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->hearts[3] = SDL_CreateTextureFromSurface(renderer, livesSurface4);
    SDL_FreeSurface(livesSurface4);

    // play
    SDL_Surface *playSurface1 = IMG_Load("graphics/playtext1.png");
    if (playSurface1 == NULL)
    {
        printf("Error: playtext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->playText[0] = SDL_CreateTextureFromSurface(renderer, playSurface1);
    SDL_FreeSurface(playSurface1);

    SDL_Surface *playSurface2 = IMG_Load("graphics/playtext2.png");
    if (playSurface2 == NULL)
    {
        printf("Error: playtext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->playText[1] = SDL_CreateTextureFromSurface(renderer, playSurface2);
    SDL_FreeSurface(playSurface2);

    // manual
    SDL_Surface *manualSurface1 = IMG_Load("graphics/manualtext1.png");
    if (manualSurface1 == NULL)
    {
        printf("Error: manualtext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->manualText[0] = SDL_CreateTextureFromSurface(renderer, manualSurface1);
    SDL_FreeSurface(manualSurface1);

    SDL_Surface *manualSurface2 = IMG_Load("graphics/manualtext2.png");
    if (manualSurface2 == NULL)
    {
        printf("Error: manualtext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->manualText[1] = SDL_CreateTextureFromSurface(renderer, manualSurface2);
    SDL_FreeSurface(manualSurface2);

    // exit
    SDL_Surface *exitSurface1 = IMG_Load("graphics/exittext1.png");
    if (exitSurface1 == NULL)
    {
        printf("Error: exittext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->exitText[0] = SDL_CreateTextureFromSurface(renderer, exitSurface1);
    SDL_FreeSurface(exitSurface1);

    SDL_Surface *exitSurface2 = IMG_Load("graphics/exittext2.png");
    if (exitSurface2 == NULL)
    {
        printf("Error: exittext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->exitText[1] = SDL_CreateTextureFromSurface(renderer, exitSurface2);
    SDL_FreeSurface(exitSurface2);

    // main menu
    SDL_Surface *menuSurface1 = IMG_Load("graphics/mainmenutext1.png");
    if (menuSurface1 == NULL)
    {
        printf("Error: mainmenutext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->mainMenuText[0] = SDL_CreateTextureFromSurface(renderer, menuSurface1);
    SDL_FreeSurface(menuSurface1);

    SDL_Surface *menuSurface2 = IMG_Load("graphics/mainmenutext2.png");
    if (menuSurface2 == NULL)
    {
        printf("Error: mainmenutext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->mainMenuText[1] = SDL_CreateTextureFromSurface(renderer, menuSurface2);
    SDL_FreeSurface(menuSurface2);

    // play again
    SDL_Surface *playAgainSurface1 = IMG_Load("graphics/playagaintext1.png");
    if (playAgainSurface1 == NULL)
    {
        printf("Error: playagaintext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->playAgainText[0] = SDL_CreateTextureFromSurface(renderer, playAgainSurface1);
    SDL_FreeSurface(playAgainSurface1);

    SDL_Surface *playAgainSurface2 = IMG_Load("graphics/playagaintext2.png");
    if (playAgainSurface2 == NULL)
    {
        printf("Error: playagaintext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->playAgainText[1] = SDL_CreateTextureFromSurface(renderer, playAgainSurface2);
    SDL_FreeSurface(playAgainSurface2);

    // highscore
}

void renderGame(GameObjects *object, GameTextures *texture, SDL_Renderer *renderer)
{
    object->font = TTF_OpenFont("graphics/Minecraft.ttf", 200);

    // bg
    SDL_Rect rectBG = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, texture->background, NULL, &rectBG);

    // darken
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 250, 250, 250, 40);
    SDL_RenderDrawRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});
    SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});

    // man
    SDL_Rect rectMan = {object->man.x, object->man.y, object->man.w, object->man.h};
    SDL_RenderCopyEx(renderer, texture->character[object->man.frame], NULL, &rectMan, 0, NULL, !(object->man.right));

    // food
    SDL_Rect rectFood0 = {object->food[0].x, object->food[0].y, object->food[0].w, object->food[0].h};
    SDL_RenderCopy(renderer, texture->burger, NULL, &rectFood0);

    SDL_Rect rectFood1 = {object->food[1].x, object->food[1].y, object->food[1].w, object->food[1].h};
    SDL_RenderCopy(renderer, texture->chknball, NULL, &rectFood1);

    SDL_Rect rectFood2 = {object->food[2].x, object->food[2].y, object->food[2].w, object->food[2].h};
    SDL_RenderCopy(renderer, texture->dblcoffee, NULL, &rectFood2);

    SDL_Rect rectFood3 = {object->food[3].x, object->food[3].y, object->food[3].w, object->food[3].h};
    SDL_RenderCopy(renderer, texture->lemonade, NULL, &rectFood3);

    SDL_Rect rectFood4 = {object->food[4].x, object->food[4].y, object->food[4].w, object->food[4].h};
    SDL_RenderCopy(renderer, texture->samosa, NULL, &rectFood4);

    SDL_Rect rectFood5 = {object->food[5].x, object->food[5].y, object->food[5].w, object->food[5].h};
    SDL_RenderCopy(renderer, texture->chaa, NULL, &rectFood5);

    SDL_Rect rectFood6 = {object->food[6].x, object->food[6].y, object->food[6].w, object->food[6].h};
    SDL_RenderCopy(renderer, texture->sndwch, NULL, &rectFood6);

    SDL_Rect rectBomb1 = {object->food[7].x, object->food[7].y, object->food[7].w, object->food[7].h};
    SDL_RenderCopy(renderer, texture->bomb1[object->food[7].frame], NULL, &rectBomb1);

    SDL_Rect rectBomb2 = {object->food[8].x, object->food[8].y, object->food[8].w, object->food[8].h};
    SDL_RenderCopy(renderer, texture->bomb2[object->food[8].frame], NULL, &rectBomb2);

    SDL_Rect rectBomb3 = {object->food[9].x, object->food[9].y, object->food[9].w, object->food[9].h};
    SDL_RenderCopy(renderer, texture->bomb3[object->food[9].frame], NULL, &rectBomb3);

    // joshim
    SDL_Rect rectjoshim = {object->joshim.x, object->joshim.y, object->joshim.w, object->joshim.h};
    SDL_RenderCopyEx(renderer, texture->josh[object->joshim.frame], NULL, &rectjoshim, 0, NULL, (object->joshim.right == 0));

    // score display
    char str[100001];

    object->scoreSurface = TTF_RenderText_Solid(object->font, "BILL: ", (SDL_Color){0, 0, 0, 255});
    if (object->scoreSurface == NULL)
    {
        printf("Error: score textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->scoreText[0] = SDL_CreateTextureFromSurface(renderer, object->scoreSurface);
    SDL_FreeSurface(object->scoreSurface);
    SDL_RenderCopy(renderer, texture->scoreText[0], NULL, &(SDL_Rect){15, 12, 10 * strlen("BILL: "), 23});

    object->scoreSurface = TTF_RenderText_Solid(object->font, itoa(score, str, 10), (SDL_Color){0, 0, 0, 255});
    texture->scoreText[1] = SDL_CreateTextureFromSurface(renderer, object->scoreSurface);
    SDL_FreeSurface(object->scoreSurface);
    SDL_RenderCopy(renderer, texture->scoreText[1], NULL, &(SDL_Rect){75, 12, 14 * strlen(str), 23});

    // lives display
    SDL_Surface *surface = TTF_RenderText_Solid(object->font, "LIVES: ", (SDL_Color){0, 0, 0, 255});
    if (surface == NULL)
    {
        printf("Error: score livesSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->livesText = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture->livesText, NULL, &(SDL_Rect){WIDTH - 175, 12, 11 * strlen("LIVES: "), 23});

    SDL_Rect rectHeart = {WIDTH - 95, 11, 76, 19};
    SDL_RenderCopy(renderer, texture->hearts[lives], NULL, &rectHeart);

    SDL_DestroyTexture(texture->livesText);

    SDL_DestroyTexture(texture->scoreText[0]);
    SDL_DestroyTexture(texture->scoreText[1]);

    SDL_RenderPresent(renderer);
}

void renderMenu(GameObjects *object, GameTextures *texture, SDL_Renderer *renderer)
{
    object->font = TTF_OpenFont("graphics/Arcadepix Plus.ttf", 200);

    SDL_Rect rectMain = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, texture->mainScreen, NULL, &rectMain);

    // play text
    SDL_Rect rectPlay = {110, HEIGHT - 175, 88, 23};
    SDL_RenderCopy(renderer, texture->playText[texture->hover[0]], NULL, &rectPlay);

    // manual text
    SDL_Rect rectManual = {(WIDTH / 2) - 70, HEIGHT - 175, 121, 23};
    SDL_RenderCopy(renderer, texture->manualText[texture->hover[1]], NULL, &rectManual);

    // options text
    SDL_Rect rectExit = {WIDTH - 110 - 142, HEIGHT - 175, 142, 23};
    SDL_RenderCopy(renderer, texture->exitText[texture->hover[2]], NULL, &rectExit);

    // highscore text
    char str[100001];

    object->highScoreSurface = TTF_RenderText_Blended(object->font, itoa(highScore, str, 10), (SDL_Color){251, 249, 241, 255});
    if (object->highScoreSurface == NULL)
    {
        printf("Error: highscore textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->highScoretext[1] = SDL_CreateTextureFromSurface(renderer, object->highScoreSurface);
    SDL_FreeSurface(object->highScoreSurface);
    SDL_RenderCopy(renderer, texture->highScoretext[1], NULL, &(SDL_Rect){((WIDTH - 13 * (strlen(str) - strlen("SCORE: "))) / 2), HEIGHT - 95, 13 * strlen(str), 21});

    object->highScoreSurface = TTF_RenderText_Blended(object->font, "SCORE: ", (SDL_Color){251, 249, 241, 255});
    texture->highScoretext[0] = SDL_CreateTextureFromSurface(renderer, object->highScoreSurface);
    SDL_FreeSurface(object->highScoreSurface);
    SDL_RenderCopy(renderer, texture->highScoretext[0], NULL, &(SDL_Rect){(WIDTH - 13 * (strlen(str) + strlen("SCORE: "))) / 2, HEIGHT - 95, 13 * strlen("SCORE: "), 21});

    SDL_DestroyTexture(texture->highScoretext[0]);
    SDL_DestroyTexture(texture->highScoretext[1]);

    SDL_RenderPresent(renderer);
}

void renderManual(GameTextures *texture, SDL_Renderer *renderer)
{
    SDL_Rect rectManual = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, texture->manualPage, NULL, &rectManual);

    // menu button
    SDL_Rect rectMenu = {WIDTH - 25 - 107, HEIGHT - 45, 107, 25};
    SDL_RenderCopy(renderer, texture->manualMenuText[texture->hover[5]], NULL, &rectMenu);

    SDL_RenderPresent(renderer);
}

void renderOptions(GameTextures *texture, SDL_Renderer *renderer)
{
    SDL_Rect rectOptions = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, texture->optionsPage, NULL, &rectOptions);

    // reset button
    SDL_Rect rectReset = {(WIDTH - 230) / 2, HEIGHT - 120, 230, 27};
    SDL_RenderCopy(renderer, texture->resetText[texture->hover[6]], NULL, &rectReset);

    // diff1 button
    SDL_Rect rectDiff1 = {150, HEIGHT / 2 - 20, 85, 85};
    SDL_RenderCopy(renderer, texture->diff1[texture->hover[7]], NULL, &rectDiff1);

    // diff2 button
    SDL_Rect rectDiff2 = {(WIDTH - 85) / 2, HEIGHT / 2 - 20, 85, 85};
    SDL_RenderCopy(renderer, texture->diff2[texture->hover[8]], NULL, &rectDiff2);

    // diff3 button
    SDL_Rect rectDiff3 = {WIDTH - 85 - 150, HEIGHT / 2 - 20, 85, 85};
    SDL_RenderCopy(renderer, texture->diff3[texture->hover[9]], NULL, &rectDiff3);

    // menu button
    SDL_Rect rectMenu = {WIDTH - 25 - 107, HEIGHT - 45, 107, 25};
    SDL_RenderCopy(renderer, texture->manualMenuText[texture->hover[5]], NULL, &rectMenu);

    SDL_RenderPresent(renderer);
}

void printReceipt(GameObjects *object, GameTextures *texture, SDL_Renderer *renderer)
{
    int i, j;

    char str[100001];

    object->font = TTF_OpenFont("graphics/Minecraft.ttf", 200);

    // bg
    SDL_Rect rectBG = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, texture->background, NULL, &rectBG);

    // darken
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
    SDL_RenderDrawRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});
    SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});

    SDL_Rect rectReceipt = {(WIDTH - 377) / 2, 0, 377, HEIGHT};
    SDL_RenderCopy(renderer, texture->receiptbg, NULL, &rectReceipt);

    for (i = 0; i < 9; i++)
    {
        // quantity of each item
        object->textSurface = TTF_RenderText_Solid(object->font, itoa(item[i], str, 10), (SDL_Color){46, 46, 46, 255});
        if (object->textSurface == NULL)
        {
            printf("Error: itemNo textSurface not found.\n");
            SDL_Quit();
            exit(1);
        }
        texture->itemNo[i] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
        SDL_FreeSurface(object->textSurface);
        SDL_RenderCopy(renderer, texture->itemNo[i], NULL, &(SDL_Rect){220, 125 + (i * 28), 9 * strlen(str), 16});

        // name of item
        object->textSurface = TTF_RenderText_Solid(object->font, itemNames[i], (SDL_Color){46, 46, 46, 255});
        if (object->textSurface == NULL)
        {
            printf("Error: itemLabels textSurface not found.\n");
            SDL_Quit();
            exit(1);
        }
        texture->itemLabels[i] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
        SDL_FreeSurface(object->textSurface);
        SDL_RenderCopy(renderer, texture->itemLabels[i], NULL, &(SDL_Rect){265, 125 + (i * 28), 9 * strlen(itemNames[i]), 16});

        // tk or lives
        if (i == 7)
        {
            object->textSurface = TTF_RenderText_Solid(object->font, "lives", (SDL_Color){46, 46, 46, 255});
            if (object->textSurface == NULL)
            {
                printf("Error: taka textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            texture->taka = SDL_CreateTextureFromSurface(renderer, object->textSurface);
            SDL_FreeSurface(object->textSurface);
            SDL_RenderCopy(renderer, texture->taka, NULL, &(SDL_Rect){425, 125 + (i * 28), 35, 16});
        }
        else
        {
            object->textSurface = TTF_RenderText_Solid(object->font, "tk", (SDL_Color){46, 46, 46, 255});
            if (object->textSurface == NULL)
            {
                printf("Error: taka textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            texture->taka = SDL_CreateTextureFromSurface(renderer, object->textSurface);
            SDL_FreeSurface(object->textSurface);
            SDL_RenderCopy(renderer, texture->taka, NULL, &(SDL_Rect){425, 125 + (i * 28), 14, 16});
        }

        // total per item
        if (i < 7)
        {
            object->textSurface = TTF_RenderText_Solid(object->font, itoa(prices[i], str, 10), (SDL_Color){46, 46, 46, 255});
            if (object->textSurface == NULL)
            {
                printf("Error: itemTotal textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            texture->itemTotal[i] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
            SDL_FreeSurface(object->textSurface);
            SDL_RenderCopy(renderer, texture->itemTotal[i], NULL, &(SDL_Rect){475, 125 + (i * 28), 9 * strlen(str), 16});
        }
        else if (i == 7)
        {
            object->textSurface = TTF_RenderText_Solid(object->font, "3", (SDL_Color){46, 46, 46, 255});
            if (object->textSurface == NULL)
            {
                printf("Error: itemTotal textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            texture->itemTotal[i] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
            SDL_FreeSurface(object->textSurface);
            SDL_RenderCopy(renderer, texture->itemTotal[i], NULL, &(SDL_Rect){475, 125 + (i * 28), 9, 16});
        }
        else if (i == 8)
        {
            object->textSurface = TTF_RenderText_Solid(object->font, itoa(-69, str, 10), (SDL_Color){46, 46, 46, 255});
            if (object->textSurface == NULL)
            {
                printf("Error: itemTotal textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            texture->itemTotal[i] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
            SDL_FreeSurface(object->textSurface);
            SDL_RenderCopy(renderer, texture->itemTotal[i], NULL, &(SDL_Rect){475, 125 + (i * 28), 8.5 * strlen(str), 16});
        }
    }

    // total price
    object->textSurface = TTF_RenderText_Solid(object->font, "TOTAL:", (SDL_Color){46, 46, 46, 255});
    if (object->textSurface == NULL)
    {
        printf("Error: textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->priceTotal[0] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
    SDL_FreeSurface(object->textSurface);
    SDL_RenderCopy(renderer, texture->priceTotal[0], NULL, &(SDL_Rect){220, 160 + (i * 28), 8.5 * strlen("TOTAL: "), 16});

    object->textSurface = TTF_RenderText_Solid(object->font, "tk", (SDL_Color){46, 46, 46, 255});
    if (object->textSurface == NULL)
    {
        printf("Error: textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->priceTotal[1] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
    SDL_FreeSurface(object->textSurface);
    SDL_RenderCopy(renderer, texture->priceTotal[1], NULL, &(SDL_Rect){425, 160 + (i * 28), 14, 16});

    object->textSurface = TTF_RenderText_Solid(object->font, itoa(score, str, 10), (SDL_Color){46, 46, 46, 255});
    if (object->textSurface == NULL)
    {
        printf("Error: textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->priceTotal[2] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
    SDL_FreeSurface(object->textSurface);
    SDL_RenderCopy(renderer, texture->priceTotal[2], NULL, &(SDL_Rect){WIDTH - 220 - 9 * strlen(str), 160 + (i * 28), 9 * strlen(str), 16});

    // play again text
    SDL_Rect rectPlay = {220, HEIGHT - 95, 115, 20};
    SDL_RenderCopy(renderer, texture->playAgainText[texture->hover[3]], NULL, &rectPlay);

    // main menu text
    SDL_Rect rectMenu = {WIDTH - 220 - 90, HEIGHT - 95, 94, 20};
    SDL_RenderCopy(renderer, texture->mainMenuText[texture->hover[4]], NULL, &rectMenu);

    destroyTextTextures(texture);

    SDL_RenderPresent(renderer);
}

// minigame
void minigameInitialize(SDL_Renderer *renderer, MinGameObjects *game)
{
    game->time = 0;

    int xposition;
    srand(time(0));
    xposition = rand() % (630 + 1 - 10) + 10;

    game->minplat.w = 200;
    game->minplat.h = 40;
    game->minplat.c = xposition;
    game->minplat.d = HEIGHT - game->minplat.h;
    game->minplat.move = 5;
    game->minplat.size = 15; // size decrease after collision

    game->mincat.w = 100;
    game->mincat.h = 80;
    game->mincat.a = xposition;
    game->mincat.b = game->minplat.d - game->mincat.h;
    game->mincat.yspeed = 2.5;
    game->mincat.xspeed = 1.5;
    game->mincat.right = true;
    game->mincat.frame = 1;

    game->tar.ee[0] = 10;
    game->tar.ff[0] = 10;
    game->tar.targw = 100;
    game->tar.targh = 55;
    game->tar.targnum = 35;

    for (int i = 1; i < game->tar.targnum; i++)
    {
        if (game->tar.ee[i - 1] == (WIDTH - game->tar.targw - 10)) // 10 na dile ekdom 0,0 theke start hobe
        {
            game->tar.ff[i] = game->tar.ff[i - 1] + game->tar.targh;
            game->tar.ee[i] = 10;
        }
        else
        {
            game->tar.ee[i] = game->tar.ee[i - 1] + game->tar.targw;
            game->tar.ff[i] = game->tar.ff[i - 1] + 0;
        }
    }
    srand(time(0));
    game->tar.j = rand() % ((game->tar.targnum - 1) + 1 - 0) + 0;
}

void minigameProcess(MinGameObjects *game, GameObjects *object)
{
    game->time++;

    game->mincat.b -= game->mincat.yspeed;
    game->mincat.a += game->mincat.xspeed;

    // movement controls
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if ((state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) && game->minplat.c >= 0)
    {
        game->minplat.c -= game->minplat.move;
    }
    else if ((state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) && game->minplat.c <= WIDTH - game->minplat.w)
    {
        game->minplat.c += game->minplat.move;
    }
    else
    {
        // frames
        if (game->time % 35 == 0)
        {
            if (game->mincat.frame == 0)
                game->mincat.frame = 1;
            else
                game->mincat.frame = 0;
        }
    }

    // facing right
    if (game->mincat.xspeed > 0)
        game->mincat.right = true;
    else
        game->mincat.right = false;

    // bounce back
    // left
    if (game->mincat.a <= 0)
    {
        game->mincat.xspeed = -game->mincat.xspeed;
    }
    // right
    if (game->mincat.a >= (WIDTH - game->mincat.w))
    {
        game->mincat.xspeed = -game->mincat.xspeed;
    }
    // up
    if (game->mincat.b <= 0)
    {
        game->mincat.yspeed = -game->mincat.yspeed;
    }

    // bottom screen collision
    if (game->mincat.b >= (HEIGHT - game->mincat.h))
    {
        game->mincat.xspeed = 0;
        game->mincat.yspeed = 0;
        game->mincat.frame = 1;

        score -= 69;
        item[8]++;
        printf("YOU HURT JOSHIM! -69tk!\n");

        Mix_HaltMusic();
        Mix_PlayChannel(-1, object->soundBytes[1], 0);
        SDL_Delay(750);
        Mix_PlayMusic(object->gameMusic, -1);
        page = PAGE_STATUS_GAME;
        // printf("MAIN GAME\n");
        //  SDL_Quit();
    }

    // platform collision
    if (game->mincat.b == (game->minplat.d - game->mincat.h) && game->mincat.a >= (game->minplat.c - game->mincat.w) && game->mincat.a <= (game->minplat.c + game->minplat.w))
    {
        Mix_PlayChannel(-1, object->soundBytes[3], 0);
        game->mincat.yspeed = -game->mincat.yspeed;
        game->minplat.w -= game->minplat.size;
    }

    // block collisions
    for (int i = 0; i < game->tar.targnum; i++)
    {
        int k = game->tar.j;

        if (game->mincat.a <= (game->tar.ee[i] + game->tar.targw) && game->tar.ee[i] <= (game->mincat.a + game->mincat.w) && game->mincat.b <= (game->tar.ff[i] + game->tar.targh) && game->tar.ff[i] <= (game->mincat.b + game->mincat.h))
        {
            game->mincat.xspeed = -game->mincat.xspeed;
            game->mincat.yspeed = -game->mincat.yspeed;
            game->tar.ee[i] = 569;
            game->tar.ff[i] = 769;
        }
        else if (game->mincat.a <= (game->tar.ee[k] + game->tar.targw) && game->tar.ee[k] <= (game->mincat.a + game->mincat.w) && game->mincat.b <= (game->tar.ff[k] + game->tar.targh) && game->tar.ff[k] <= (game->mincat.b + game->mincat.h))
        {
            game->tar.ee[k] = 569;
            game->tar.ff[k] = 769;
            game->mincat.xspeed = 0;
            game->mincat.yspeed = 0;

            Mix_HaltMusic();
            Mix_PlayChannel(-1, object->soundBytes[0], 0);
            SDL_Delay(750);

            if (k % 2)
            {
                if (lives < 3)
                    lives++;
                printf("BONUS! +1 LIFE\n");
                // printf("\nUIII");
            }
            else
            {
                score += 150;
                printf("BONUS! +150tk\n");

                // printf("\nyee");
            }

            Mix_PlayMusic(object->gameMusic, -1);
            page = PAGE_STATUS_GAME;
        }
    }
}

void renderMinigameSurface(SDL_Renderer *renderer, MinGameObjects *game)
{
    SDL_SetRenderDrawColor(renderer, 243, 207, 198, 0);

    SDL_RenderClear(renderer);

    // background
    SDL_Rect rectBack = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, game->back, NULL, &rectBack);

    // darken
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 250, 250, 250, 20);
    SDL_RenderDrawRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});
    SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});

    SDL_Rect rectCat = {game->mincat.a, game->mincat.b, game->mincat.w, game->mincat.h};
    SDL_RenderCopyEx(renderer, game->cat[game->mincat.frame], NULL, &rectCat, 0, NULL, game->mincat.right);

    SDL_Rect rectPlat = {game->minplat.c, game->minplat.d, game->minplat.w, game->minplat.h};
    SDL_RenderCopy(renderer, game->plat, NULL, &rectPlat);

    for (int i = 0; i < game->tar.targnum; i++)
    {
        int k = game->tar.j;
        if (i == k && k % 2)
        {
            SDL_Rect rectpanjim = {game->tar.ee[i], game->tar.ff[i], game->tar.targw, game->tar.targh};
            SDL_RenderCopy(renderer, game->panjim, NULL, &rectpanjim);
            continue;
        }
        else if (i == k && !(k % 2))
        {
            SDL_Rect rectheart = {game->tar.ee[i], game->tar.ff[k], game->tar.targw, game->tar.targh};
            SDL_RenderCopy(renderer, game->heart, NULL, &rectheart);
            continue;
        }
        else
        {
            SDL_Rect recttar = {game->tar.ee[i], game->tar.ff[i], game->tar.targw, game->tar.targh};
            SDL_RenderCopy(renderer, game->targ, NULL, &recttar);
        }
    }

    SDL_RenderPresent(renderer);
}

void loadMinigameSurfaces(SDL_Renderer *renderer, MinGameObjects *game)
{
    // joshim
    SDL_Surface *catSurface1 = IMG_Load("graphics/joshim3.png");
    if (catSurface1 == NULL)
    {
        printf("Error: joshim3.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    game->cat[0] = SDL_CreateTextureFromSurface(renderer, catSurface1);
    SDL_FreeSurface(catSurface1);

    SDL_Surface *catSurface2 = IMG_Load("graphics/joshim4.png");
    if (catSurface2 == NULL)
    {
        printf("Error: joshim4.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    game->cat[1] = SDL_CreateTextureFromSurface(renderer, catSurface2);
    SDL_FreeSurface(catSurface2);

    // platform
    SDL_Surface *platSurface = IMG_Load("graphics/plat.png");
    if (platSurface == NULL)
    {
        printf("Error: plat.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    game->plat = SDL_CreateTextureFromSurface(renderer, platSurface);
    SDL_FreeSurface(platSurface);

    // blocks
    SDL_Surface *blockSurface = IMG_Load("graphics/targ.png");
    if (blockSurface == NULL)
    {
        printf("Error: targ.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    game->targ = SDL_CreateTextureFromSurface(renderer, blockSurface);
    SDL_FreeSurface(blockSurface);

    // background
    SDL_Surface *backSurface = IMG_Load("graphics/miniBackground.png");
    if (backSurface == NULL)
    {
        printf("Error: backgr.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    game->back = SDL_CreateTextureFromSurface(renderer, backSurface);
    SDL_FreeSurface(backSurface);

    // ...panjim?
    SDL_Surface *panjimSurface = IMG_Load("graphics/heart.png");
    if (panjimSurface == NULL)
    {
        printf("Error: heart.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    game->panjim = SDL_CreateTextureFromSurface(renderer, panjimSurface);
    SDL_FreeSurface(panjimSurface);

    // heart
    SDL_Surface *heartSurface = IMG_Load("graphics/meal.png");
    if (heartSurface == NULL)
    {
        printf("Error: heart.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    game->heart = SDL_CreateTextureFromSurface(renderer, heartSurface);
    SDL_FreeSurface(heartSurface);
}

void destroyMinigameTextures(MinGameObjects *game)
{
    SDL_DestroyTexture(game->cat[0]);
    SDL_DestroyTexture(game->cat[1]);
    SDL_DestroyTexture(game->panjim);
    SDL_DestroyTexture(game->targ);
    SDL_DestroyTexture(game->plat);
    SDL_DestroyTexture(game->back);
    SDL_DestroyTexture(game->heart);
}

void playMinigame(SDL_Renderer *renderer, MinGameObjects *game, GameObjects *object)
{
    renderMinigameSurface(renderer, game);
    minigameProcess(game, object);
}

// game
bool quitCheck(SDL_Event *event)
{
    switch (event->type)
    {
    case SDL_QUIT:
        return true;
        break;
    case SDL_KEYDOWN:
        if (event->key.keysym.sym == SDLK_ESCAPE)
        {
            return true;
            break;
        }
    }
    return false;
}

/* void makeFullScreen(SDL_Event *event, SDL_Window *window)
{
    if (event->type == SDL_KEYDOWN)
    {
        if (event->key.keysym.sym == SDLK_f)
        {
            // Toggle fullscreen mode
            if (fullScreen)
            {
                SDL_SetWindowFullscreen(window, 0); // Exit fullscreen
            }
            else
            {
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP); // Enter fullscreen
            }
            fullScreen = !fullScreen;
        }
    }
}*/

void gameInitialise(GameObjects *object)
{
    Mix_PlayMusic(object->gameMusic, -1);
    loadMan(object);
    loadJoshim(object);
    for (int i = 0; i < 9; i++)
    {
        item[i] = 0;
    }
    for (int j = 0; j < 10; j++)
    {
        loadFood(object, j);
    }
    score = 0;
    lives = 3;
    object->time = 0;
}

void playGame(GameObjects *object, GameTextures *texture, MinGameObjects *miniObject, SDL_Renderer *renderer, FILE *file)
{
    renderGame(object, texture, renderer);

    manProcess(object);
    foodProcess(object);
    joshimProcess(object);

    // game over
    if (!lives)
    {
        Mix_HaltMusic();
        Mix_PlayChannel(-1, object->soundBytes[4], 0);

        if (score > highScore)
        {
            highScore = score;
            fclose(file);
            file = fopen("highscore.txt", "w");
            fprintf(file, "%ld", highScore);
        }

        printf("GAME OVER!\nTOTAL BILL:%12s %ld\n", "tk", score);
        printf("HIGHSCORE: %ld\n", highScore);

        page = PAGE_STATUS_OVER;
    }

    // joshim
    int c = (rand() % 2);
    if (!(object->time % 1750))
    {
        if (c == 1)
            joshimLeft(object);
        else
            joshimRight(object);
    }
    if (object->joshim.screen)
    {
        manJump(object);

        if (joshimCollision(object, object))
        {
            Mix_PlayChannel(-1, object->soundBytes[1], 0);
            loadJoshim(object);

            SDL_Delay(750);

            // minigame
            Mix_HaltMusic();
            Mix_PlayMusic(object->minigameMusic, -1);
            minigameInitialize(renderer, miniObject);
            page = PAGE_STATUS_MINIGAME;
        }
    }

    int r = (rand() % difficulty);
    if (!(object->time % 8) && (object->food[r].y >= HEIGHT || object->food[r].y < -(object->food[r].h)))
    {
        dropFood(object, r);
    }
    for (int i = 0; i < difficulty; i++)
    {
        if (foodCollision(object, object, i))
        {
            if (i > 6)
            {
                Mix_PlayChannel(-1, object->soundBytes[2], 0);
                lives--;
                item[7]++;
                printf("BOMB! LIVES: %d\n", lives);
                // SDL_Delay(250);
            }
            else
            {
                Mix_PlayChannel(-1, object->soundBytes[0], 0);
                score += prices[i];
                item[i]++;
                printf("1 %-15s    tk %d\n", itemNames[i], prices[i]);
            }
            loadFood(object, i);
        }
    }
}

void renderPage(PageStatus page, GameObjects *object, GameTextures *texture, MinGameObjects *miniObject, SDL_Renderer *renderer, SDL_Event *event, FILE *file)
{
    switch (page)
    {
    case PAGE_STATUS_MENU:
        renderMenu(object, texture, renderer);
        break;
    case PAGE_STATUS_GAME:
        playGame(object, texture, miniObject, renderer, file);
        break;
    case PAGE_STATUS_MINIGAME:
        playMinigame(renderer, miniObject, object);
        break;
    case PAGE_STATUS_MANUAL:
        renderManual(texture, renderer);
        break;
    case PAGE_STATUS_OPTIONS:
        renderOptions(texture, renderer);
        break;
    case PAGE_STATUS_OVER:
        printReceipt(object, texture, renderer);
        break;
    }
}

void navigator(GameObjects *object, GameTextures *texture, SDL_Event *event, FILE *file)
{
    if (page == PAGE_STATUS_MENU)
    {
        // play button
        if (event->button.x >= 110 && event->button.x <= 198 && event->button.y >= HEIGHT - 175 && event->button.y <= HEIGHT - 175 + 19)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
            {
                gameInitialise(object);
                page = PAGE_STATUS_GAME;
            }
            else
                texture->hover[0] = 1;
        }
        else
            texture->hover[0] = 0;

        // manual button
        if (event->button.x >= (WIDTH / 2) - 70 && event->button.x <= (WIDTH / 2) - 70 + 121 && event->button.y >= HEIGHT - 175 && event->button.y <= HEIGHT - 175 + 19)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
                page = PAGE_STATUS_MANUAL;
            else
                texture->hover[1] = 1;
        }
        else
            texture->hover[1] = 0;

        // options button
        if (event->button.x >= WIDTH - 110 - 142 && event->button.x <= WIDTH - 110 && event->button.y >= HEIGHT - 175 && event->button.y <= HEIGHT - 175 + 19)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
                page = PAGE_STATUS_OPTIONS;
            else
                texture->hover[2] = 1;
        }
        else
            texture->hover[2] = 0;
    }

    if (page == PAGE_STATUS_MANUAL)
    {
        //  menu button
        if (event->button.x >= WIDTH - 25 - 107 && event->button.x <= WIDTH - 25 && event->button.y >= HEIGHT - 45 && event->button.y <= HEIGHT - 45 + 25)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
                page = PAGE_STATUS_MENU;
            else
                texture->hover[5] = 1;
        }
        else
            texture->hover[5] = 0;
    }

    if (page == PAGE_STATUS_OPTIONS)
    {
        //  menu button
        if (event->button.x >= WIDTH - 25 - 107 && event->button.x <= WIDTH - 25 && event->button.y >= HEIGHT - 45 && event->button.y <= HEIGHT - 45 + 25)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
                page = PAGE_STATUS_MENU;
            else
                texture->hover[5] = 1;
        }
        else
            texture->hover[5] = 0;

        // reset
        if (event->button.x >= (WIDTH - 230) / 2 && event->button.x <= (WIDTH - 230) / 2 + 230 && event->button.y >= HEIGHT - 120 && event->button.y <= HEIGHT - 120 + 27)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
            {
                fclose(file);
                highScore = 0;
                file = fopen("highscore.txt", "w");
                fprintf(file, "%d", highScore);
                printf("GAME HAS BEEN RESET! HIGHSCORE: %d\n", highScore);
            }
            else
                texture->hover[6] = 1;
        }
        else
            texture->hover[6] = 0;

        //  diff1 button
        if (event->button.x >= 150 && event->button.x <= 150 + 85 && event->button.y >= (HEIGHT / 2) - 20 && event->button.y <= (HEIGHT / 2) - 20 + 85)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
            {
                difficulty = 8;
                printf("DIFFICULTY: EASY\n");
            }
            else
                texture->hover[7] = 1;
        }
        else
            texture->hover[7] = 0;

        //  diff2 button
        if (event->button.x >= (WIDTH - 85) / 2 && event->button.x <= (WIDTH - 85) / 2 + 85 && event->button.y >= (HEIGHT / 2) - 20 && event->button.y <= (HEIGHT / 2) - 20 + 85)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
            {
                difficulty = 9;
                printf("DIFFICULTY: MEDIUM\n");
            }
            else
                texture->hover[8] = 1;
        }
        else
            texture->hover[8] = 0;

        //  diff3 button
        if (event->button.x >= WIDTH - 85 - 150 && event->button.x <= WIDTH - 150 && event->button.y >= (HEIGHT / 2) - 20 && event->button.y <= (HEIGHT / 2) - 20 + 85)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
            {
                difficulty = 10;
                printf("DIFFICULTY: HARD\n");
            }
            else
                texture->hover[9] = 1;
        }
        else
            texture->hover[9] = 0;
    }

    if (page == PAGE_STATUS_OVER)
    {
        // replay button
        if (event->button.x >= 220 && event->button.x <= 335 && event->button.y >= HEIGHT - 95 && event->button.y <= HEIGHT - 75)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
            {
                gameInitialise(object);
                page = PAGE_STATUS_GAME;
            }
            else
                texture->hover[3] = 1;
        }
        else
            texture->hover[3] = 0;

        // menu button
        if (event->button.x >= WIDTH - 220 - 90 && event->button.x <= WIDTH - 220 && event->button.y >= HEIGHT - 95 && event->button.y <= HEIGHT - 75)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
            {
                Mix_PlayMusic(object->menuMusic, -1);
                page = PAGE_STATUS_MENU;
            }
            else
                texture->hover[4] = 1;
        }
        else
            texture->hover[4] = 0;
    }
}

void endGame(GameObjects *object, GameTextures *texture, MinGameObjects *miniObject, SDL_Renderer *renderer, SDL_Window *window, FILE *file)
{
    fclose(file);
    destroyTextures(texture);
    destroyMinigameTextures(miniObject);
    destroyMusic(object);
    TTF_CloseFont(object->font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

// main
int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("✧ CDSMania ✧", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    GameObjects object;
    GameTextures texture;
    loadMusic(&object);
    loadSurfaces(&object, &texture, renderer);
    difficulty = 8;

    MinGameObjects miniObject;
    loadMinigameSurfaces(renderer, &miniObject);

    FILE *file = fopen("highscore.txt", "r");
    fscanf(file, "%ld", &highScore);

    Mix_PlayMusic(object.menuMusic, -1);
    page = PAGE_STATUS_MENU;

    // main game loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;

        renderPage(page, &object, &texture, &miniObject, renderer, &event, file);

        if (SDL_PollEvent(&event))
        {
            quit = quitCheck(&event);

            navigator(&object, &texture, &event, file);

            // makeFullScreen(&event, window);
        }
    }

    endGame(&object, &texture, &miniObject, renderer, window, file);
    SDL_Quit();

    return 0;
}
