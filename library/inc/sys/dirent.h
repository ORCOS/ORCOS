/*
 * dirent.h
 *
 *  Created on: 19.04.2015
 *      Author: Daniel Baldin
 */

#ifndef LIBRARY_INC_SYS_DIRENT_H_
#define LIBRARY_INC_SYS_DIRENT_H_


struct dirent {
  int               d_ino;              /* file serial number */
  unsigned char     d_type;             /* file type  */
  char              d_name[256 + 1];    /* file name */
};


typedef struct __DIR {
  int     pos;
  int     remainingBytes;
  int     dd_fd;   /* opened filde descriptor */
  char*   dd_buf;  /* buffer given by user */
  int     dd_len;  /* size of buf */
  int     dd_loc;  /* current location (entry number) */
  int     dd_seek;
  unsigned __flags;
} DIR;


int            closedir(DIR *);
DIR           *opendir(const char *);
struct dirent *readdir(DIR *);
int            readdir_r(DIR *, struct dirent *, struct dirent **);
void           rewinddir(DIR *);
void           seekdir(DIR *, long int);
long int       telldir(DIR *);

#endif /* LIBRARY_INC_SYS_DIRENT_H_ */
