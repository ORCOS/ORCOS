#include <orcos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define FTP_PASV 1 << 1

#define DATA_CMD_LIST 1
#define DATA_CMD_STOR 2
#define DATA_CMD_RETR 3

class FTPServer {
private:
    int                 socketfd;
    /* current directory */
    char                cwd[256];
    /* last directory */
    char                lwd[256];

    char                tmp_path[256];
    char                responseMsg[256];

    int                 dirfd;

    char                renameFromFile[256];
    char                renameToFile[256];
    char                dataBuffer[2048];

    int                 ftp_mode; /* current mode (PASV etc) */

    /* Address of remote data socket to send data to in ACTIVE mode
     * set by PORT command
    */
    sockaddr            dataremote;

    sockaddr            datasockaddr;   /* current address of the data socket*/
    int                 datasocket;     /* socket the data thread uses */
    volatile int        data_command;   /* command to be handled by data thread */
    ErrorT              data_result;
    int                 data_fd;

private:
    ErrorT  receiveFile            (int datasock, int fd);
    ErrorT  sendFile               (int datasock, int fd);
    ErrorT  sendDirectoryContents  (int datasock);
    void    sendResponse           (int socket, char* msg, ...);


    char* getFTPPath(char* msgptr);
public:

    FTPServer(int socket);



    /* handler thread */
    void thread_entry();

    /* data connection list thread entry (in PASV mode) */
    void data_thread_entry();
};
