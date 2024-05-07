#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

const int WIDTH = 720, HEIGHT = 540;

const float gravity = 0.35;

int score, lives;
const int prices[7] = {70, 25, 40, 25, 10, 10, 50};
unsigned int item[9] = {0}, totalPerItem[7] = {0};
const char itemNames[9][101] = {"x Burger", "x Chicken Ball", "x Double Coffee", "x Lemonade", "x Samosa", "x Chaa", "x Sandwich", "x Bombs", "x Joshims hurt"};

// general
typedef struct
{
    float x, y, dy, dx;
    int h, w, frame;
    bool right, up;
    bool jumpAllowed;
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
    Things food[8];
    Cat joshim;

    int time;

    TTF_Font *font;

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

    SDL_Surface *scoreSurface, *livesSurface;

    Mix_Music *menuMusic, *gameMusic;
    Mix_Chunk *soundBytes[5];

    SDL_Renderer *renderer;

} gameObjects;

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

typedef enum
{
    PAGE_STATUS_MENU,
    PAGE_STATUS_GAME,
    PAGE_STATUS_OVER
} PageStatus;

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
    if (state[SDL_SCANCODE_LEFT])
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
    else if (state[SDL_SCANCODE_RIGHT])
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
        Mix_PlayChannel(-1, object->soundBytes[3], 0);

        object->man.y -= object->man.dy;
        object->man.dy -= gravity;

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
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
        {
            if (object->man.y > 0)
            {
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
        object->food[i].w = 60;
        object->food[i].h = 88;
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
    object->joshim.h = 79;
    object->joshim.w = 112;
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
void renderGame(gameObjects *object, SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 135, 175, 255);
    // SDL_Rect rect = {0, 0, WIDTH, HEIGHT};
    SDL_RenderClear(renderer);

    // bg
    // SDL_Rect rectBG = {0, 0, WIDTH, HEIGHT};
    // SDL_RenderCopy(renderer, object->background, NULL, &rectBG);

    // man
    SDL_Rect rectMan = {object->man.x, object->man.y, object->man.w, object->man.h};
    SDL_RenderCopyEx(renderer, object->character[object->man.frame], NULL, &rectMan, 0, NULL, (object->man.right == 0));

    // food
    SDL_Rect rectFood0 = {object->food[0].x, object->food[0].y, object->food[0].w, object->food[0].h};
    SDL_RenderCopy(renderer, object->burger, NULL, &rectFood0);

    SDL_Rect rectFood1 = {object->food[1].x, object->food[1].y, object->food[1].w, object->food[1].h};
    SDL_RenderCopy(renderer, object->chknball, NULL, &rectFood1);

    SDL_Rect rectFood2 = {object->food[2].x, object->food[2].y, object->food[2].w, object->food[2].h};
    SDL_RenderCopy(renderer, object->dblcoffee, NULL, &rectFood2);

    SDL_Rect rectFood3 = {object->food[3].x, object->food[3].y, object->food[3].w, object->food[3].h};
    SDL_RenderCopy(renderer, object->lemonade, NULL, &rectFood3);

    SDL_Rect rectFood4 = {object->food[4].x, object->food[4].y, object->food[4].w, object->food[4].h};
    SDL_RenderCopy(renderer, object->samosa, NULL, &rectFood4);

    SDL_Rect rectFood5 = {object->food[5].x, object->food[5].y, object->food[5].w, object->food[5].h};
    SDL_RenderCopy(renderer, object->chaa, NULL, &rectFood5);

    SDL_Rect rectFood6 = {object->food[6].x, object->food[6].y, object->food[6].w, object->food[6].h};
    SDL_RenderCopy(renderer, object->sndwch, NULL, &rectFood6);

    SDL_Rect rectBomb1 = {object->food[7].x, object->food[7].y, object->food[7].w, object->food[7].h};
    SDL_RenderCopy(renderer, object->bomb[object->food[7].frame], NULL, &rectBomb1);

    // joshim
    SDL_Rect rectjoshim = {object->joshim.x, object->joshim.y, object->joshim.w, object->joshim.h};
    SDL_RenderCopyEx(renderer, object->josh[object->joshim.frame], NULL, &rectjoshim, 0, NULL, (object->joshim.right == 0));

    // score display
    unsigned char str[100001];

    object->scoreSurface = TTF_RenderText_Solid(object->font, "BILL: ", (SDL_Color){255, 255, 255, 255});
    if (object->scoreSurface == NULL)
    {
        printf("Error: score textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->scoreText[0] = SDL_CreateTextureFromSurface(renderer, object->scoreSurface);
    SDL_FreeSurface(object->scoreSurface);
    SDL_RenderCopy(renderer, object->scoreText[0], NULL, &(SDL_Rect){20, 20, 10 * strlen("BILL: "), 24});

    object->scoreSurface = TTF_RenderText_Solid(object->font, itoa(score, str, 10), (SDL_Color){255, 255, 255, 255});
    object->scoreText[1] = SDL_CreateTextureFromSurface(renderer, object->scoreSurface);
    SDL_FreeSurface(object->scoreSurface);
    SDL_RenderCopy(renderer, object->scoreText[1], NULL, &(SDL_Rect){80, 20, 14 * strlen(str), 24});

    // lives display
    object->livesSurface = TTF_RenderText_Solid(object->font, "LIVES: ", (SDL_Color){255, 255, 255, 255});
    if (object->livesSurface == NULL)
    {
        printf("Error: score livesSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->livesText = SDL_CreateTextureFromSurface(renderer, object->livesSurface);
    SDL_FreeSurface(object->livesSurface);
    SDL_RenderCopy(renderer, object->livesText, NULL, &(SDL_Rect){WIDTH - 180, 20, 11 * strlen("LIVES: "), 24});

    SDL_Rect rectHeart = {WIDTH - 100, 20, 80, 20};
    SDL_RenderCopy(renderer, object->hearts[lives], NULL, &rectHeart);

    SDL_RenderPresent(renderer);
}

void renderMenu(gameObjects *object, SDL_Renderer *renderer)
{
    SDL_Rect rectMain = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, object->mainScreen, NULL, &rectMain);

    SDL_RenderPresent(renderer);
}

void loadSurfaces(gameObjects *object, SDL_Renderer *renderer)
{
    // man
    SDL_Surface *charSurface1 = IMG_Load("character1.png");
    if (charSurface1 == NULL)
    {
        printf("Error: character1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->character[0] = SDL_CreateTextureFromSurface(renderer, charSurface1);
    SDL_FreeSurface(charSurface1);

    SDL_Surface *charSurface2 = IMG_Load("character2.png");
    if (charSurface2 == NULL)
    {
        printf("Error: character2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->character[1] = SDL_CreateTextureFromSurface(renderer, charSurface2);
    SDL_FreeSurface(charSurface2);

    // food
    SDL_Surface *burgerSurface = IMG_Load("burger.png");
    if (burgerSurface == NULL)
    {
        printf("Error: burger.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->burger = SDL_CreateTextureFromSurface(renderer, burgerSurface);
    SDL_FreeSurface(burgerSurface);

    SDL_Surface *chknballSurface = IMG_Load("chknball.png");
    if (chknballSurface == NULL)
    {
        printf("Error: chknball.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->chknball = SDL_CreateTextureFromSurface(renderer, chknballSurface);
    SDL_FreeSurface(chknballSurface);

    SDL_Surface *dblcoffeeSurface = IMG_Load("dblcoffee.png");
    if (dblcoffeeSurface == NULL)
    {
        printf("Error: dblcoffee.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->dblcoffee = SDL_CreateTextureFromSurface(renderer, dblcoffeeSurface);
    SDL_FreeSurface(dblcoffeeSurface);

    SDL_Surface *lemonadeSurface = IMG_Load("lemonade.png");
    if (lemonadeSurface == NULL)
    {
        printf("Error: lemonade.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->lemonade = SDL_CreateTextureFromSurface(renderer, lemonadeSurface);
    SDL_FreeSurface(lemonadeSurface);

    SDL_Surface *samosaSurface = IMG_Load("samosa.png");
    if (samosaSurface == NULL)
    {
        printf("Error: samosa.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->samosa = SDL_CreateTextureFromSurface(renderer, samosaSurface);
    SDL_FreeSurface(samosaSurface);

    SDL_Surface *chaaSurface = IMG_Load("chaa.png");
    if (chaaSurface == NULL)
    {
        printf("Error: chaa.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->chaa = SDL_CreateTextureFromSurface(renderer, chaaSurface);
    SDL_FreeSurface(chaaSurface);

    SDL_Surface *sndwchSurface = IMG_Load("sndwch.png");
    if (sndwchSurface == NULL)
    {
        printf("Error: sndwch.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->sndwch = SDL_CreateTextureFromSurface(renderer, sndwchSurface);
    SDL_FreeSurface(sndwchSurface);

    SDL_Surface *bombSurface1 = IMG_Load("bomb1.png");
    if (bombSurface1 == NULL)
    {
        printf("Error: bomb1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->bomb[0] = SDL_CreateTextureFromSurface(renderer, bombSurface1);
    SDL_FreeSurface(bombSurface1);

    SDL_Surface *bombSurface2 = IMG_Load("bomb2.png");
    if (bombSurface2 == NULL)
    {
        printf("Error: bomb2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->bomb[1] = SDL_CreateTextureFromSurface(renderer, bombSurface2);
    SDL_FreeSurface(bombSurface2);

    /*//background
    SDL_Surface *bgSurface = IMG_Load("background.png");
    if(bgSurface == NULL)
    {
        printf("Error: background.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->background = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);*/

    // screen
    SDL_Surface *mainSurface = IMG_Load("mainScreen.png");
    if (mainSurface == NULL)
    {
        printf("Error: mainScreen.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->mainScreen = SDL_CreateTextureFromSurface(renderer, mainSurface);
    SDL_FreeSurface(mainSurface);

    // joshim
    SDL_Surface *joshimSurface1 = IMG_Load("joshim1.png");
    if (joshimSurface1 == NULL)
    {
        printf("Error: joshim1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->josh[0] = SDL_CreateTextureFromSurface(renderer, joshimSurface1);
    SDL_FreeSurface(joshimSurface1);

    SDL_Surface *joshimSurface2 = IMG_Load("joshim2.png");
    if (joshimSurface2 == NULL)
    {
        printf("Error: joshim2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->josh[1] = SDL_CreateTextureFromSurface(renderer, joshimSurface2);
    SDL_FreeSurface(joshimSurface2);

    // receipt
    SDL_Surface *receiptSurface = IMG_Load("receipt.png");
    if (receiptSurface == NULL)
    {
        printf("Error: receiptt.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->receiptbg = SDL_CreateTextureFromSurface(renderer, receiptSurface);
    SDL_FreeSurface(receiptSurface);

    // hearts
    SDL_Surface *livesSurface1 = IMG_Load("lives0.png");
    if (livesSurface1 == NULL)
    {
        printf("Error: lives0.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->hearts[0] = SDL_CreateTextureFromSurface(renderer, livesSurface1);
    SDL_FreeSurface(livesSurface1);

    SDL_Surface *livesSurface2 = IMG_Load("lives1.png");
    if (livesSurface2 == NULL)
    {
        printf("Error: lives1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->hearts[1] = SDL_CreateTextureFromSurface(renderer, livesSurface2);
    SDL_FreeSurface(livesSurface2);

    SDL_Surface *livesSurface3 = IMG_Load("lives2.png");
    if (livesSurface3 == NULL)
    {
        printf("Error: lives2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->hearts[2] = SDL_CreateTextureFromSurface(renderer, livesSurface3);
    SDL_FreeSurface(livesSurface3);

    SDL_Surface *livesSurface4 = IMG_Load("lives3.png");
    if (livesSurface4 == NULL)
    {
        printf("Error: lives3.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->hearts[3] = SDL_CreateTextureFromSurface(renderer, livesSurface4);
    SDL_FreeSurface(livesSurface4);
}

void destroyTextures(gameObjects *object)
{
    SDL_DestroyTexture(object->burger);
    SDL_DestroyTexture(object->chknball);
    SDL_DestroyTexture(object->dblcoffee);
    SDL_DestroyTexture(object->lemonade);
    SDL_DestroyTexture(object->samosa);
    SDL_DestroyTexture(object->chaa);
    SDL_DestroyTexture(object->sndwch);

    SDL_DestroyTexture(object->mainScreen);
    SDL_DestroyTexture(object->background);

    for (int j = 0; j < 2; j++)
    {
        SDL_DestroyTexture(object->character[j]);
        SDL_DestroyTexture(object->bomb[j]);
        SDL_DestroyTexture(object->josh[j]);
        SDL_DestroyTexture(object->scoreText[j]);
    }

    for (int i = 0; i < 9; i++)
    {
        SDL_DestroyTexture(object->itemNo[i]);
        SDL_DestroyTexture(object->itemLabels[i]);
        SDL_DestroyTexture(object->itemTotal[i]);
    }
    SDL_DestroyTexture(object->priceTotal[0]);
    SDL_DestroyTexture(object->priceTotal[1]);
    SDL_DestroyTexture(object->priceTotal[2]);
    SDL_DestroyTexture(object->taka);

    SDL_DestroyTexture(object->livesText);
    for (int k = 0; k < 4; k++)
    {
        SDL_DestroyTexture(object->hearts[k]);
    }

    SDL_DestroyTexture(object->receiptbg);
}

void printReceipt(gameObjects *object, SDL_Renderer *renderer)
{
    int i, j;

    char str[100001];

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect rectReceipt = {(WIDTH - 377) / 2, 0, 377, HEIGHT};
    SDL_RenderCopy(renderer, object->receiptbg, NULL, &rectReceipt);

    SDL_Surface *textSurface;

    for (i = 0; i < 9; i++)
    {
        // quantity of each item
        textSurface = TTF_RenderText_Solid(object->font, itoa(item[i], str, 10), (SDL_Color){46, 46, 46, 255});
        if (textSurface == NULL)
        {
            printf("Error: itemNo textSurface not found.\n");
            SDL_Quit();
            exit(1);
        }
        object->itemNo[i] = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
        SDL_RenderCopy(renderer, object->itemNo[i], NULL, &(SDL_Rect){215, 135 + (i * 27), 9 * strlen(str), 16});

        // name of item
        textSurface = TTF_RenderText_Solid(object->font, itemNames[i], (SDL_Color){46, 46, 46, 255});
        if (textSurface == NULL)
        {
            printf("Error: itemLabels textSurface not found.\n");
            SDL_Quit();
            exit(1);
        }
        object->itemLabels[i] = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
        SDL_RenderCopy(renderer, object->itemLabels[i], NULL, &(SDL_Rect){260, 135 + (i * 27), 9 * strlen(itemNames[i]), 16});

        // tk or lives
        if (i == 7)
        {
            textSurface = TTF_RenderText_Solid(object->font, "lives", (SDL_Color){46, 46, 46, 255});
            if (textSurface == NULL)
            {
                printf("Error: taka textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            object->taka = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);
            SDL_RenderCopy(renderer, object->taka, NULL, &(SDL_Rect){405, 135 + (i * 27), 35, 16});
        }
        else
        {
            textSurface = TTF_RenderText_Solid(object->font, "tk", (SDL_Color){46, 46, 46, 255});
            if (textSurface == NULL)
            {
                printf("Error: taka textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            object->taka = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);
            SDL_RenderCopy(renderer, object->taka, NULL, &(SDL_Rect){405, 135 + (i * 27), 14, 16});
        }

        // total per item
        if (i < 7)
        {
            totalPerItem[i] = item[i] * prices[i];
            textSurface = TTF_RenderText_Solid(object->font, itoa(totalPerItem[i], str, 10), (SDL_Color){46, 46, 46, 255});
            if (textSurface == NULL)
            {
                printf("Error: itemTotal textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            object->itemTotal[i] = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);
            SDL_RenderCopy(renderer, object->itemTotal[i], NULL, &(SDL_Rect){455, 135 + (i * 27), 9 * strlen(str), 16});
        }
        else if (i == 7)
        {
            textSurface = TTF_RenderText_Solid(object->font, "3", (SDL_Color){46, 46, 46, 255});
            if (textSurface == NULL)
            {
                printf("Error: itemTotal textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            object->itemTotal[i] = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);
            SDL_RenderCopy(renderer, object->itemTotal[i], NULL, &(SDL_Rect){455, 135 + (i * 27), 9, 16});
        }
        else if (i == 8)
        {
            int joshimTotal = item[i] * (-69);
            textSurface = TTF_RenderText_Solid(object->font, itoa(joshimTotal, str, 10), (SDL_Color){46, 46, 46, 255});
            if (textSurface == NULL)
            {
                printf("Error: itemTotal textSurface not found.\n");
                SDL_Quit();
                exit(1);
            }
            object->itemTotal[i] = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);
            SDL_RenderCopy(renderer, object->itemTotal[i], NULL, &(SDL_Rect){455, 135 + (i * 27), 8.5 * strlen(str), 16});
        }
    }

    // total price
    textSurface = TTF_RenderText_Solid(object->font, "TOTAL:", (SDL_Color){46, 46, 46, 255});
    if (textSurface == NULL)
    {
        printf("Error: textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->priceTotal[0] = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, object->priceTotal[0], NULL, &(SDL_Rect){215, 180 + (i * 27), 50, 16});

    textSurface = TTF_RenderText_Solid(object->font, "tk", (SDL_Color){46, 46, 46, 255});
    if (textSurface == NULL)
    {
        printf("Error: textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->priceTotal[1] = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, object->priceTotal[1], NULL, &(SDL_Rect){405, 180 + (i * 27), 14, 16});

    textSurface = TTF_RenderText_Solid(object->font, itoa(score, str, 10), (SDL_Color){46, 46, 46, 255});
    if (textSurface == NULL)
    {
        printf("Error: textSurface not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->priceTotal[2] = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, object->priceTotal[2], NULL, &(SDL_Rect){455, 180 + (i * 27), 9 * strlen(str), 16});

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

void playGame(gameObjects *object, SDL_Renderer *renderer)
{
    renderGame(object, renderer);

    manProcess(object);
    foodProcess(object);
    joshimProcess(object);

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
            SDL_Delay(250);
            loadJoshim(object);
        }
    }

    // srand(time(0));
    int r = (rand() % 8);
    if (!(object->time % 15) && (object->food[r].y >= HEIGHT || object->food[r].y < -(object->food[r].h)))
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
                SDL_Delay(250);
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

void renderPage(PageStatus page, gameObjects *object, SDL_Renderer *renderer, SDL_Event *event)
{
    switch (page)
    {
    case PAGE_STATUS_MENU:
        renderMenu(object, renderer);
        break;
    case PAGE_STATUS_GAME:
        playGame(object, renderer);
        break;
    case PAGE_STATUS_OVER:
        printReceipt(object, renderer);
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
    object.font = TTF_OpenFont("Minecraft.ttf", 200);

    loadMan(&object);
    loadJoshim(&object);
    for (int j = 0; j < 8; j++)
    {
        loadFood(&object, j);
    }
    loadSurfaces(&object, renderer);
    loadMusic(&object);

    score = 0;
    lives = 3;

    PageStatus page = PAGE_STATUS_MENU;

    Mix_PlayMusic(object.menuMusic, -1);
    // main game loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        renderPage(page, &object, renderer, &event);

        while (SDL_PollEvent(&event))
        {
            quit = quitCheck(&event);

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.x >= 300 && event.button.x <= 420 && event.button.y >= 330 && event.button.y <= 390)
            {
                Mix_PlayMusic(object.gameMusic, -1);
                page = PAGE_STATUS_GAME;
            }
        }
        if (!lives && page == PAGE_STATUS_GAME)
        {
            Mix_HaltMusic();
            Mix_PlayChannel(-1, object.soundBytes[4], 0);
            page = PAGE_STATUS_OVER;
        }
    }

    destroyTextures(&object);
    destroyMusic(&object);
    TTF_CloseFont(object.font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}