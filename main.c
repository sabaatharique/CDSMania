#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

const int WIDTH = 720, HEIGHT = 540;

const float GRAVITY = 0.325;

long int score, highScore;
int lives;
int prices[7] = {70, 25, 40, 25, 10, 10, 50};
int item[9] = {0}, totalPerItem[7] = {0};
const char itemNames[9][101] = {"x Burger", "x Chicken Ball", "x Double Coffee", "x Lemonade", "x Samosa", "x Chaa", "x Sandwich", "x Bombs", "x Joshims hurt"};

// struct galore
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
    float x, y;

} Platform;

typedef struct
{
    Man man;
    Things food[8];
    Cat joshim;

    unsigned int time;

    TTF_Font *font;

    Mix_Music *menuMusic, *gameMusic;
    Mix_Chunk *soundBytes[5];

    SDL_Surface *scoreSurface, *livesSurface, *textSurface, *highScoreSurface;

    SDL_Renderer *renderer;

} gameObjects;

typedef struct
{
    SDL_Texture *character[2];

    SDL_Texture *burger;
    SDL_Texture *chknball;
    SDL_Texture *dblcoffee;
    SDL_Texture *lemonade;
    SDL_Texture *samosa;
    SDL_Texture *chaa;
    SDL_Texture *sndwch;

    SDL_Texture *bomb[2];

    SDL_Texture *josh[2];

    SDL_Texture *hearts[4];

    SDL_Texture *mainScreen;
    SDL_Texture *background;

    SDL_Texture *itemNo[9], *itemLabels[9], *itemTotal[9], *priceTotal[3];
    SDL_Texture *receiptbg, *taka;
    SDL_Texture *scoreText[2], *livesText;

    SDL_Texture *playText[2], *manualText[2], *exitText[2];
    SDL_Texture *highScoretext[2];
    SDL_Texture *mainMenuText[2], *playAgainText[2];
    int hover[5];

} gameTextures;

typedef struct
{
    Cat joshim;
    Platform ledges[10];

} miniObjects;

typedef enum
{
    PAGE_STATUS_MENU,
    PAGE_STATUS_GAME,
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
7 = bomb
*/

// player
void loadMan(gameObjects *object)
{
    // man
    object->man.h = 142;
    object->man.w = 85;
    object->man.dy = 0;
    object->man.dx = 3.25;
    object->man.up = false;
    object->man.x = ((WIDTH - object->man.w) / 2);
    object->man.y = HEIGHT - object->man.h;
    object->man.frame = 0;
    object->man.right = true;

    // time
    object->time = 0;
}

void manProcess(gameObjects *object)
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
        if (object->time % 15 == 0)
        {
            if (object->man.frame == 0)
                object->man.frame = 1;
            else
                object->man.frame = 0;
        }
    }

    // jump mechanics
    if (object->man.up)
    {
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

void manJump(gameObjects *object)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_w))
        {
            if (object->man.y > 0)
            {
                Mix_PlayChannel(-1, object->soundBytes[3], 0);
                object->man.dy = 2.5;
                object->man.y = HEIGHT - object->man.h - 185;
                object->man.up = true;
            }
        }
    }
}

// food
void loadFood(gameObjects *object, int i)
{
    if (i == 7)
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

void foodProcess(gameObjects *object)
{
    for (int i = 0; i < 8; i++)
    {
        if (object->food[i].screen)
        {
            object->food[i].y += object->food[i].dy;
        }
        if (object->food[i].y == HEIGHT)
        {
            object->food[i].screen = false;
        }
    }

    if (object->time % 15 == 0 && object->food[7].screen)
    {
        if (object->food[7].frame == 0)
            object->food[7].frame = 1;
        else
            object->food[7].frame = 0;
    }
}

bool foodCollision(gameObjects *obj1, gameObjects *obj2, int i)
{
    float x1 = obj1->man.x, y1 = obj1->man.y;
    float w1 = obj1->man.w, h1 = obj1->man.h;

    float x2 = obj2->food[i].x, y2 = obj2->food[i].y;
    float w2 = obj2->food[i].w, h2 = obj2->food[i].h;

    if (x1 < (x2 + w2) && x2 < (x1 + w1) && y1 < (y2 + h2) && y2 < (y1 + h1))
        return true;

    return false;
}

void dropFood(gameObjects *object, int i)
{
    object->food[i].screen = true;
    object->food[i].x = (rand() % (WIDTH - object->food[i].w));
    object->food[i].y = -(object->food[i].h);
}

// joshim
void loadJoshim(gameObjects *object)
{
    object->joshim.h = 75;
    object->joshim.w = 105;
    object->joshim.x = -(object->joshim.w);
    object->joshim.y = HEIGHT;
    object->joshim.frame = 0;
    object->joshim.screen = false;
}

void joshimProcess(gameObjects *object)
{
    if (object->joshim.screen)
    {
        object->joshim.x += object->joshim.dx;
        if (object->time % 20 == 0)
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

bool joshimCollision(gameObjects *obj1, gameObjects *obj2)
{
    float x1 = obj1->man.x, y1 = obj1->man.y;
    float w1 = obj1->man.w, h1 = obj1->man.h;

    float x2 = obj2->joshim.x, y2 = obj2->joshim.y;
    float w2 = obj2->joshim.w, h2 = obj2->joshim.h;

    if (x1 < (x2 + w2) && x2 < (x1 + w1) && y1 < (y2 + h2) && y2 < (y1 + h1))
        return true;

    return false;
}

void joshimLeft(gameObjects *object)
{
    object->joshim.x = -(object->joshim.w);
    object->joshim.y = HEIGHT - object->joshim.h;
    object->joshim.dx = 2;
    object->joshim.frame = 0;
    object->joshim.right = false;
    object->joshim.screen = true;
}

void joshimRight(gameObjects *object)
{
    object->joshim.x = WIDTH;
    object->joshim.y = HEIGHT - object->joshim.h;
    object->joshim.dx = -2;
    object->joshim.frame = 0;
    object->joshim.right = true;
    object->joshim.screen = true;
}

// music
void loadMusic(gameObjects *object)
{
    object->menuMusic = Mix_LoadMUS("Undertale OST - Hotel.mp3");
    object->gameMusic = Mix_LoadMUS("Undertale OST - Can You Really Call This A Hotel I Didn't Receive A Mint On My Pillow.mp3");

    object->soundBytes[0] = Mix_LoadWAV("eatSound.wav");
    object->soundBytes[1] = Mix_LoadWAV("joshimSound.wav");
    object->soundBytes[2] = Mix_LoadWAV("boomSound.wav");
    object->soundBytes[3] = Mix_LoadWAV("jumpSound.wav");
    object->soundBytes[4] = Mix_LoadWAV("gameoverSound.wav");
}

void destroyMusic(gameObjects *object)
{
    Mix_FreeMusic(object->gameMusic);
    Mix_FreeMusic(object->menuMusic);
    for (int i = 0; i < 5; i++)
    {
        Mix_FreeChunk(object->soundBytes[i]);
    }
    Mix_CloseAudio();
}

// visuals
void destroyTextures(gameTextures *texture)
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

    for (int j = 0; j < 2; j++)
    {
        SDL_DestroyTexture(texture->character[j]);
        SDL_DestroyTexture(texture->bomb[j]);
        SDL_DestroyTexture(texture->josh[j]);

        SDL_DestroyTexture(texture->playText[j]);
        SDL_DestroyTexture(texture->manualText[j]);
        SDL_DestroyTexture(texture->exitText[j]);

        SDL_DestroyTexture(texture->highScoretext[j]);

        SDL_DestroyTexture(texture->mainMenuText[j]);
        SDL_DestroyTexture(texture->playAgainText[j]);
    }

    for (int k = 0; k < 4; k++)
    {
        SDL_DestroyTexture(texture->hearts[k]);
    }

    SDL_DestroyTexture(texture->receiptbg);
}

void destroyTextTextures(gameTextures *texture)
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

void renderGame(gameObjects *object, gameTextures *texture, SDL_Renderer *renderer)
{
    object->font = TTF_OpenFont("Minecraft.ttf", 200);

    // bg
    SDL_Rect rectBG = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, texture->background, NULL, &rectBG);

    // darken
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 250, 250, 250, 70);
    SDL_RenderDrawRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});
    SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});

    // man
    SDL_Rect rectMan = {object->man.x, object->man.y, object->man.w, object->man.h};
    SDL_RenderCopyEx(renderer, texture->character[object->man.frame], NULL, &rectMan, 0, NULL, (object->man.right == 0));

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
    SDL_RenderCopy(renderer, texture->bomb[object->food[7].frame], NULL, &rectBomb1);

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
    object->livesSurface = TTF_RenderText_Solid(object->font, "LIVES: ", (SDL_Color){0, 0, 0, 255});
    if (object->livesSurface == NULL)
    {
        printf("Error: score livesSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->livesText = SDL_CreateTextureFromSurface(renderer, object->livesSurface);
    SDL_FreeSurface(object->livesSurface);
    SDL_RenderCopy(renderer, texture->livesText, NULL, &(SDL_Rect){WIDTH - 175, 12, 11 * strlen("LIVES: "), 23});

    SDL_Rect rectHeart = {WIDTH - 95, 11, 76, 19};
    SDL_RenderCopy(renderer, texture->hearts[lives], NULL, &rectHeart);

    SDL_DestroyTexture(texture->livesText);

    SDL_DestroyTexture(texture->scoreText[0]);
    SDL_DestroyTexture(texture->scoreText[1]);

    SDL_RenderPresent(renderer);
}

void renderMenu(gameObjects *object, gameTextures *texture, SDL_Renderer *renderer)
{
    object->font = TTF_OpenFont("Arcadepix Plus.ttf", 200);

    SDL_Rect rectMain = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, texture->mainScreen, NULL, &rectMain);

    // play text
    SDL_Rect rectPlay = {110, HEIGHT - 175, 88, 23};
    SDL_RenderCopy(renderer, texture->playText[texture->hover[0]], NULL, &rectPlay);

    // manual text
    SDL_Rect rectManual = {(WIDTH / 2) - 60, HEIGHT - 175, 121, 23};
    SDL_RenderCopy(renderer, texture->manualText[texture->hover[1]], NULL, &rectManual);

    // exit text
    SDL_Rect rectExit = {WIDTH - 110 - 92, HEIGHT - 175, 92, 23};
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

void loadSurfaces(gameObjects *object, gameTextures *texture, SDL_Renderer *renderer)
{
    // buttons
    for (int i = 0; i < 5; i++)
    {
        texture->hover[i] = 0;
    }

    // man
    SDL_Surface *charSurface1 = IMG_Load("character1.png");
    if (charSurface1 == NULL)
    {
        printf("Error: character1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->character[0] = SDL_CreateTextureFromSurface(renderer, charSurface1);
    SDL_FreeSurface(charSurface1);

    SDL_Surface *charSurface2 = IMG_Load("character2.png");
    if (charSurface2 == NULL)
    {
        printf("Error: character2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->character[1] = SDL_CreateTextureFromSurface(renderer, charSurface2);
    SDL_FreeSurface(charSurface2);

    // food
    SDL_Surface *burgerSurface = IMG_Load("burger.png");
    if (burgerSurface == NULL)
    {
        printf("Error: burger.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->burger = SDL_CreateTextureFromSurface(renderer, burgerSurface);
    SDL_FreeSurface(burgerSurface);

    SDL_Surface *chknballSurface = IMG_Load("chknball.png");
    if (chknballSurface == NULL)
    {
        printf("Error: chknball.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->chknball = SDL_CreateTextureFromSurface(renderer, chknballSurface);
    SDL_FreeSurface(chknballSurface);

    SDL_Surface *dblcoffeeSurface = IMG_Load("dblcoffee.png");
    if (dblcoffeeSurface == NULL)
    {
        printf("Error: dblcoffee.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->dblcoffee = SDL_CreateTextureFromSurface(renderer, dblcoffeeSurface);
    SDL_FreeSurface(dblcoffeeSurface);

    SDL_Surface *lemonadeSurface = IMG_Load("lemonade.png");
    if (lemonadeSurface == NULL)
    {
        printf("Error: lemonade.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->lemonade = SDL_CreateTextureFromSurface(renderer, lemonadeSurface);
    SDL_FreeSurface(lemonadeSurface);

    SDL_Surface *samosaSurface = IMG_Load("samosa.png");
    if (samosaSurface == NULL)
    {
        printf("Error: samosa.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->samosa = SDL_CreateTextureFromSurface(renderer, samosaSurface);
    SDL_FreeSurface(samosaSurface);

    SDL_Surface *chaaSurface = IMG_Load("chaa.png");
    if (chaaSurface == NULL)
    {
        printf("Error: chaa.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->chaa = SDL_CreateTextureFromSurface(renderer, chaaSurface);
    SDL_FreeSurface(chaaSurface);

    SDL_Surface *sndwchSurface = IMG_Load("sndwch.png");
    if (sndwchSurface == NULL)
    {
        printf("Error: sndwch.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->sndwch = SDL_CreateTextureFromSurface(renderer, sndwchSurface);
    SDL_FreeSurface(sndwchSurface);

    SDL_Surface *bombSurface1 = IMG_Load("bomb1.png");
    if (bombSurface1 == NULL)
    {
        printf("Error: bomb1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->bomb[0] = SDL_CreateTextureFromSurface(renderer, bombSurface1);
    SDL_FreeSurface(bombSurface1);

    SDL_Surface *bombSurface2 = IMG_Load("bomb2.png");
    if (bombSurface2 == NULL)
    {
        printf("Error: bomb2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->bomb[1] = SDL_CreateTextureFromSurface(renderer, bombSurface2);
    SDL_FreeSurface(bombSurface2);

    // background
    SDL_Surface *bgSurface = IMG_Load("background.png");
    if (bgSurface == NULL)
    {
        printf("Error: background.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->background = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    // screen
    SDL_Surface *mainSurface = IMG_Load("mainScreen.png");
    if (mainSurface == NULL)
    {
        printf("Error: mainScreen.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->mainScreen = SDL_CreateTextureFromSurface(renderer, mainSurface);
    SDL_FreeSurface(mainSurface);

    // joshim
    SDL_Surface *joshimSurface1 = IMG_Load("joshim1.png");
    if (joshimSurface1 == NULL)
    {
        printf("Error: joshim1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->josh[0] = SDL_CreateTextureFromSurface(renderer, joshimSurface1);
    SDL_FreeSurface(joshimSurface1);

    SDL_Surface *joshimSurface2 = IMG_Load("joshim2.png");
    if (joshimSurface2 == NULL)
    {
        printf("Error: joshim2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->josh[1] = SDL_CreateTextureFromSurface(renderer, joshimSurface2);
    SDL_FreeSurface(joshimSurface2);

    // receipt
    SDL_Surface *receiptSurface = IMG_Load("receipt.png");
    if (receiptSurface == NULL)
    {
        printf("Error: receipt.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->receiptbg = SDL_CreateTextureFromSurface(renderer, receiptSurface);
    SDL_FreeSurface(receiptSurface);

    // hearts
    SDL_Surface *livesSurface1 = IMG_Load("lives0.png");
    if (livesSurface1 == NULL)
    {
        printf("Error: lives0.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->hearts[0] = SDL_CreateTextureFromSurface(renderer, livesSurface1);
    SDL_FreeSurface(livesSurface1);

    SDL_Surface *livesSurface2 = IMG_Load("lives1.png");
    if (livesSurface2 == NULL)
    {
        printf("Error: lives1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->hearts[1] = SDL_CreateTextureFromSurface(renderer, livesSurface2);
    SDL_FreeSurface(livesSurface2);

    SDL_Surface *livesSurface3 = IMG_Load("lives2.png");
    if (livesSurface3 == NULL)
    {
        printf("Error: lives2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->hearts[2] = SDL_CreateTextureFromSurface(renderer, livesSurface3);
    SDL_FreeSurface(livesSurface3);

    SDL_Surface *livesSurface4 = IMG_Load("lives3.png");
    if (livesSurface4 == NULL)
    {
        printf("Error: lives3.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->hearts[3] = SDL_CreateTextureFromSurface(renderer, livesSurface4);
    SDL_FreeSurface(livesSurface4);

    // play
    SDL_Surface *playSurface1 = IMG_Load("playtext1.png");
    if (playSurface1 == NULL)
    {
        printf("Error: playtext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->playText[0] = SDL_CreateTextureFromSurface(renderer, playSurface1);
    SDL_FreeSurface(playSurface1);

    SDL_Surface *playSurface2 = IMG_Load("playtext2.png");
    if (playSurface2 == NULL)
    {
        printf("Error: playtext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->playText[1] = SDL_CreateTextureFromSurface(renderer, playSurface2);
    SDL_FreeSurface(playSurface2);

    // manual
    SDL_Surface *manualSurface1 = IMG_Load("manualtext1.png");
    if (manualSurface1 == NULL)
    {
        printf("Error: manualtext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->manualText[0] = SDL_CreateTextureFromSurface(renderer, manualSurface1);
    SDL_FreeSurface(manualSurface1);

    SDL_Surface *manualSurface2 = IMG_Load("manualtext2.png");
    if (manualSurface2 == NULL)
    {
        printf("Error: manualtext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->manualText[1] = SDL_CreateTextureFromSurface(renderer, manualSurface2);
    SDL_FreeSurface(manualSurface2);

    // exit
    SDL_Surface *exitSurface1 = IMG_Load("exittext1.png");
    if (exitSurface1 == NULL)
    {
        printf("Error: exittext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->exitText[0] = SDL_CreateTextureFromSurface(renderer, exitSurface1);
    SDL_FreeSurface(exitSurface1);

    SDL_Surface *exitSurface2 = IMG_Load("exittext2.png");
    if (exitSurface2 == NULL)
    {
        printf("Error: exittext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->exitText[1] = SDL_CreateTextureFromSurface(renderer, exitSurface2);
    SDL_FreeSurface(exitSurface2);

    // main menu
    SDL_Surface *menuSurface1 = IMG_Load("mainmenutext1.png");
    if (menuSurface1 == NULL)
    {
        printf("Error: mainmenutext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->mainMenuText[0] = SDL_CreateTextureFromSurface(renderer, menuSurface1);
    SDL_FreeSurface(menuSurface1);

    SDL_Surface *menuSurface2 = IMG_Load("mainmenutext2.png");
    if (menuSurface2 == NULL)
    {
        printf("Error: mainmenutext2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->mainMenuText[1] = SDL_CreateTextureFromSurface(renderer, menuSurface2);
    SDL_FreeSurface(menuSurface2);

    // play again
    SDL_Surface *playAgainSurface1 = IMG_Load("playagaintext1.png");
    if (playAgainSurface1 == NULL)
    {
        printf("Error: playagaintext1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->playAgainText[0] = SDL_CreateTextureFromSurface(renderer, playAgainSurface1);
    SDL_FreeSurface(playAgainSurface1);

    SDL_Surface *playAgainSurface2 = IMG_Load("playagaintext2.png");
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

void printReceipt(gameObjects *object, gameTextures *texture, SDL_Renderer *renderer)
{
    int i, j;

    char str[100001];

    object->font = TTF_OpenFont("Minecraft.ttf", 200);

    // bg
    SDL_Rect rectBG = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, texture->background, NULL, &rectBG);

    // darken
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
    SDL_RenderDrawRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});
    SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});

    SDL_Rect rectReceipt = {75, 0, 377, HEIGHT};
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
        SDL_RenderCopy(renderer, texture->itemNo[i], NULL, &(SDL_Rect){120, 135 + (i * 27), 9 * strlen(str), 16});

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
        SDL_RenderCopy(renderer, texture->itemLabels[i], NULL, &(SDL_Rect){165, 135 + (i * 27), 9 * strlen(itemNames[i]), 16});

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
            SDL_RenderCopy(renderer, texture->taka, NULL, &(SDL_Rect){320, 135 + (i * 27), 35, 16});
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
            SDL_RenderCopy(renderer, texture->taka, NULL, &(SDL_Rect){320, 135 + (i * 27), 14, 16});
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
            SDL_RenderCopy(renderer, texture->itemTotal[i], NULL, &(SDL_Rect){370, 135 + (i * 27), 9 * strlen(str), 16});
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
            SDL_RenderCopy(renderer, texture->itemTotal[i], NULL, &(SDL_Rect){370, 135 + (i * 27), 9, 16});
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
            SDL_RenderCopy(renderer, texture->itemTotal[i], NULL, &(SDL_Rect){370, 135 + (i * 27), 8.5 * strlen(str), 16});
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
    SDL_RenderCopy(renderer, texture->priceTotal[0], NULL, &(SDL_Rect){120, 180 + (i * 27), 50, 16});

    object->textSurface = TTF_RenderText_Solid(object->font, "tk", (SDL_Color){46, 46, 46, 255});
    if (object->textSurface == NULL)
    {
        printf("Error: textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->priceTotal[1] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
    SDL_FreeSurface(object->textSurface);
    SDL_RenderCopy(renderer, texture->priceTotal[1], NULL, &(SDL_Rect){320, 180 + (i * 27), 14, 16});

    object->textSurface = TTF_RenderText_Solid(object->font, itoa(score, str, 10), (SDL_Color){46, 46, 46, 255});
    if (object->textSurface == NULL)
    {
        printf("Error: textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    texture->priceTotal[2] = SDL_CreateTextureFromSurface(renderer, object->textSurface);
    SDL_FreeSurface(object->textSurface);
    SDL_RenderCopy(renderer, texture->priceTotal[2], NULL, &(SDL_Rect){370, 180 + (i * 27), 9 * strlen(str), 16});

    // play again text
    SDL_Rect rectPlay = {517, HEIGHT / 2 - 40, 115, 20};
    SDL_RenderCopy(renderer, texture->playAgainText[texture->hover[3]], NULL, &rectPlay);

    // main menu text
    SDL_Rect rectMenu = {517, HEIGHT / 2 + 20, 94, 20};
    SDL_RenderCopy(renderer, texture->mainMenuText[texture->hover[4]], NULL, &rectMenu);

    destroyTextTextures(texture);

    SDL_RenderPresent(renderer);
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

void playGame(gameObjects *object, gameTextures *texture, SDL_Renderer *renderer, FILE *file)
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

        printf("HIGHSCORE: %ld\n", highScore);

        page = PAGE_STATUS_OVER;
    }

    // joshim
    int c = (rand() % 2);
    if (!(object->time % 1100))
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
            item[8]++;
            score -= 69;
            printf("YOU HURT JOSHIM! -69!\nSCORE: %d\n", score);
            // SDL_Delay(250);
            loadJoshim(object);
        }
    }

    // srand(time(0));
    int r = (rand() % 8);
    if (!(object->time % 7) && (object->food[r].y >= HEIGHT || object->food[r].y < -(object->food[r].h)))
    {
        dropFood(object, r);
    }
    for (int i = 0; i < 8; i++)
    {
        if (foodCollision(object, object, i))
        {
            if (i == 7)
            {
                Mix_PlayChannel(-1, object->soundBytes[2], 0);
                lives--;
                printf("BOMB! LIVES: %d\n", lives);
                // SDL_Delay(250);
            }
            else
            {
                Mix_PlayChannel(-1, object->soundBytes[0], 0);
                score += prices[i];
                printf("SCORE: %d\n", score);
            }
            item[i]++;
            loadFood(object, i);
        }
    }
}

void renderPage(PageStatus page, gameObjects *object, gameTextures *texture, SDL_Renderer *renderer, SDL_Event *event, FILE *file)
{
    switch (page)
    {
    case PAGE_STATUS_MENU:
        renderMenu(object, texture, renderer);
        break;
    case PAGE_STATUS_GAME:
        playGame(object, texture, renderer, file);
        break;
    case PAGE_STATUS_OVER:
        printReceipt(object, texture, renderer);
        break;
    }
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("✧ CDSMania ✧", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    gameObjects object;
    gameTextures texture;

    loadMusic(&object);
    loadSurfaces(&object, &texture, renderer);

    FILE *file = fopen("highscore.txt", "r");
    fscanf(file, "%ld", &highScore);
    // printf("beg: %ld\n", highScore);

    Mix_PlayMusic(object.menuMusic, -1);
    page = PAGE_STATUS_MENU;

    // main game loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;

        renderPage(page, &object, &texture, renderer, &event, file);

        if (SDL_PollEvent(&event))
        {
            quit = quitCheck(&event);

            if (page == PAGE_STATUS_MENU)
            {
                // play button
                if (event.button.x >= 110 && event.button.x <= 198 && event.button.y >= HEIGHT - 175 && event.button.y <= HEIGHT - 175 + 19)
                {
                    if (event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        Mix_PlayMusic(object.gameMusic, -1);
                        loadMan(&object);
                        loadJoshim(&object);
                        for (int j = 0; j < 8; j++)
                        {
                            loadFood(&object, j);
                        }
                        score = 0;
                        lives = 3;
                        object.time = 0;
                        page = PAGE_STATUS_GAME;
                    }
                    else
                    {
                        texture.hover[0] = 1;
                    }
                }
                else
                {
                    texture.hover[0] = 0;
                }
                // manual button
                if (event.button.x >= (WIDTH / 2) - 60 && event.button.x <= (WIDTH / 2) - 60 + 121 && event.button.y >= HEIGHT - 175 && event.button.y <= HEIGHT - 175 + 19)
                {
                    if (event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        //
                    }

                    else
                    {
                        texture.hover[1] = 1;
                    }
                }
                else
                {
                    texture.hover[1] = 0;
                }
                // exit button
                if (event.button.x >= WIDTH - 110 - 92 && event.button.x <= WIDTH - 110 && event.button.y >= HEIGHT - 175 && event.button.y <= HEIGHT - 175 + 19)
                {
                    if (event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        quit = true;
                    }

                    else
                    {
                        texture.hover[2] = 1;
                    }
                }
                else
                {
                    texture.hover[2] = 0;
                }
            }
            if (page == PAGE_STATUS_OVER)
            {
                // replay button
                if (event.button.x >= 517 && event.button.x <= 632 && event.button.y >= HEIGHT / 2 - 40 && event.button.y <= HEIGHT / 2 - 20)
                {
                    if (event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        Mix_PlayMusic(object.gameMusic, -1);
                        loadMan(&object);
                        loadJoshim(&object);
                        for (int j = 0; j < 8; j++)
                        {
                            loadFood(&object, j);
                        }
                        score = 0;
                        lives = 3;
                        object.time = 0;
                        page = PAGE_STATUS_GAME;
                    }
                    else
                    {
                        texture.hover[3] = 1;
                    }
                }
                else
                {
                    texture.hover[3] = 0;
                }
                // menu button
                if (event.button.x >= 517 && event.button.x <= 611 && event.button.y >= HEIGHT / 2 - +20 && event.button.y <= HEIGHT / 2 + 40)
                {
                    if (event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        Mix_PlayMusic(object.menuMusic, -1);
                        page = PAGE_STATUS_MENU;
                    }
                    else
                    {
                        texture.hover[4] = 1;
                    }
                }
                else
                {
                    texture.hover[4] = 0;
                }
            }
        }
    }

    fclose(file);
    destroyTextures(&texture);
    destroyMusic(&object);
    TTF_CloseFont(object.font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}