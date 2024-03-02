#include <stdio.h>
#include <SDL3/SDL.h>
#include <clog.h>

#include <time.h>


void fail(void) {
    clog(CLOG_ERROR, "SDL_Error: %s", SDL_GetError())
    exit(1);
}

unsigned short running = 1;

int main(void) {
    CLOG_INIT;
    clog_set_fmt("%c[%L]%r: %m");

    clock_t start, end;

    clog(CLOG_DEBUG, "Initializing SDL3");
    start = clock();
    if (SDL_Init(SDL_INIT_VIDEO)) fail();
    end = clock();
    clog(CLOG_DEBUG, "Initialized SDL3 in %fs", (double)(end - start) / (double)CLOCKS_PER_SEC)

    clog(CLOG_DEBUG, "Initializing SDL_Window")
    start = clock();
    SDL_Window *window = SDL_CreateWindow("Sand", 1080, 720, 0);
    if (!window) fail();
    end = clock();
    clog(CLOG_DEBUG, "Initialized SDL_Window in %fs", (double)(end - start) / (double)CLOCKS_PER_SEC)

    SDL_ShowWindow(window);
    clog(CLOG_INFO, "Initialized application");
    while (running) {
         SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = 0;
            break;
        }
    }
    start = clock();
    SDL_DestroyWindow(window);
    SDL_Quit();
    end = clock();
    clog(CLOG_INFO, "Successfully closed and cleaned up application!");
    clog(CLOG_DEBUG, "Cleaning up took %fs", (double)(end - start) / (double)CLOCKS_PER_SEC)

    return 0;
}