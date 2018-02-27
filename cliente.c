//*******************************************************
//Laura Mateos Fonseca 45133141B			*
// Miguel Prieto Gonzalez 70957907D			*
//*******************************************************
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


#define PUERTO 23911
#define TAM_BUFFER 2048
#define BUFFERSIZE	2048
#define TIMEOUT 6
#define RETRIES 5
void udp(char* host, char* fich);
void tcp(char* host, char* fich);
void handler()
{
 printf("Alarma recibida \n");
}



main(int argc, char *argv[])
{

        if(argc!=4) {
        printf("Faltan args||args invalidos.\n");
        exit(1); }
        else
        {
            if(strcmp(argv[2],"TCP")==0)
            {
                tcp(argv[1],argv[3]);
            }
            else if(strcmp(argv[2],"UDP")==0)
            {
                udp(argv[1],argv[3]);
            }
        }

        exit(0);

}




//--------------------FUNCION PARA UDP--

void udp(char* host, char* fich)
{
    FILE *fpsalida, *fp;
    char fichero[BUFFERSIZE];

	int x, errcode;
	int retry = RETRIES;		/* holds the retry count */
    int s;				/* socket descriptor */
    long timevar;                       /* contains time returned by time() */
    struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in servaddr_in;	/* for server socket address */
    int	addrlen, n_retry;
    struct sigaction vec;
    struct addrinfo hints, *res;
    char buff[BUFFERSIZE];
    char *ordenes=fich;

    fp = fopen(ordenes, "r");
    if(fp == NULL)
    {
       printf("No se puede abrir el fichero.\n");
       exit(1);
    }

    	/* Create the socket. */
	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror("clienteUDP");
		fprintf(stderr, "%s: unable to create socket\n", "clienteUDP");
		exit(1);
	}

    /* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

	myaddr_in.sin_family = AF_INET;
	myaddr_in.sin_port = 0;
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror("clienteUDP");
		fprintf(stderr, "%s: unable to bind socket\n", "clienteUDP");
		exit(1);
	   }

    addrlen = sizeof(struct sockaddr_in);
    if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
            perror("clienteUDP");
            fprintf(stderr, "%s: unable to read socket address\n", "clienteUDP");
            exit(1);
    }

            /* Print out a startup message for the user. */
    time(&timevar);

    printf("Connected to %s on port %u at %s", host, ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));

	/* Set up the server address. */
	servaddr_in.sin_family = AF_INET;
		/* Get the host information for the server's hostname that the
		 * user passed in.
		 */
    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET;

    errcode = getaddrinfo (host, NULL, &hints, &res);
        if (errcode != 0){
                /* Name was not found.  Return a
                 * special value signifying the error. */
            fprintf(stderr, "%s: No es posible resolver la IP de %s\n",
                    "clienteUDP", host);
            exit(1);
          }
        else {
                /* Copy address of host */
            servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
         }
     freeaddrinfo(res);
     /* puerto del servidor en orden de red*/
	 servaddr_in.sin_port = htons(PUERTO);

   /* Registrar SIGALRM para no quedar bloqueados en los recvfrom */
    vec.sa_handler = (void *) handler;
    vec.sa_flags = 0;
        if ( sigaction(SIGALRM, &vec, (struct sigaction *) 0) == -1) {
                perror(" sigaction(SIGALRM)");
                fprintf(stderr,"%s: unable to register the SIGALRM signal\n", "clienteUDP");
                exit(1);
            }

    sprintf(fichero, "%d.txt", ntohs(myaddr_in.sin_port));
    fpsalida = fopen(fichero, "a");
    if(fpsalida == NULL)
    {
       printf("No se puede abrir el fichero.\n");
       exit(1);
    }

    strcpy(buff, "newPort");
           if (sendto (s, buff, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1)
       {
          perror("clienteUDP");
          fprintf(stderr, "%s: unable to send request\n", "clienteUDP");
          exit(1);
       }

    //*********************

    do
    {
       strcpy(buff, "CONECTADO");
       n_retry=RETRIES;
       while (n_retry > 0) {
            /* Send the request to the nameserver. */
       if (sendto (s, buff, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1)
       {
          perror("clienteUDP");
          fprintf(stderr, "%s: unable to send request\n", "clienteUDP");
          exit(1);
       }

       alarm(TIMEOUT);
       /* Wait for the reply to come in. */
       if (recvfrom (s, buff, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in, &addrlen) == -1)
       {
       if (errno == EINTR)
       {
          /* Alarm went off and aborted the receive.
          */
          printf("attempt %d (retries %d).\n", n_retry, RETRIES);
          n_retry--;
       }
       else
       {
          printf("Unable to get response from");
          exit(1);
       }
       }
       else
       {
          alarm(0);
          fprintf(fpsalida, "%s %s\n", buff, "Simple Mail Transfer Service Ready");
          break;
       }
    }
    if (n_retry == 0)
    {
    printf("Unable to get response from");
    printf(" %s after %d attempts.\n", host, RETRIES);
    }
    }while(strcmp(buff, "220")!=0);



    do
    {
        if(fgets(buff, BUFFERSIZE, fp)!=NULL)
        {
            x=strlen(buff)-1;
            while(1){
                if(buff[x]<20)
                {
                }
                else{
                    x++;
                    break;
                }
                x--;

            }
        buff[x]='\0';
        fflush(stdin);


        }
        else
        {
            printf("\nc :EOF enviando -QUIT- ");
            strcpy(buff,"QUIT");
        }

        n_retry=RETRIES;

        while (n_retry > 0) {
            /* Send the request to the nameserver. */
            if (sendto (s, buff, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in,
                    sizeof(struct sockaddr_in)) == -1) {
                    perror("clienteUDP");
                    fprintf(stderr, "%s: unable to send request\n", "clienteUDP");
                    exit(1);
                }

            alarm(TIMEOUT);
            /* Wait for the reply to come in. */
            if (recvfrom (s, buff, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in, &addrlen) == -1) {
                if (errno == EINTR) {

                     printf("attempt %d (retries %d).\n", n_retry, RETRIES);
                     n_retry--;
                        }
                else  {
                    printf("Unable to get response from");
                    exit(1);
                    }
                  }
            else {
                alarm(0);
                /* response. */
                    if(strcmp(buff, "500")==0)
                    {
                    fprintf(fpsalida, "%s %s\n", buff, "Syntax error");
             
                    }
                    else if(strcmp(buff, "250")==0)
                    {
                    fprintf(fpsalida, "%s %s\n", buff, "OK");
                    
                    }
                    else if(strcmp(buff, "354")==0){
                    fprintf(fpsalida, "%s %s\n", buff, "Start mail input, end with.");
                    




                    do
                    {
                               if(fgets(buff, BUFFERSIZE, fp)!=NULL)
                                {
                                    x=strlen(buff)-1;
                                    while(1){
                                        if(buff[x]<20)
                                        {
                                        }
                                        else{
                                            x++;
                                            break;
                                        }
                                        x--;

                                    }
                                buff[x]='\0';
                                
                                fflush(stdin);


                                }
                                else
                                {
                                    printf("\nc :EOF enviando -QUIT- ");
                                    strcpy(buff,".");
                                }
                               n_retry=RETRIES;
                               while (n_retry > 0) {
                           /* Send the request to the nameserver. */
                                   if (sendto (s, buff, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in,
                                    sizeof(struct sockaddr_in)) == -1) {
                                    perror("clienteUDP");
                                    fprintf(stderr, "%s: unable to send request\n", "clienteUDP");
                                    exit(1);
                                   }
                                    alarm(TIMEOUT);

                            if (recvfrom (s, buff, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in, &addrlen) == -1) {
                                if (errno == EINTR) {

                                    printf("attempt %d (retries %d).\n", n_retry, RETRIES);
                                    n_retry--;
                                }
                                else  {
                                    printf("Unable to get response from");
                                    exit(1);
                                }
                          }
                            else { // Recvfrom sin error
                                alarm(0);
                                /* Response. */
                                if(strcmp(buff, "250")==0)
                                {
                                   fprintf(fpsalida, "%s %s\n", buff, "OK");
                                 }
                                break;
                            }
                    }

                    if (n_retry == 0) {
                       printf("Unable to get response from");
                       printf(" %s after %d attempts.\n", host, RETRIES);
                       }


                }while(strcmp(buff, "250")!=0);
                    }

                break;
                }
      }

        if (n_retry == 0) {
           printf("Unable to get response from");
           printf(" %s after %d attempts.\n", host, RETRIES);
           }


    }while(strcmp(buff, "221")!=0);
    fprintf(fpsalida, "%s %s\n", buff, "Service closing transmission channel");
    fclose(fpsalida);
    return;


}



//--------------------FUNCION PARA TCP--
void tcp(char* host, char* fich)
{
    FILE *fp, *fpsalida;
    char fichero[TAM_BUFFER];
    int x;
    int s;				/* connected socket descriptor */
   	struct addrinfo hints, *res;
    long timevar;			/* contains time returned by time() */
    struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in servaddr_in;	/* for server socket address */
	int addrlen, i, j, errcode;
    /* This example uses TAM_BUFFER byte messages. */
	char buf[TAM_BUFFER];
    char *ordenes=fich;

    fp = fopen(ordenes, "r");
    if(fp == NULL)
    {
       printf("No se puede abrir el fichero.\n");
       exit(1);
    }


	/* Create the socket. */
	s = socket (AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror("clienteTCP");
		fprintf(stderr, "%s: unable to create socket\n", "clienteTCP");
		exit(1);
	}

	/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

	/* Set up the peer address to which we will connect. */
	servaddr_in.sin_family = AF_INET;

	/* Get the host information for the hostname that the
	 * user passed in. */
      memset (&hints, 0, sizeof (hints));
      hints.ai_family = AF_INET;
      errcode = getaddrinfo (host, NULL, &hints, &res);
        if (errcode != 0){
                /* Name was not found.  Return a
                 * special value signifying the error. */
            fprintf(stderr, "%s: No es posible resolver la IP de %s\n",
                    "clienteTCP", host);
            exit(1);
            }
        else {
            /* Copy address of host */
            servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
            }

        freeaddrinfo(res);
        /* puerto del servidor en red*/
        servaddr_in.sin_port = htons(PUERTO);






        if (connect(s, (const struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror("clienteTCP");
		fprintf(stderr, "%s: unable to connect to remote\n", "clienteTCP");
		exit(1);
        }

        addrlen = sizeof(struct sockaddr_in);
        if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
            perror("clienteTCP");
            fprintf(stderr, "%s: unable to read socket address\n", "clienteTCP");
            exit(1);
         }

	/* Print out a startup message for the user. */
        time(&timevar);

	printf("Connected to %s on port %u at %s", host, ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));

	sprintf(fichero, "%d.txt", ntohs(myaddr_in.sin_port));
    fpsalida = fopen(fichero, "a");
    if(fpsalida == NULL)
    {
       printf("No se puede abrir el fichero.\n");
       exit(1);
    }

    //////////////////////************************//////////////************//*/*//*//*//*///*//**/****//*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*
    do
    {
    recv(s, buf, TAM_BUFFER, 0);
    }
    while(strcmp(buf, "220")!=0);
    fprintf(fpsalida, "%s %s\n", buf, "Simple Mail Transfer Service Ready");





do
{
    if(fgets(buf, TAM_BUFFER, fp)!=NULL)
    {
        x=strlen(buf)-1;
        while(1){
            if(buf[x]<20)
            {
            }
            else{
                x++;
                break;
            }
            x--;

        }
    buf[x]='\0';
    fflush(stdin);


    }
    else
    {
        printf("\nc :EOF enviando -QUIT- ");
        strcpy(buf,"QUIT");
    }


if (send(s, buf, TAM_BUFFER, 0) != TAM_BUFFER)
{
   fprintf(stderr, "Send error");
   exit(1);
}

recv(s, buf, TAM_BUFFER, 0);


if(strcmp(buf, "500")==0)
{
   fprintf(fpsalida, "%s %s\n", buf, "Syntax error");
}
else if(strcmp(buf, "250")==0)
{
   fprintf(fpsalida, "%s %s\n", buf, "OK");
}
else if(strcmp(buf, "354")==0)
{
   fprintf(fpsalida, "%s %s\n", buf, "Start mail input, end with.");
   do
   {
              if((fgets(buf, TAM_BUFFER, fp)!=NULL))
            {
                x=strlen(buf)-1;
                while(1){
                        if(buf[x]<20)
                        {


                        }
                        else
                        {
                            x++;
                            break;
                        }
                        x--;

                    }
                buf[x]='\0';
            }
            else
            {
                strcpy(buf,".");
            }
      if (send(s, buf, TAM_BUFFER, 0) != TAM_BUFFER)
	{
	   fprintf(stderr, "Send error");
	   exit(1);
	}
   } while(strcmp(buf, ".")!=0);
   recv(s, buf, TAM_BUFFER, 0);
   if(strcmp(buf, "250")==0)
   {
      fprintf(fpsalida, "%s %s\n", buf, "OK");

   }
}
}while(strcmp(buf, "221")!=0);



fprintf(fpsalida, "%s %s\n", buf, "Service closing transmission channel");


fclose(fp);
fclose(fpsalida);

	if (shutdown(s, 1) == -1) {
		perror("clienteTCP");
		fprintf(stderr, "%s: unable to shutdown socket\n", "clienteTCP");
		exit(1);
	}



    /* Print message indicating completion of task. */
	time(&timevar);
	printf("\nc:All done at %s", (char *)ctime(&timevar));
	return;
}
