#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>

#define BITDEPTH 32
#define FRAMELENGTH_MS 16
#define MICRO_PER_MILLI 1000
#define SCREENPOS_X 100
#define SCREENPOS_Y 100
#define WIN_W 450
#define WIN_H 600

// Bird Stats:
#define BIRD_A 0.3
#define CLICK_V -7.0
#define MAX_V 5.0

// Game screens:
#define MENU_SCREEN 0
#define PLAY_SCREEN 1

// Pipe stats:
#define GAP_H 150
#define GAP_TOP_MIN 20
#define GAP_TOP_MAX 380
#define PIPE_START_X 400
#define PIPE_W 50
// Speed at which a pipe moves left:
#define PIPE_V -8
// Distance from the start of one pipe to the start of the next:
#define PIPE_D 300

/**
 * Linked list node for a pipe.
 */
struct Pipe {
    // Horizontal position of the pipe:
    int x;
    // Top of the opening:
    int y;
    // The next pipe in the chain:
    struct Pipe* next;
};

/**
 * Draw the bird onto a surface
 * @param surf The surface to draw the bird onto
 * @param x The x-coordinate of the center of the bird
 * @param y The y-coordinate of the center of the bird
 */
void draw_bird(SDL_Surface* surf, int x, int y) {
    // For right now the bird is just a 25x25 square:
    SDL_Rect birdrect;
    birdrect.w = 25;
    birdrect.h = 25;
    birdrect.x = x - (birdrect.w / 2);
    birdrect.y = y - (birdrect.h / 2);
    
    SDL_FillRect(surf, &birdrect, SDL_MapRGB(surf->format, 255, 255, 255));
}

/**
 * Draw a pipe onto a surface
 * @param surf The surface to draw the pipe onto
 * @param x The left edge of the pipe
 * @param y The top of the gap in the pipe
 */
void draw_pipe(SDL_Surface* surf, int x, int y) {
    SDL_Rect piperect;
    
    // Top pipe:
    piperect.w = PIPE_W;
    piperect.h = y;
    piperect.x = x;
    piperect.y = 0;
    
    SDL_FillRect(surf, &piperect, SDL_MapRGB(surf->format, 0, 127, 0));
    
    // Bottom pipe:
    piperect.y = y + GAP_H;
    piperect.h = WIN_H - piperect.y;
    
    SDL_FillRect(surf, &piperect, SDL_MapRGB(surf->format, 0, 127, 0));
}

struct Pipe* make_pipe(int x) {
    struct Pipe* p = malloc(sizeof(struct Pipe));
    
    p->x = x;
    p->y = rand() % (GAP_TOP_MAX - GAP_TOP_MIN) + GAP_TOP_MIN;
    
    return p;
}

int main(int argc, char** argv) {
    // Game Setup:
    const int bird_start_x = WIN_W / 4;
    const int bird_start_y = WIN_H / 2;
    int bird_y;
    // Start velocity downward, px per frame:
    const double bird_start_v = 0.0;
    double bird_v;
    
    // First pipe in the chain each game:
    struct Pipe* first_pipe = NULL;
    
    // Seed the random number generator:
    srand((int)clock());
    
    // SDL Setup:
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow(
        "Flappy Bird",
        SCREENPOS_X,
        SCREENPOS_Y,
        WIN_W,
        WIN_H,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    
    // Main program loop:
    clock_t last_frame_time = clock();
    int run = 1;
    SDL_Event event;
    int screen = MENU_SCREEN;
    while (run) {
        // Process events:
        while (SDL_PollEvent(&event)) {
            // We do this rather than run = event.type != SDL_QUIT
            // in case there are other events after.
            if (event.type == SDL_QUIT) {
                run = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                // A single click on the menu brings you to the game:
                if (screen == MENU_SCREEN) {
                    screen = PLAY_SCREEN;
                } else if (screen == PLAY_SCREEN) {
                    // Each jump is just an instant V change:
                    bird_v = CLICK_V;
                }
            }
        }
        
        // Process frames:
        if (clock() - last_frame_time >= MICRO_PER_MILLI * FRAMELENGTH_MS) {
            // Draw the frame:
            SDL_Surface* surf = SDL_CreateRGBSurface(
                0,
                WIN_W,
                WIN_H,
                BITDEPTH,
                0,
                0,
                0,
                0
            );
            
            if (screen == PLAY_SCREEN) {
                // Move each pipe along:
                first_pipe->x += PIPE_V;
                draw_pipe(surf, first_pipe->x, first_pipe->y);
                
                // Accelerate bird until terminal velocity:
                bird_v = fmin(BIRD_A + bird_v, MAX_V);
                bird_y += bird_v;
                draw_bird(surf, bird_start_x, bird_y);
            } else if (screen == MENU_SCREEN) {
                // Reset bird:
                bird_y = bird_start_y;
                bird_v = bird_start_v;
                
                // Set up pipe chain:
                first_pipe = make_pipe(PIPE_START_X);
            }
        
            // Render the frame:
            SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
            SDL_FreeSurface(surf);
            SDL_RenderClear(ren);
            SDL_RenderCopy(ren, tex, NULL, NULL);
            SDL_DestroyTexture(tex);
            SDL_RenderPresent(ren);
        
            // Start the next frame:
            last_frame_time = clock();
        }
    }
    
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}