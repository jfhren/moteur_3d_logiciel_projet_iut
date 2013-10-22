/**
 * Copyright © 2013 Jean-François Hren <jfhren@gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


#define HOW_MUCH_TO_READ 1024

char* readFile(int fd) {

    char* result = NULL;
    char* tmp = NULL;
    int alreadyRead = 0;   
    int justRead = 0;

    if((result = (char*)malloc(sizeof(char) * HOW_MUCH_TO_READ)) == NULL) {
        perror("Error while allocating memories for reading the file");
        return NULL;
    }

    while((justRead = read(fd, result + alreadyRead, HOW_MUCH_TO_READ)) > 0) {
        alreadyRead += justRead;

        if((tmp = realloc(result, sizeof(char) * (alreadyRead + HOW_MUCH_TO_READ))) == NULL) {
            perror("Error while reallocating memories for reading the file");
            free(result);
            return NULL;
        }

        result = tmp;
    }

    if(justRead == -1) {
        perror("Error while reading the file");
        free(result);
        return NULL;
    }

    if((tmp = (char*)realloc(result, sizeof(char) * (alreadyRead + 1))) == NULL) {
        perror("Error while reallocating memories for NULL terminating the string");
        free(result);
        return NULL;
    }

    result = tmp;
    result[alreadyRead] = '\0';

    return result;

}


int parsePathPoint(char** crtPosition, double** path, unsigned int* nbControlPoints) {
    char* newDelim = NULL;
    double xPoint = 0;
    double yPoint = 0;
    double* tmp = NULL;

    xPoint = strtod(*crtPosition, &newDelim);
    if(*crtPosition == newDelim) {
        fprintf(stderr, "Missing path x value\n");
        return -1;
    }
    if(*newDelim != ',') {
        fprintf(stderr,"Missing ',' between value\n");
        return -1;
    }

    *crtPosition = newDelim + 1;
    yPoint = strtod(*crtPosition, &newDelim);
    if(*crtPosition == newDelim) {
        fprintf(stderr, "Missing path y value\n");
        return -1;
    }
    if(*newDelim != ' ') {
        fprintf(stderr,"Missing ' ' between value\n");
        return -1;
    }
    *crtPosition = newDelim;

    (*nbControlPoints)++;
    tmp = realloc(*path, sizeof(double) * *nbControlPoints * 2);
    if(tmp == NULL) {
        fprintf(stderr, "Error while allocating memories for the path data\n");
        return -1;
    }
    *path = tmp;

    (*path)[2 * (*nbControlPoints - 1)] = xPoint;
    (*path)[(2 * (*nbControlPoints - 1)) + 1] = yPoint;

    return 0;

}


int parseFile(char* file, unsigned int* pWidth, unsigned int* pHeight, double** pPath, unsigned int* pNbControlPoints, double** pCircles, unsigned int* pNbCircles) {

    unsigned int width = 0;
    unsigned int height = 0;

    char* crtPosition = file;
    char* delimPosition = NULL;

    double* circles = NULL;
    unsigned int nbCircles = 0;
    double* path = NULL;
    unsigned int nbControlPoints = 0;

    if((crtPosition = strstr(crtPosition, "width=\"")) == NULL) {
        fprintf(stderr, "Width not found\n");
        return -1;
    }

    crtPosition += 7; /* strlen("width=\"") == 7) */
    width = (unsigned int) strtol(crtPosition, &delimPosition, 10);

    if(crtPosition == delimPosition) {
        fprintf(stderr, "No width value found\n");
        return -1;
    }

    if(*delimPosition != '\"'){
        fprintf(stderr, "\" not balanced for width\n");
        return -1;
    }

    crtPosition = delimPosition + 1;

    if((crtPosition = strstr(crtPosition, "height=\"")) == NULL) {
        fprintf(stderr, "Width not found\n");
        return -1;
    }

    crtPosition += 8; /* strlen("height=\"") == 8) */
    height = (unsigned int) strtol(crtPosition, &delimPosition, 10);

    if(crtPosition == delimPosition) {
        fprintf(stderr, "No height value found\n");
        return -1;
    }

    if(*delimPosition != '\"'){
        fprintf(stderr, "\" not balanced for height\n");
        return -1;
    }

    if((crtPosition = strstr(crtPosition, "inkscape:label=\"camera\"")) == NULL) {
        fprintf(stderr, "The layer \"camera\" was not found\n");
        return -1;
    }

    while((crtPosition = strstr(crtPosition, "<path")) != NULL) {
        char* isCircle = NULL;
        double* tmp = NULL;
        char* newDelim = NULL;

        if((delimPosition = strstr(crtPosition, "/>")) == NULL) {
            fprintf(stderr, "The SVG file is malformed\n");
            free(circles);
            free(path);
            return -1;
        }

        isCircle = strstr(crtPosition, "sodipodi:cx=\"");

        if((isCircle != NULL) && (isCircle < delimPosition)) {
            char* isTranslated = strstr(crtPosition, "transform=\"translate(");
            double xCenter = 0;
            double yCenter = 0;

            if((isTranslated != NULL) && (isTranslated < delimPosition)) {
                isTranslated += 21;
                xCenter = strtod(isTranslated, &newDelim);

                if(isTranslated == newDelim) {
                    fprintf(stderr, "Missing translation x value\n");
                    free(circles);
                    free(path);
                    return -1;
                }
                if(*newDelim != ',') {
                    fprintf(stderr, "Missing ',' for the translation value\n");
                    free(circles);
                    free(path);
                    return -1;
                }

                isTranslated = newDelim + 1;
                yCenter = strtod(isTranslated, &newDelim);

                if(isTranslated == newDelim) {
                    fprintf(stderr, "Missing translation y value\n");
                    free(circles);
                    free(path);
                    return -1;
                }
                if(*newDelim != ')') {
                    fprintf(stderr, "Missing a closing braket for the translation value\n");
                    free(circles);
                    free(path);
                    return -1;
                }
            }

            isCircle += 13;
            xCenter += strtod(isCircle, &newDelim);

            if(isCircle == newDelim) {
                fprintf(stderr, "Missing center x value\n");
                free(circles);
                free(path);
                return -1;
            }
            if(*newDelim != '\"') {
                fprintf(stderr, "\" not balanced for center x value\n");
                free(circles);
                free(path);
                return -1;
            }

            isCircle = strstr(crtPosition, "sodipodi:cy=\"");

            if((isCircle == NULL) || (isCircle > delimPosition)) {
                fprintf(stderr, "Missing center y value\n");
                free(circles);
                free(path);
                return -1;
            }

            isCircle += 13;
            yCenter += strtod(isCircle, &newDelim);

            if(isCircle == newDelim) {
                fprintf(stderr, "Missing center y value\n");
                free(circles);
                free(path);
                return -1;
            }
            if(*newDelim != '\"') {
                fprintf(stderr, "\" not balanced for center y value\n");
                free(circles);
                free(path);
                return -1;
            }

            nbCircles++;
            tmp = realloc(circles, sizeof(double) * nbCircles * 2);
            if(tmp == NULL) {
                fprintf(stderr, "Error while allocating space for a new circle\n");
                free(circles);
                free(path);
                return -1;
            }

            circles = tmp;
            circles[2 * (nbCircles - 1)] = xCenter;
            circles[(2 * (nbCircles - 1)) + 1] = yCenter;

        } else {
            if(path != NULL) {
                fprintf(stderr, "There should be only one path in the camera layer\n");
                free(circles);
            }

            if((crtPosition = strstr(crtPosition, "d=\"M ")) == NULL) {
                fprintf(stderr, "Missing path data\n");
                free(circles);
                return -1;
            }
            if((newDelim = strstr(crtPosition, " z\"")) == NULL) {
                fprintf(stderr, "Missing path data\n");
                free(circles);
                return -1;
            }

            crtPosition += 5;
            if(parsePathPoint(&crtPosition, &path, &nbControlPoints) == -1) {
                free(circles);
                return -1;
            }

            while(((crtPosition = strstr(crtPosition, " C ")) != NULL) && (crtPosition < newDelim)) {
                crtPosition += 3;
                if(parsePathPoint(&crtPosition, &path, &nbControlPoints) == -1) {
                    free(path);
                    free(circles);
                    return -1;
                }

                crtPosition += 1;
                if(parsePathPoint(&crtPosition, &path, &nbControlPoints) == -1) {
                    free(path);
                    free(circles);
                    return -1;
                }

                crtPosition += 1;
                if(parsePathPoint(&crtPosition, &path, &nbControlPoints) == -1) {
                    free(path);
                    free(circles);
                    return -1;
                }
            }

        }

        crtPosition = delimPosition + 2;
    }

    *pWidth = width;
    *pHeight = height;
    *pPath = path;
    *pNbControlPoints = nbControlPoints;
    *pCircles = circles;
    *pNbCircles = nbCircles;

    return 0;

}


int main(int argc, char* argv[]) {

    int fd = -1;
    char* file = NULL;
    unsigned int width = 0;
    unsigned int height = 0;
    double* path = NULL;
    unsigned int nbControlPoints = 0;
    double* circles = NULL;
    unsigned int nbCircles = 0;
    unsigned int i = 0;
    int pid = -1;


    if(argc != 2) {
        fprintf(stderr, "Usage: %s filename.svg\n", argv[0]);
        return EXIT_FAILURE;
    }

    if((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("Error while opening the file");
        return EXIT_FAILURE;
    }

    if((file = readFile(fd)) == NULL)
        return EXIT_FAILURE;

    close(fd);

    if(parseFile(file, &width, &height, &path, &nbControlPoints, &circles, &nbCircles) == -1) {
        free(file);
        return EXIT_FAILURE;
    }

    printf("Width: %u\n", width);
    printf("Height: %u\n", height);

    for(; i < nbCircles; i++)
        printf("Circle %u: x: %f  y: %f\n", i, circles[2 * i], circles[(2 * i) + 1]);

    for(i = 0; i < (nbControlPoints - 3); i+=3)
        printf("Cubic Bezier Curve %i: P0:%f,%f  P1:%f,%f  P2:%f,%f  P3:%f,%f\n", i/3, path[2 * i], path[(2 * i) + 1], path[(2 * i) + 2], path[(2 * i) + 3], path[(2 * i) + 4], path[(2 * i) + 5], path[(2 * i) + 6], path[(2 * i) + 7]);

    free(path);
    free(circles);
    free(file);

    if(mkfifo("/tmp/tmp.png", S_IRUSR | S_IWUSR) == -1) {
        perror("Error while creating the fifo");
        return EXIT_FAILURE;
    }

    if((pid = fork()) == -1) {
        perror("Error while forking process");
        return EXIT_FAILURE;
    } else if(pid == 0) {
        char*args[5];
        args[0] = "inkscape";
        args[1] = "-e";
        args[2] = "/tmp/tmp.png";
        args[3] = argv[1];
        args[4] = NULL;
        execvp(args[0], args);
        perror("Error while launching Inkscape");
        return EXIT_FAILURE;
    }

    if((pid = fork()) == -1) {
        perror("Error while forking process");
        return EXIT_FAILURE;
    } else if(pid == 0) {
        int length = strlen(argv[1]);
        char* args[4];
        args[0] = "convert";
        args[1] = "/tmp/tmp.png";
        argv[1][length-3] = 'b';
        argv[1][length-2] = 'm';
        argv[1][length-1] = 'p';
        args[2] = argv[1];
        args[3] = NULL;
        execvp(args[0], args);
        perror("Error while launching convert");
        return EXIT_FAILURE;
    }

    printf("Launched the export\n");
    waitpid(pid, NULL, 0);
    printf("Export done\n");

    return EXIT_SUCCESS;

}
