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

int recieveFile(int socket, struct sockaddr_in * sin, int sLength)
{ 
    double executionTime = 0.0;
    double dataRate = 0.0;
    clock_t begin = clock();
    int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
    char imagearray[1024],verify = '1';
    FILE *fp;
    stat = recvfrom(socket, &size, sizeof(int), 0, sin, &sLength); 
    printf("File size: %i\n",size);

    fp = fopen("output.jpg", "w");    // Can recieve any kind of file (txt, jpg, mp4)

    if( fp == NULL) 
    {
        printf("Error has occurred. File could not be opened\n");
        return -1; 
    }

    int flag = 1, flag2 = 0;
    int nbrecv = 0;   
    int count = 0; 
    while(flag == 1)
    {
        memset(imagearray, '\0', BUF_SIZE);

        if((nbrecv = recvfrom(socket, imagearray, BUF_SIZE, 0, sin, &sLength)) == -1){
            fprintf(stderr, "fail while receiving data! \n");
            exit(-1);
            return 0;
        }


        count += nbrecv;
        printf("Packet Size: = %d\n", nbrecv);
        printf("Packet Number: %i\n",packet_index);
        packet_index++;

        fwrite(imagearray, 1, nbrecv, fp);
        
        delay(1000);   // 1000us delay

        if(nbrecv<1024){
            flag = 0;
        }
    }

    fclose(fp);
    printf("File Successfully Recieved!\n");
    clock_t end = clock();
    executionTime += (double)(end-begin) / CLOCKS_PER_SEC;
    printf("The total execution time: %lf Seconds\n", executionTime);
    dataRate = size/executionTime;
    printf("Data Rate: %lf Bytes/Second\n", dataRate);
    return 1;
}


int main(int argc, char * argv[]){
  
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  int sLength = sizeof(sin);
  char *host;
  char buf[BUF_SIZE];
  int s, state;
  int len;

  if ((argc==2)||(argc == 3)) {
    host = argv[1];
  }
  else {
    fprintf(stderr, "usage: client serverIP [download_filename(optional)]\n");
    exit(1);
  }

  if(argc == 3) {
    fp = fopen(argv[2], "w");
    if (fp == NULL) {
      fprintf(stderr, "Error opening output file\n");
      exit(1);
    }
  }

  /* translate host name into peer's IP address */
  hp = gethostbyname(host);
  if (!hp) {
    fprintf(stderr, "client: unknown host: %s\n", host);
    exit(1);
  }
  else
    printf("Host %s found!\n", argv[1]);

  /* build address data structure */
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);
  

  /* create socket */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("client: socket");
    exit(1);
  }
  
  printf("Client will get data from to %s:%d.\n", argv[1], SERVER_PORT);
  
  while(1)
  {
      /* send message to server */  
      fgets(buf, sizeof(buf), stdin);
      buf[BUF_SIZE-1] = '\0';
      len = strlen(buf) + 1;
      if (sendto(s, buf, len, 0, (struct sockaddr *)&sin, sLength) < 0) { 
        perror("Client: sendto()"); 
        return 0; 
      }
      
      if(buf[0]=='G' && buf[1]=='E' && buf[2]=='T')
      {
          memset(buf, 0, sizeof(buf));
          state = recieveFile(s, &sin, sLength);
      }  
      else
      {   
          printf("Invalid Command Passed! Send GET to recieve file!\n");
      }

      // recieving bye
      memset(buf, 0, sizeof(buf));
      len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &sLength);
      fputs(buf, stdout);
      printf("\n");

  }
  
}