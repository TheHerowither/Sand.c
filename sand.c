#include <stdio.h>
#include <stdbool.h>

#include <SDL3/SDL.h>
#include <clog.h>

#include <time.h>


#define BOARD_WIDTH 128
#define BOARD_HEIGHT 128

#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 720

#define SAND_COLOR_A 240, 240, 108, 255
#define SAND_COLOR_B 255, 255, 49, 255
#define SAND_COLOR_A_ARGS .r = 240, .g = 240, .b = 108, .a = 255
#define SAND_COLOR_B_ARGS .r = 255, .g = 255, .b = 49, .a = 255

#define GRAVITY_CONSTANT 1
#define FRAMERATE_LIMIT 60.0

#define grain(b, x, y) b.board[(b.width * (x)) + (y)]
#define clean_board(b) free(b.board)


typedef struct {
    int x;
    int y;
    bool simulated;
    size_t r, g, b, a;
} Grain;

typedef struct {
    Grain *items;
    size_t size;
} Grains;

typedef struct {
    bool *board;
    size_t width;
    size_t height;
} Board;


Board init_board(size_t w, size_t h) {
    Board b = (Board) {
        .board = malloc(w * h),
        .width = w,
        .height = h
    };
    memset(b.board, 0, w*h);
    return b;
}


void fail(void) {
    clog(CLOG_ERROR, "SDL_Error: %s", SDL_GetError())
    exit(1);
}

bool check_collision(size_t grain_idx, Grains *grains) {
    Grain grain = (*grains).items[grain_idx];
    for (size_t i = 0; i < grains->size; i++) {
        if (i != grain_idx) {
            Grain g = (*grains).items[i];
            if (grain.y+GRAVITY_CONSTANT == g.y && grain.x == g.x) {return 1;}
            //else if (grain.y == g.y && grain.x == g.x) {return 1;}
        }
    }
    return 0;
}

void simulate_grain(Board board, size_t grain_idx, Grains *grains) {
    Grain *grain = &(*grains).items[grain_idx];
    if ((grain->x >= 0 && grain->x < board.width - 1) && (grain->y >= 0 && grain->y < board.height - 1) && !grain->simulated && !check_collision(grain_idx, grains)) {
        grain->y+=GRAVITY_CONSTANT;
        grain->simulated = 1;
    }
}

void set_board(Board board, Grains *grains) {
    memset(board.board, 0, board.width * board.height);
    for (size_t i = 0; i < grains->size; i++) {
        if (((*grains).items[i].x >= 0 && (*grains).items[i].x < board.width) && ((*grains).items[i].y >= 0 && (*grains).items[i].y < board.height))
        grain(board, (*grains).items[i].x, (*grains).items[i].y) = 1;
    }
}

void reset_board(Grains *grains) {
    for (size_t i = 0; i < grains->size; i++) {
        grains->items[i].simulated = 0;
    }
}

void simulate(Board board, Grains *grains) {
    reset_board(grains);
    for (int x = 0; x < board.width; x++) {
        for (int y = 0; y < board.height; y++) {
            for (size_t k = 0; k < grains->size; k++) {
                if (x == (*grains).items[k].x && y == (*grains).items[k].y) simulate_grain(board, k, grains); 
            }
        }
    }
    set_board(board, grains);
}

Grain grain_at_position(Grains grains, size_t x, size_t y) {
    for (size_t i = 0; i < grains.size; i++) {
        if (grains.items[i].x == x && grains.items[i].y == y) return grains.items[i];
    }
}

void draw_board(SDL_Renderer *renderer, Board board, Grains grains) {
    for (size_t x = 0; x < board.width; x++) {
        for (size_t y = 0; y < board.height; y++) {
            if (grain(board, x, y) == 1) {
                Grain g = grain_at_position(grains, x, y);
                SDL_SetRenderDrawColor(renderer, g.r, g.g, g.b, g.a);
            }
            if (grain(board, x, y) == 0) SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderPoint(renderer, x, y);
        }
    }
}

void grains_append(Grains *grains, Grain grain) {
    (*grains).size++;
    (*grains).items = realloc((*grains).items, grains->size * sizeof(Grain));
    (*grains).items[grains->size-1] = grain;
}

void convert_to_boardspace(float *x, float *y, int limit_x, int limit_y, Board board) {
    *x = (*x) / limit_x * board.width;
    *y = (*y) / limit_y * board.height;
}

bool running = 1;

int main(void) {
    CLOG_INIT;
    clog_set_fmt("%c[%L]%r: %m");

    clock_t start, end;

    // SDL3 init
    clog(CLOG_DEBUG, "Initializing SDL3");
    start = clock();
    if (SDL_Init(SDL_INIT_VIDEO)) fail();
    end = clock();
    clog(CLOG_DEBUG, "Initialized SDL3 in %fs", (double)(end - start) / (double)CLOCKS_PER_SEC)

    // Window init
    clog(CLOG_DEBUG, "Initializing SDL_Window")
    start = clock();
    SDL_Window *window = SDL_CreateWindow("Sand", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) fail();
    end = clock();
    clog(CLOG_DEBUG, "Initialized SDL_Window in %fs", (double)(end - start) / (double)CLOCKS_PER_SEC)

    // Renderer init
    clog(CLOG_DEBUG, "Initializing SDL_Renderer")
    start = clock();
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, 0);
    if (!window) fail();
    end = clock();
    clog(CLOG_DEBUG, "Initialized SDL_Renderer in %fs", (double)(end - start) / (double)CLOCKS_PER_SEC)

    Board board = init_board(BOARD_WIDTH, BOARD_HEIGHT);
    Grains grains = {.items = malloc(0 * sizeof(Grain)), .size = 0};
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, BOARD_WIDTH, BOARD_HEIGHT);
    
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    // Mainloop
    SDL_ShowWindow(window);
    clog(CLOG_INFO, "Initialized application");

    Uint64 a, b; 
    double delta;
    bool spawned_this_frame = 0;
    while (running) {
        a = SDL_GetTicks();
        delta = a - b;

        if (delta > 1000.0/FRAMERATE_LIMIT) {
            spawned_this_frame = 0;
            printf("Framerate: %f. Amount of Grains: %llu\r", 1000.0 / delta, grains.size);
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type)
                {
                case SDL_EVENT_QUIT:
                    running = 0;
                    break;                
                default:
                    break;
                }
            }

            if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK && !spawned_this_frame) {
                float x, y;
                SDL_GetMouseState(&x, &y);
                convert_to_boardspace(&x, &y, WINDOW_WIDTH, WINDOW_HEIGHT, board);
                int mod = rand()%2;
                if (mod == 1) grains_append(&grains, (Grain)  {.x = (int)x, .y = (int)y, SAND_COLOR_A_ARGS});
                else if (mod == 0) grains_append(&grains, (Grain)  {.x = (int)x, .y = (int)y, SAND_COLOR_B_ARGS});
                spawned_this_frame = 1;
            }
            if (!running) break;
            SDL_RenderClear(renderer);
            SDL_SetRenderTarget(renderer, texture);
            simulate(board, &grains);
            draw_board(renderer, board, grains);
            b = a;
            SDL_SetRenderTarget(renderer, NULL);
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
    }

    // Cleanup
    start = clock();
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(grains.items);
    free(board.board);
    end = clock();
    clog(CLOG_INFO, "Successfully closed and cleaned up application!");
    clog(CLOG_DEBUG, "Cleaning up took %fs", (double)(end - start) / (double)CLOCKS_PER_SEC)

    return 0;
}