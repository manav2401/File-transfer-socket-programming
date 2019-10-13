#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#define SERVER_PORT 5432
#define BUF_SIZE 1024

// us delay
void delay(int t)
{
    long pause;
    clock_t now,then;

    pause = t*(CLOCKS_PER_SEC/1000000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}

void sendFile(int socket, struct sockaddr * sin, int sLength)
{

   FILE *picture;
   int size, read_size, stat=0, packet_index;
   char send_buffer[1024], read_buffer[256];
   packet_index = 1;

   picture = fopen("input.jpg", "r");		// Can recieve any kind of file (txt, jpg, mp4)
   printf("Getting Picture Size\n");   

    if(picture == NULL) 
    {
    	printf("Error Opening File\n"); 
    } 
	else
   	{
		fseek(picture, 0, SEEK_END);
		size = ftell(picture);
		fseek(picture, 0, SEEK_SET);
		printf("Total File size: %i\n",size);

		//Send File Size
		printf("Sending File Size\n");
		sendto(socket, (void *)&size, sizeof(int), 0, sin, sLength); 

		//Send Picture as Byte Array
		printf("Sending File as Byte Array\n");

		int n=0, count=0;
    	while(!feof(picture))
      	{
       		n = fread(send_buffer, 1, BUF_SIZE, picture);
            count += n;
            printf("Packet Size: = %d\n", n);

            if(sendto(socket, send_buffer, n, 0, sin, sLength) == -1)
            {
                fprintf(stderr, "error while sending data!\n");
                exit(-1);
            }
            printf("Packet Number: %i\n",packet_index);
            packet_index++;
            delay(1000);		// 1000us delay
        }
        printf("%d bytes sent. \n", count);
   }
}

int main(int argc, char * argv[]){

	struct sockaddr_in sin;
	struct sockaddr_storage client_addr;
	char clientIP[INET_ADDRSTRLEN]; /* For IPv4 addresses */
	socklen_t client_addr_len;
	int sLength = sizeof(client_addr);
	char buf[BUF_SIZE];
	int len, s;
	char *host;
	struct hostent *hp;

	/* Create a socket */
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("server: socket");
	exit(1);
	}

	/* build address data structure and bind to all local addresses*/
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

	/* If socket IP address specified, bind to it. */
	if(argc == 2) {
	host = argv[1];
	hp = gethostbyname(host);
	if (!hp) {
	fprintf(stderr, "server: unknown host %s\n", host);
	exit(1);
	}
	memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
	}
	/* Else bind to 0.0.0.0 */
	else
	sin.sin_addr.s_addr = INADDR_ANY;

	sin.sin_port = htons(SERVER_PORT);

	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
	perror("server: bind");
	exit(1);
	}
	else{
	/* Add code to parse IPv6 addresses */
	inet_ntop(AF_INET, &(sin.sin_addr), clientIP, INET_ADDRSTRLEN);
	printf("Server is listening at address %s:%d\n", clientIP, SERVER_PORT);
	}

	printf("Client needs to send \"GET\" to receive the file %s\n", argv[1]);  
	client_addr_len = sizeof(client_addr);

	/* Receive messages from clients*/
	while(1)
	{
		// Recieves GET from Client
		memset(buf, 0, sizeof(buf));
		len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_len);
		inet_ntop(client_addr.ss_family,
		      &(((struct sockaddr_in *)&client_addr)->sin_addr),
		      clientIP, INET_ADDRSTRLEN);
  
  		// Sends a fixed file to client
		printf("String: %s\n", buf);
		if(buf[0]=='G' && buf[1]=='E' && buf[2]=='T')
		{
			memset(buf, 0, sizeof(buf));
			sendFile(s, &client_addr, sLength);
		}
		else
		{
			printf("Client needs to send GET in order to recieve file.\n");
		}

		// Sends BYE at the end
		memset(buf, 0, sizeof(buf));
		strcpy(buf, "BYE");
		sendto(s, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, client_addr_len);

	}

}