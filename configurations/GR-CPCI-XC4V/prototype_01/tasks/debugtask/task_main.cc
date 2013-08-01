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

/*BCD Tabelle*/
#define BCD0 48
#define BCD1 49
#define BCD2 50
#define BCD3 51
#define BCD4 52
#define BCD5 53
#define BCD6 54
#define BCD7 55
#define BCD8 56
#define BCD9 57

void ByteToBcd( char *byte );

void _32BitToBcd( char value, char *bcd ) {
    bcd[ 0 ] = value % 10;
    ByteToBcd( &bcd[ 0 ] );
    value = value / 10;

    bcd[ 1 ] = value % 10;
    ByteToBcd( &bcd[ 1 ] );
    value = value / 10;

    bcd[ 2 ] = value % 10;
    ByteToBcd( &bcd[ 2 ] );
    value = value / 10;

    bcd[ 3 ] = value % 10;
    ByteToBcd( &bcd[ 3 ] );
    value = value / 10;

    bcd[ 4 ] = value % 10;
    ByteToBcd( &bcd[ 4 ] );
    value = value / 10;

    bcd[ 5 ] = value % 10;
    ByteToBcd( &bcd[ 5 ] );
    value = value / 10;

    bcd[ 6 ] = value % 10;
    ByteToBcd( &bcd[ 6 ] );
    value = value / 10;

    bcd[ 7 ] = value % 10;
    ByteToBcd( &bcd[ 7 ] );
    value = value / 10;

    bcd[ 8 ] = value % 10;
    ByteToBcd( &bcd[ 8 ] );
    value = value / 10;

    bcd[ 9 ] = value % 10;
    ByteToBcd( &bcd[ 9 ] );
    value = value / 10;
}

void ByteToBcd( char *byte ) {
    if ( *byte == 0 ) {
        *byte = BCD0;
    }
    else if ( *byte == 1 ) {
        *byte = BCD1;
    }
    else if ( *byte == 2 ) {
        *byte = BCD2;
    }
    else if ( *byte == 3 ) {
        *byte = BCD3;
    }
    else if ( *byte == 4 ) {
        *byte = BCD4;
    }
    else if ( *byte == 5 ) {
        *byte = BCD5;
    }
    else if ( *byte == 6 ) {
        *byte = BCD6;
    }
    else if ( *byte == 7 ) {
        *byte = BCD7;
    }
    else if ( *byte == 8 ) {
        *byte = BCD8;
    }
    else if ( *byte == 9 ) {
        *byte = BCD9;
    }
}

void ByteToHex( char byte, char* buffer ) {

    char nibble;

    buffer[ 0 ] = 0x30;
    buffer[ 1 ] = 0x78;

    // calc high nibble
    nibble = ( byte ) >> 4;
    if ( nibble < 10 ) {
        buffer[ 2 ] = nibble + 0x30;
    }
    else {
        buffer[ 2 ] = nibble + 0x41 - 10;
    }

    // calc low nibble
    nibble = ( byte & 0x0f );
    if ( nibble < 10 ) {
        buffer[ 3 ] = nibble + 0x30;
    }
    else {
        buffer[ 3 ] = nibble + 0x41 - 10;
    }
}

void processLine( char * lineBuffer, char * lineEnd, int serial_id ) {
    char pointer = 0;
    char hexDigit[ 4 ];
    int commandIndex = 0xff;

#define COMMAND_LENGTH 5
    char commands[][ COMMAND_LENGTH ] = { "help", "top", "task" };

    // find end of first word
    while ( ( pointer <= *lineEnd ) && ( lineBuffer[ pointer ] != 0x20 ) ) {
        pointer++;
    }

    //	ByteToHex(pointer,hexDigit);
    //	fwrite(hexDigit,1,4,serial_id);
    //	ByteToHex(*lineEnd,hexDigit);
    //	fwrite(hexDigit,1,4,serial_id);
    //	fwrite(lineBuffer,1,pointer,serial_id);

    // find the command given
    for ( int i = 0; i < sizeof( commands ) / COMMAND_LENGTH; i++ ) {
        for ( int j = 0; j <= pointer; j++ ) {
            if ( lineBuffer[ j ] != commands[ i ][ j ] ) {
                break;
            }
            if ( commands[ i ][ j + 1 ] == 0x00 ) {
                commandIndex = i;
                break;
            }
        }
    }

    //ByteToHex(commandIndex,hexDigit);
    //fwrite(hexDigit,1,4,serial_id);

    char msg1[] = "\n\rHello World";
    char taskCmd[][ 7 ] = { "stop", "resume" };
    int sd, startPtr, taskId;

    switch ( commandIndex ) {
        case 0:
            fwriteString( "\n\rcommands: help, top, task stop #, task resume #.", serial_id );
            break;
        case 1:

            fwriteString( "\n\rtrying to read /proc/info\r\n", serial_id );

            char theInput[ 13 ];
            sd = fopen( "proc/info" );

            if ( sd < 0 ) {
                fwriteString( "\n\rERROR: could not open /proc/info! aborting ...\n\r", serial_id );
                break;
            }

            int readBytes;
            do {
                readBytes = fread( theInput, 12, 1, sd ); // we want to read 12 bytes
                if ( readBytes < 0 ) {
                    // error condition
                    fwriteString( "\n\rERROR: could not read /proc/info! aborting ...\n\r", serial_id );
                }
                else {
                    // write as many bytes we got
                    fwrite( theInput, 1, readBytes, serial_id );
                }
            }
            while ( readBytes >= 12 );

            fwriteString( "\n\rdone!", serial_id );
            fclose( sd );
            break;
        case 2:
            fwriteString( "\n\rTaskmanger\r\n", serial_id );

            startPtr = ++pointer;
            while ( ( pointer <= *lineEnd ) && ( lineBuffer[ pointer ] != 0x20 ) ) {
                pointer++;
            }

            commandIndex = -1;
            // find the command given
            for ( int i = 0; i < sizeof( taskCmd ) / 7; i++ ) {
                for ( int j = 0; j <= pointer; j++ ) {
                    if ( lineBuffer[ startPtr + j ] != taskCmd[ i ][ j ] ) {
                        break;
                    }
                    if ( taskCmd[ i ][ j + 1 ] == 0x00 ) {
                        commandIndex = i;
                        break;
                    }
                }
            }

            startPtr = ++pointer;

            while ( ( pointer < *lineEnd - 1 ) && ( lineBuffer[ pointer ] != 0x20 ) ) {
                pointer++;
            }

            for ( taskId = 0; startPtr <= pointer; startPtr++ ) {
                taskId *= 10;
                taskId = taskId + ( lineBuffer[ startPtr ] - 48 );
            }

            switch ( commandIndex ) {
                case 0:
                    fwriteString( "stopping task", serial_id );

                    task_stop( taskId );
                    break;
                case 1:
                    fwriteString( "resuming task", serial_id );
                    task_resume( taskId );
                    break;

                default:
                    fwriteString( "unknown command: try stop or resume", serial_id );
                    break;
            }

            break;
        default:
            fwriteString( "\n\rcommand or filename not found", serial_id );
            break;
    }

}

extern "C"int task_main()
{

    int serial_id;
    char theInput [10]; char c;
    char lineEnd = 0;
    char lineBuffer [256];
    char hexDigit [4];

    sleep(5000);

    serial_id = fopen("dev/comm/serial0");
    printf("Serial id: %d\n\r", serial_id);

    // Init Terminal
    c = 0x1b;
    fwrite(&c,1,1,serial_id);
    fwrite("[1;32;40m",1,9,serial_id);

    fwrite(&c,1,1,serial_id);
    fwrite("[2J",1,3,serial_id);

    fwrite("fake_bash:~# ",1,13,serial_id);

    // clear the read data
    for (int j = 0; j<10; j++) {
        theInput[j] = 0;
    }

    while(1)
    {
        fread(theInput,10,1,serial_id);

        // output char by char. so we can find a CR
        for (int j = 0; j<10; j++) {
            if (theInput[j] == 0) {
                break;
            }

            // copy the input into the line buffer
            lineBuffer[lineEnd++] = theInput[j];

            //ByteToHex(theInput[j],hexDigit);
            //fwrite(hexDigit,1,4,serial_id);

            switch (theInput[j]) {
                case 8: // Backspace
                lineEnd --;
                if (lineEnd>0) {
                    fwrite(&(theInput[j]),1,1,serial_id);
                    fwrite(&c,1,1,serial_id);
                    fwrite("[K",1,2,serial_id);
                    lineEnd--;
                }
                //fwrite("this is backspace!!",1,19,serial_id);
                break;
                case 13: // CR
                lineEnd--; // remove CR from line
                processLine(lineBuffer, &lineEnd, serial_id);
                fwrite("\n\r",2,1,serial_id);
                fwrite("fake_bash:~# ",1,13,serial_id);

                lineEnd = 0;
                break;
                default:
                fwrite(&(theInput[j]),1,1,serial_id);
                //_32BitToBcd(theInput[j],theCommand);
                break;
            }
        }

        // clear the read data
        for (int j = 0; j<10; j++) {
            theInput[j] = 0;
        }

        sleep(2000);

    }
}

