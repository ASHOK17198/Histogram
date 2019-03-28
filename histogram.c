/**********************************************************************
**  histogram.c
**  	Histogramm from Memory Buffer
**  	
**      Version 1.00
**
**  Copyright (C) 2019 Ashok Singh
**
**      This program is free software: you can redistribute it and/or
**   	modify it under the terms of the GNU General Public License
**  	as published by the Free Software Foundation, either version
**  	2 of the License, or (at your option) any later version.
**
**  Refrence : https://github.com/apertus-open-source-cinema/axiom-beta-firmware/blob/master/software/sensor_tools 
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#define	CLOCK_ID    CLOCK_REALTIME

static u_int16_t num_cols = 4096;
static u_int16_t num_rows = 3072;

static inline
double	delta_ns(struct timespec *a, struct timespec *b)
{
	double delta = b->tv_nsec - a->tv_nsec;

	return delta + (b->tv_sec - a->tv_sec) * 1e9;
}

static inline
double	ns_to_s(double val)
{
	return val * 1e-9;
}

static inline
double	delta_s(struct timespec *a, struct timespec *b)
{
	return ns_to_s(delta_ns(a, b));
}

static inline
void histogram(u_int64_t *buffer)
{   
    int hist[4][32] = {{ 0 }};
    u_int64_t *ptr = (u_int64_t*)buffer; 
    __int128_t reservoir = 0;
    
    /*bits in reservoir*/
    int bits = 0; 

    /*taking 64-bits at a time to reduce cache expense*/
    for (int j = 0; j < num_rows; ++j) {
        for (int i = 0; i < num_cols / 2; ++i) {
            if (bits < 24) {
                reservoir = (reservoir << 64) | htobe64(*ptr++);
                bits += 64;
            }

            u_int8_t x = (reservoir >> (bits - 5)) & 31;
            bits -= 12;
            u_int8_t y = (reservoir >> (bits - 5)) & 31;
            bits -= 12;
            
            if (j % 2 != 0) {
                hist[0][x]++;       /* CH[0] */
                hist[1][y]++;       /* CH[1] */
            }
            else {
                hist[2][x]++;       /* CH[2] */
                hist[3][y]++;       /* CH[3] */
            } 
        }
    }
    printf("\n--------------------------------------------");   
    printf("\nBUCKET NO.    R1         G1         G2         B1");
    for(int i = 0; i < 32; ++i) {
        printf("\n%2d   ",i);
        printf(" =  ");
        printf(" %7d     %7d     %7d     %7d ",
               hist[0][i], hist[1][i],
               hist[2][i], hist[3][i]);
    } 
    printf("\n---------------------------------------------");
}

int main(int argc, char *argv[])
{
    struct timespec time_base , now;
    struct stat st;
    size_t size;

    /* Opening raw12 file with read/write permission */
    int fd = open("sources/index.raw12", O_RDWR);
    if (fd == -1) {
	    printf("error opening FILE\n");
	    exit(1);
	}
    
    int rc = fstat(fd, &st);
    size=st.st_size;

    /* Mapping buffer for memory allocation */
    void *buffer = mmap(0, size, PROT_READ|PROT_EXEC,
                        MAP_SHARED, fd, 0);
   
    /*get time for running histogram function */
    clock_gettime(CLOCK_ID, &time_base);
    histogram(buffer);
    clock_gettime(CLOCK_ID, &now);
    
    double delta = delta_s(&time_base, &now);
    printf("\n time = %f",delta);
    
    rc = munmap(buffer, size);
    printf("\n unmap() = %d\n", rc);
    close(fd);

    return 0;
}
