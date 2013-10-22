/**
 * Copyright © 2013 Jean-François Hren <jfhren@gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

#include <SDL/SDL.h>

typedef struct {
    Uint32 startedAt;
    Uint32 lastTick;
    unsigned int frameCountSinceStart;
    unsigned int frameCountSinceLastTick;
} FPSRecorder;


int initFPSRecorder(FPSRecorder* fps) {

    if(SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
        fprintf(stderr, "Could not start SDL Timer: %s\n", SDL_GetError());
        return -1;
    }

    fps->startedAt = SDL_GetTicks();
    fps->lastTick = fps->startedAt;
    fps->frameCountSinceStart = 0;
    fps->frameCountSinceLastTick = 0;

    return 0;

}


void updateFPSRecorder(FPSRecorder* fps) {

    Uint32 tick = SDL_GetTicks();

    fps->frameCountSinceLastTick++;

    if((tick - fps->lastTick) >= 1000) {
        printf("FPS: %4.u\n", fps->frameCountSinceLastTick);
        fps->frameCountSinceStart += fps->frameCountSinceLastTick;
        fps->frameCountSinceLastTick = 0;
        fps->lastTick = tick;
    }

}


void resultFPSRecorder(FPSRecorder* fps) {

    if(fps->startedAt == fps->lastTick)
        printf("Average FPS: unavailable\n");
    else
        printf("Average FPS: %.2f\n", fps->frameCountSinceStart / ((fps->lastTick - fps->startedAt) / 1000.0));

}

int main(void) {

    SDL_Surface* screen = NULL;
    short i = 0;
    short j = 0;
    int width = 640;
    int height = 480;
    char stillWaiting = 1;
    FPSRecorder fps;

    if(SDL_Init(SDL_INIT_VIDEO) == -1){
        fprintf(stderr, "Could not start SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if((screen = SDL_SetVideoMode(width, height, 0, SDL_SWSURFACE)) == NULL) {
        fprintf(stderr, "Could not setup the screen surface: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    if(initFPSRecorder(&fps) == -1) {
        SDL_Quit();
        return EXIT_FAILURE;
    }

    while(stillWaiting) {
        SDL_Event event;

        if(SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 255, 255, 255, 255)) == -1) {
            fprintf(stderr, "Could not clear the screen surface: %s\n", SDL_GetError());
            SDL_Quit();
            return EXIT_FAILURE;
        }

        for(i = 0; i < width; i+=64)
            for(j = 0; j < height; j++) {
                SDL_Rect pixel = {0, 0, 1, 1};

                pixel.x = i;
                pixel.y = j;

                if(SDL_FillRect(screen, &pixel, SDL_MapRGBA(screen->format, 0, 0, 0, 255)) == -1) {
                    fprintf(stderr, "Could not draw pixel at (%d,%d): %s\n", i, j, SDL_GetError());
                    SDL_Quit();
                    return EXIT_FAILURE;
                }
            }

        SDL_UpdateRect(screen, 0,0,0,0);

        while(SDL_PollEvent(&event) != 0) {
            if(((event.type == SDL_KEYDOWN) && (event.key.keysym.sym == SDLK_ESCAPE)) || (event.type == SDL_QUIT)) {
                stillWaiting = 0;
                break;
            }
        }
        updateFPSRecorder(&fps);
    }

    resultFPSRecorder(&fps);

    SDL_Quit();
    return EXIT_SUCCESS;

}
