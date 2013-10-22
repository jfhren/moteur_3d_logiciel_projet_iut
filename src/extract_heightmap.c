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

int readBMPInfo(int fd, unsigned int* width, unsigned int* height, off_t* offset) {

    if(lseek(fd, 10, SEEK_SET) == -1) {
        perror("Error while seeking the BMP file for the offset");
        return -1;
    }

    if(read(fd, offset, 4) != 4) {
        perror("Error while reading the offset");
        return -1;
    }

    if(lseek(fd, 4, SEEK_CUR) == -1) {
        perror("Error while seeking the BMP file fort the width");
        return -1;
    }

    if(read(fd, width, 4) != 4) {
        perror("Error while reading the width");
        return -1;
    }

    if(read(fd, height, 4) != 4) {
        perror("Error while reading the height");
        return -1;
    }

    return 0;

}


int main(int argc, char* argv[]) {

    int fd = -1;
    unsigned int height = 0;
    unsigned int width = 0;
    off_t offset = 0;
    unsigned int i = 0;
    unsigned int pixel = 0;

    if(argc != 2) {
        fprintf(stderr, "Usage: %s filename.bmp\n", argv[0]);
        return EXIT_FAILURE;
    }

    if((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("Error while opening file");
        return EXIT_FAILURE;
    }

    if(readBMPInfo(fd, &width, &height, &offset) == -1)
        return EXIT_FAILURE;

    printf("Width: %u\n", width);
    printf("Height: %u\n", height);
    lseek(fd, offset, SEEK_SET);

    printf("The 32 first pixel (ARGB)\n");
    while((read(fd, &pixel, sizeof(unsigned int)) > 0) && (i++ < 32))
        printf("%i: A:%d  R:%d  G:%d  B:%d\n",i, (0xFF000000&pixel)>>24, (0x00FF0000&pixel)>>16, (0x0000FF00&pixel)>>8, (0x000000FF&pixel));

    return EXIT_SUCCESS;

}
