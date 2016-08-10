

#include <orcos.h>
#include <stdio.h>
#include <string.h>

#define LF "\r\n"


static char buf[1024];


#define ITERATIONS (2048 * 2)

// entry point
extern "C" int main(int argc, char** argv) __attribute__((used));

extern "C" int main(int argc, char** argv)
{
    volatile int val = 0;

    if (argc == 1) {
        puts("Usage: filetest <filepath>" LF);
        thread_exit(-1);
    }
    int fd = open(argv[1]);

    if (fd > 0) {
        puts("File aready exists.. aborting."LF);
        close(fd);
    }

    fd = create(argv[0], 0);

    // bail out if file not found
    if (fd < 0) {
        puts("Could not create file."LF);
        thread_exit(-1);
    }

   printf("Performing Write/Read Test on %d KB File." LF, (ITERATIONS * 1024) / 1024);

   memset(buf, 0, 1024);

   unint8 start = getTime();


   for (int i = 0; i < ITERATIONS  ; i++ ) {
       int error = write(fd, buf, 1024);
       if (error < 0) {
           printf("Error writing to file: %s."LF, strerror(error));
           close(fd);
           thread_exit(cOk);
       }
   }

   unint8 end = getTime();
   unint4 duration = end - start;

   unint4 us = duration / 1000;
   unint4 ms = us / 1000;
   unint4 byteperms = ((ITERATIONS) * 1024) / ms;
   unint4 kbits = (byteperms * 1000) / 1024;

   if (kbits <= 1024) {
       printf("Write: %d KB/s (%u us)" LF,  kbits, us);
   } else {
       int mbits = kbits / 1024;
       int rem   = kbits - (mbits * 1024);
       printf("Write: %d.%d MB/s (%u us)" LF, mbits, rem, us);
   }

   close(fd);

   fd = open(argv[1]);

   start = getTime();

   for (int i = 0; i < ITERATIONS; i++ ) {
       int error = read(fd, buf, 1024);
       if (error < 0) {
           printf("Error reading from file: %s."LF, strerror(error));
           close(fd);
           thread_exit(cOk);
       }
   }

   end = getTime();

   duration = end - start;

   us        = duration / 1000;
   ms        = us / 1000;
   byteperms = ((ITERATIONS) * 1024) / ms;
   kbits     = (byteperms * 1000) / 1024;

   if (kbits <= 1024) {
       printf("Read : %d KB/s (%u us)" LF,  kbits, us);
   } else {
       int mbits = kbits / 1024;
       int rem   = kbits - (mbits * 1024);
       printf("Read : %d.%d MB/s (%u us)" LF, mbits, rem, us);
   }

}
