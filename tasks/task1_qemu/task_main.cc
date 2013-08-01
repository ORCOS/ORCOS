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

#include <orcos.hh>

#include "pthread.hh"

pthread_mutex_t uart_lock;

void* thread_main( void* arg ) {
    int id = (int) arg;

    while ( 1 ) {
        pthread_mutex_lock( &uart_lock );

        printf( "START%d ", id );

        sched_yield();

        // print text to UART and sleep
        printf( "Task1.Thread%d says >Hello World!<#############################\n", id );

        pthread_mutex_unlock( &uart_lock );

        sleep( 1 );
    }
}

extern "C" int task_main() {
    pthread_mutexattr_t mutex_attrs;
    pthread_mutexattr_init( &mutex_attrs );
    pthread_mutex_init( &uart_lock, &mutex_attrs );
    pthread_mutexattr_destroy( &mutex_attrs );

    // create a second thread
    pthread_t threadid0;
    pthread_t threadid1;
    pthread_t threadid2;
    pthread_t threadid3;
    pthread_attr_t attr;

    pthread_attr_init( &attr );
    pthread_attr_setstacksize( &attr, 1200 );
//    attr.phase = 0;
//    attr.priority = 0;
//    attr.period = 0;
    pthread_create( &threadid0, &attr, &thread_main, (void*) 0 );
    pthread_create( &threadid1, &attr, &thread_main, (void*) 1 );
    pthread_create( &threadid2, &attr, &thread_main, (void*) 2 );
    pthread_create( &threadid3, &attr, &thread_main, (void*) 3 );

    pthread_attr_destroy( &attr );

    while ( 1 ) {

        pthread_mutex_lock( &uart_lock );

        // print text to UART and sleep
        printf( "Task1.MainThread says >Hello World!<\n" );

        pthread_mutex_unlock( &uart_lock );

        sleep( 2 );

    }

    pthread_mutex_destroy( &uart_lock );
}

