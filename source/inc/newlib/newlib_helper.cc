/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "newlib_helper.hh"

int close(int file) {
    return -1;
}

int fstat(int file, struct stat *st) {
    return 0;
}

int getpid() {
    return -1;
}

int isatty(int file) {
    return 1;
}

int kill(int pid, int sig) {
    return (-1);
}

int lseek(int file, int ptr, int dir) {
    return 0;
}

void print(char* string) {

}

void noop() {

}

