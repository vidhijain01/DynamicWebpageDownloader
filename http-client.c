/*
 * http-client.c
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>  //added for the gethostbyname() function

static void die(const char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char **argv)
{
    // ignore SIGPIPE so that we don't terminate when we call
    // send() on a disconnected socket.
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        die("signal() failed");

    if (argc != 4) 
    { 
        fprintf(stderr, "usage: %s <host-name> <port-number> <file-path>\n", argv[0]);
        exit(1);
    }

    unsigned short port = atoi(argv[2]);
    char *filepath = argv[3];
    struct hostent *he;
    char *serverName = argv[1];

    // get server ip from server name
     if ((he = gethostbyname(serverName)) == NULL) 
     {
	    die("gethostbyname failed");
     }
     char *serverIP = inet_ntoa(*(struct in_addr *)he->h_addr);
    // Create a socket for TCP connection

    int sock; // socket descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("socket failed");

    // Construct a server address structure

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr)); // must zero out the structure
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(serverIP);
    servaddr.sin_port        = htons(port); // must be in network byte order

    // Establish a TCP connection to the server

    if (connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        die("connect failed");

    // Wrap the socket descripter into a file pointer
    FILE *input = fdopen(sock, "r");

    char buf[4096];

    int num = snprintf(buf, sizeof(buf), "GET %s HTTP/1.0\r\nHost: %s:%d\r\n\r\n ", filepath, serverName, port);
    int len = strlen(buf);

    if (send(sock, buf, len, 0) != len)
        die("send failed");

    if(fgets(buf, sizeof(buf), input) == NULL)
	    die("fgets failed");

    //check the first line for the protocol version
    if(strncmp("200", buf + 9, 3) != 0)
    {
	    printf("%s", buf);
	    die("invalid protocol version");
    }
    
    //skip through all the headers till you reach the new line
    while(fgets(buf, sizeof(buf), input))
    {
	//if you find a new line leave the loop
	if(strncmp(buf, "\r\n", 2) == 0)
		break;

	//if buf is NULL, that means no new line was found so no content was found, exit
	if(buf == NULL)
	{
		die("no file content found");
	}
    }


    //split the filepath to get just the name and store it in a pointer
    char *filename = strrchr(filepath, '/');

    //add 1 to get rid of the slash
    filename = filename + 1; 

    //create an output file to write into with the name provided
    //switch to fread so that you can read binary data
    //read in 4096 chunks at a time
    //fopen bc we're not using the file descriptor anymore
    FILE *output = fopen(filename, "w");

    int x;
    while((x = fread(buf, 1, sizeof(buf), input)) > 0)
    {
	fwrite(buf, x, 1, output);  
    }


    //close everything and clean
    fclose(input);
    fclose(output);
    close(sock);
    
    return 0;
}


//--------------------NOTES FROM LECTURE----------------------
//use fgets or some gets to get the file
//write it to the disk
//lab prompt gives a piece of code to convert url to ip address
//you are sending a get request
//protocol version HTTP/1.0
//GET (the url) HTTP/1.0
//File *input = fdopen(socket_descriptor, "r")
//call fgets rather than recv, and send the input file from fdopen
//ex. fgets(buf, sizeof(buf), input) 
//read the first line and make sure you got 200 or whatever, if not deal with error conditions
//read through all the lines until you hit the blank line
//the blank line will be 2 bytes: "\r\n"
//aka compare everyline with "\r\n" till you find a match
//once you find a match you know you are about to read the content
//at this point, switch to fread so that you can read in both texts and binary(aka image, movie, etc)
//code is exactly the same as tcp-client up till connect
//send get request rather than line from stdin
//fread will read 4096 byte chunks at a time
//as you recieve each chunk from fread, you open a file for writing and write it out to the disk 
//keep reading chunks from socket until you read nothing and then yk ur done 
//tcp sender look at fread and send in while loop, instead of sending here, you will right it out to the disk 
//in the send function dont send sizeof(buf), send what freads returns

