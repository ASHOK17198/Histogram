/**********************************************************************
**  histogram.c
**  Histogramm from Memory Buffer
**  Version 1.00
**
**  Copyright (C) 2019 Ashok Singh
**
**  This program is free software: you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation, either version
**  2 of the License, or (at your option) any later version.
**
**  Refrence : https://github.com/apertus-open-source-cinema/axiom-beta-firmware/blob/master/software/sensor_tools/hist/hist.c 
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

static u_int16_t num_cols = 4096;
static u_int16_t num_rows = 3072;

int main(int argc, char *argv[])
{
    struct timeval  tv1, tv2;
    gettimeofday(&tv1, NULL);
    
    int hist[4][32] = {{ 0 }};
    u_int8_t *buffer;

    struct stat st;
    size_t size;

    int fd = open("sources/index.raw12", O_RDWR);
    if (fd == -1) {
	    printf("error opening FILE\n");
	    exit(1);
	}
    
    int rc = fstat(fd, &st);
    size=st.st_size;

    buffer = mmap(0, size, PROT_READ|PROT_EXEC,
                  MAP_SHARED, fd, 0);
    if (buffer == (u_int8_t *)-1) {
	    printf("error mapping ");
	    exit(2);
	} 
 
    int length = num_rows * num_cols * (3)/2;                           /* 2 - 12 bits equals to 3 - 8 bits */
  
    for(int i = 0; i < length; i += 3) {               
       if (i % (3 * 8092 / 2) < 4096 * (3) / 2) {                      /* ODD ROW */
          hist[0][(buffer[i] >> 3)]++;                            /* CH[0] */
          hist[1][(buffer[i + 1] & 0x0F << 1 |            /* CH[1] */
                   buffer[i + 2] >> 7)]++;          
        } else {                                        /* EVEN ROW*/
          hist[2][(buffer[i] >> 3)]++;                  /* CH[2] */
          hist[3][(buffer[i + 1] & 0x0F << 1 |            /* CH[3] */
                (buffer[i + 2] >> 7))]++ ;               
        }  
    } 

    printf("\n-----------------------------------------------");   
    printf("\nBUCKET NO.    R1        G1       G2        B1");
    
    for(int i = 0; i < 32; ++i) {
       printf("\n%d        ",i);
       printf("   =");
       printf(" %d     %d     %d     %d ",
              hist[0][i], hist[1][i],
              hist[2][i], hist[3][i]);
    } 
  
    printf("\n------------------------------------------------");
    printf("\n");

    rc = munmap(buffer, size);
    printf("\n unmap() = %d\n", rc);
    close(fd);

    gettimeofday(&tv2, NULL);
    printf("\nTotal time = %f seconds\n",
          (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
          (double) (tv2.tv_sec - tv1.tv_sec));

    return 0;
}
   //12582912 
