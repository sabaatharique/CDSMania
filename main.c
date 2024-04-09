#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
//#include <SDL2/SDL_ttf.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

//const float gravity = 0.06f;

const int WIDTH = 720, HEIGHT = 510;

typedef struct
{
    float x, y, dy;
    int h, w, frame, right, up;

} Man;


typedef struct
{
    float x, y, dy;
    int h, w, frame;
    char name[100];

} Food;


typedef struct
{
    Man man;
    Food food[6];

    int time;

    SDL_Texture *character[2];
    SDL_Texture *burger;
    SDL_Texture *chknball;
    SDL_Texture *dblcoffee;
    SDL_Texture *lemonade;
    SDL_Texture *samosa;
    SDL_Texture *bomb[3]; 
    SDL_Texture *mainScreen;

    SDL_Renderer *renderer;
    
} gameObjects;



void renderObjects(SDL_Renderer *renderer, gameObjects *object)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect rect = {0, 0, WIDTH, HEIGHT};
    SDL_RenderFillRect(renderer, &rect);

    //bg
    //SDL_Rect bgRect = {0, 0, WIDTH, HEIGHT};
    //SDL_RenderCopy(renderer, object->mainScreen, NULL, &bgRect);


    //man
    SDL_Rect charMan1 = {object->man.x, object->man.y, object->man.w, object->man.h};
    SDL_RenderCopyEx(renderer, object->character[object->man.frame], NULL, &charMan1, 0, NULL, (object->man.right == 0));

    //SDL_Rect charMan2 = {object->man.x, object->man.y, object->man.w, object->man.h};
    //SDL_RenderCopy(renderer, object->character[1], NULL, &charMan2);

    //food
    SDL_Rect charRect0 = {object->food[0].x, object->food[0].y, object->food[0].w, object->food[0].h};
    SDL_RenderCopy(renderer, object->burger, NULL, &charRect0);

    SDL_Rect charRect1 = {object->food[1].x, object->food[1].y, object->food[1].w, object->food[1].h};
    SDL_RenderCopy(renderer, object->chknball, NULL, &charRect1);

    SDL_Rect charRect2 = {object->food[2].x, object->food[2].y, object->food[2].w, object->food[2].h};
    SDL_RenderCopy(renderer, object->dblcoffee, NULL, &charRect2);

    SDL_Rect charRect3 = {object->food[3].x, object->food[3].y, object->food[3].w, object->food[3].h};
    SDL_RenderCopy(renderer, object->lemonade, NULL, &charRect3);

    SDL_Rect charRect4 = {object->food[4].x, object->food[4].y, object->food[4].w, object->food[4].h};
    SDL_RenderCopy(renderer, object->samosa, NULL, &charRect4);

    SDL_Rect charRect5 = {object->food[5].x, object->food[5].y, object->food[5].w, object->food[5].h};
    SDL_RenderCopy(renderer, object->bomb[object->food[5].frame], NULL, &charRect5);

    SDL_RenderPresent(renderer);
}


/* 
0 = burger
1 = chknball
2 = dblcoffee
3 = lemonade
4 = samosa
5 = bomb
*/


bool quitCheck(SDL_Event *event, gameObjects *object)
{
    bool quit = false;
    switch(event->type){
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_KEYDOWN:
            if(event->key.keysym.sym == SDLK_ESCAPE){
                quit = true;
                break;
            }
    }

    return quit;
}


void gameProcess(gameObjects *object)
{
    //time
    object->time++;

    //movement
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if(state[SDL_SCANCODE_LEFT])
    {
        object->man.right = 0;
        if(object->man.x > 0)
        {
            object->man.x -= 4;
        }

        if(object->time % 15 == 0)
        {
            if(object->man.frame == 0)
                object->man.frame = 1;
            else
                object->man.frame = 0;
        }
    }
    else if(state[SDL_SCANCODE_RIGHT])
    {
        
        object->man.right = 1;
        if(object->man.x < (WIDTH - object->man.w))
        {
            object->man.x += 4;
        }
    
        if(object->time % 15 == 0)
        {
            if(object->man.frame == 0)
                object->man.frame = 1;
            else
                object->man.frame = 0;
        }
    }


    if(object->man.up)
    {
        object->man.dy = 1.5;
        object->man.y += object->man.dy;

        if(object->man.y >= (HEIGHT - object->man.h))
        {
            object->man.dy = 0;
            object->man.up = 0;
        }  
    }

    //food drop
    for(int i=0; i<6; i++){
        object->food[i].y += object->food[i].dy;
    }
}

void manJump(gameObjects *object, SDL_Event *event)
{
    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_SPACE)
    {
        if(object->man.y > 0)
        object->man.y = HEIGHT - object->man.h - 150;
        object->man.up = 1;
    }
}


bool collision(gameObjects *obj1, gameObjects *obj2, int i)
{
    float x1 = obj1->man.x, y1 = obj1->man.y;
    float w1 = obj1->man.w, h1 = obj1->man.h;

    float x2 = obj2->food[i].x, y2 = obj2->food[i].y;
    float w2 = obj2->food[i].w, h2 = obj2->food[i].h;

    if(x1 < (x2 + w2) && x2 < (x1 + w1) && y1 < (y2 + h2) && y2 <(y1 + h1))
        return true;

    return false;
}


void loadGame(gameObjects *object)
{
    //man
    object->man.h = 145;
    object->man.w = 80;
    object->man.dy = 0;
    object->man.up = 0;
    object->man.x = ((WIDTH - object->man.w)/2);
    object->man.y = HEIGHT - object->man.h;
    object->man.frame = 0;
    object->man.right = 1;

    //time
    object->time = 0;
    
    //food
    for(int i=0; i<6; i++)
    {
        if(i == 5)
        {
            object->food[i].w = 65;
            object->food[i].h = 65;
        }
        else
        {
            object->food[i].w = 50;
            object->food[i].h = 50;
        }
        
        object->food[i].frame = 0;
        object->food[i].x = (rand() % (WIDTH - object->food[i].w));
        object->food[i].y = -(object->food[i].h);
        object->food[i].dy = 1 + (i/2);
        object->food[i].y += object->food[i].dy;
    }

}

void loadSurfaces(gameObjects *object, SDL_Renderer *renderer){
    //man
    SDL_Surface *charSurface1 = IMG_Load("character1.png");
    if(charSurface1 == NULL)
    {
        printf("Error: character1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->character[0] = SDL_CreateTextureFromSurface(renderer, charSurface1);
    SDL_FreeSurface(charSurface1);

    SDL_Surface *charSurface2 = IMG_Load("character2.png");
    if(charSurface2 == NULL)
    {
        printf("Error: character2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->character[1] = SDL_CreateTextureFromSurface(renderer, charSurface2);
    SDL_FreeSurface(charSurface2);

    //food
    SDL_Surface *burgerSurface = IMG_Load("burger.png");
    if(burgerSurface == NULL)
    {
        printf("Error: burger.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->burger = SDL_CreateTextureFromSurface(renderer, burgerSurface);
    SDL_FreeSurface(burgerSurface);

    SDL_Surface *chknballSurface = IMG_Load("chknball.png");
    if(chknballSurface == NULL)
    {
        printf("Error: chknball.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->chknball = SDL_CreateTextureFromSurface(renderer, chknballSurface);
    SDL_FreeSurface(chknballSurface);

    SDL_Surface *dblcoffeeSurface = IMG_Load("dblcoffee.png");
    if(dblcoffeeSurface == NULL)
    {
        printf("Error: dblcoffee.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->dblcoffee = SDL_CreateTextureFromSurface(renderer, dblcoffeeSurface);
    SDL_FreeSurface(dblcoffeeSurface);

    SDL_Surface *lemonadeSurface = IMG_Load("lemonade.png");
    if(lemonadeSurface == NULL)
    {
        printf("Error: lemonade.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->lemonade = SDL_CreateTextureFromSurface(renderer, lemonadeSurface);
    SDL_FreeSurface(lemonadeSurface);

    SDL_Surface *samosaSurface = IMG_Load("samosa.png");
    if(samosaSurface == NULL)
    {
        printf("Error: samosa.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->samosa = SDL_CreateTextureFromSurface(renderer, samosaSurface);
    SDL_FreeSurface(samosaSurface);

    SDL_Surface *bombSurface1 = IMG_Load("bomb1.png");
    if(bombSurface1 == NULL)
    {
        printf("Error: bomb1.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->bomb[0] = SDL_CreateTextureFromSurface(renderer, bombSurface1);
    SDL_FreeSurface(bombSurface1);

    SDL_Surface *bombSurface2 = IMG_Load("bomb2.png");
    if(bombSurface2 == NULL)
    {
        printf("Error: bomb2.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->bomb[1] = SDL_CreateTextureFromSurface(renderer, bombSurface2);
    SDL_FreeSurface(bombSurface2);

    SDL_Surface *bombSurface3 = IMG_Load("bomb3.png");
    if(bombSurface3 == NULL)
    {
        printf("Error: bomb3.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->bomb[2] = SDL_CreateTextureFromSurface(renderer, bombSurface3);
    SDL_FreeSurface(bombSurface3);

    SDL_Surface *bgSurface = IMG_Load("mainScreen.png");
    if(bgSurface == NULL)
    {
        printf("Error: mainScreen.png not found.\n");
        SDL_Quit();
        exit(1);
    }
    object->mainScreen = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);
}


void dropFood(gameObjects *object, int i){
    object->food[i].x = (rand() % (WIDTH - object->food[i].w));
    object->food[i].y = -(object->food[i].h);
    object->food[i].y += object->food[i].dy;
}


void destroyTextures(gameObjects *object){
    SDL_DestroyTexture(object->character[0]);
    SDL_DestroyTexture(object->character[1]);
    SDL_DestroyTexture(object->burger);
    SDL_DestroyTexture(object->chknball);
    SDL_DestroyTexture(object->dblcoffee);
    SDL_DestroyTexture(object->lemonade);
    SDL_DestroyTexture(object->samosa);
    SDL_DestroyTexture(object->bomb[0]);
    SDL_DestroyTexture(object->bomb[1]);
    SDL_DestroyTexture(object->bomb[2]);
    SDL_DestroyTexture(object->mainScreen);
}


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    //TTF_Init();

    SDL_Window *window = SDL_CreateWindow("✧ CDSMania ✧", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    gameObjects object;

    loadGame(&object);
    loadSurfaces(&object, renderer);

    int score = 0;
    int lives = 3;
    int prices[5] = {70, 25, 40, 25, 10};

    //main game loop
    bool quit = false;
    while(!quit)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            //manJump(&object, &event);
            quit = quitCheck(&event, &object);
        }

        if(!lives)
        {
            printf("GAME OVER!");
            SDL_Delay(750);
            quit = true;
        }

        renderObjects(renderer, &object);
        gameProcess(&object);

        int r = (rand() % 7) - 1;
        if(!(object.time % 15) && (object.food[r].y >= HEIGHT || object.food[r].y < -(object.food[r].h)))
        {
            dropFood(&object, r);
        }
        
        for(int i=0; i<6; i++)
        {
            if(collision(&object, &object, i))
            {
                if(i == 5)
                {
                    SDL_Delay(250);
                    lives--;
                    printf("BOMB! LIVES: %d\n", lives);
                }
                else
                {
                    score += prices[i];
                    printf("SCORE: %d\n", score);
                }
                object.food[i].y = HEIGHT;
            }
        }   
    }
        
    SDL_Delay(60);
    destroyTextures(&object);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
