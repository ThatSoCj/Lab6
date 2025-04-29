/*.........................................................................*/
/*                  TSHLIB.C ------> TSH test program                     */
/*                                                                         */
/*                  By Alexis Graham, April '25               */
/*.........................................................................*/

#include "tshtest.h"
char login[NAME_LEN];
int tshsock;
#define INITIAL_SIZE 10 // random length idk
char* read_input();

int status;
short PORT = 6205;
char *TSHIPAddress = "127.0.0.1";


void tshInIt(short PORT)
{

		   tshsock = connectTsh(PORT);
}



void OpShell()
{;
   u_short this_op;

   printf("TSH_OP_SHELL") ;
   printf("\n----------\n") ;
				/* obtain tuple name, priority, length, */
   printf("\nEnter command string> ") ; /* and the tuple */

   char *commandLine = read_input();
   //you're using the same variable name for a pointer and int
   //int commandLine = strlen(commandLine); <- previous line
   int commandLength = strlen(commandLine); 

   tsh_shell_it out;
   tsh_shell_ot in;

   this_op = htons(TSH_OP_SHELL);
   if (!writen(tshsock, (char *)&this_op, sizeof(this_op))) {
      perror("\nOpShell sending op code error::writen\n");
      getchar();
      return;
   }
   out.length = htonl(commandLength);

   if (!writen(tshsock, (char *)&out, sizeof(out)))
   {
      perror("\nOpShell sending commandLine error::writen\n") ;
      getchar() ;
      return ;
   }

   if (!writen(tshsock, commandLine, commandLength))
   {
      perror("\nOpShell sending commandLine error::writen\n") ;
      getchar() ;
      return ;
   }

   if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpShell::readn\n") ;
       getchar() ;
       return ;
    }

   printf("\n\nFrom TSH (OPSHELL):\n") ;
   printf("\nstatus : %d", ntohs(in.status)) ;
   printf("\nerror : %d\n", ntohs(in.error)) ;
   printf("Stdout:[%s]\n", in.stdout);
   getchar() ;

}


void OpPut(char * TpName, char *Tuple, long TpLength)
{
   tsh_put_it out ;
   tsh_put_ot in ;
   int tmp ;
   char *buff,*st ;

   status=system("clear") ;
   printf("TSH_OP_PUT") ;
   printf("\n----------\n") ;
				/* obtain tuple name, priority, length, */
   printf("\nEnter tuple name : ") ; /* and the tuple */
   status=scanf("%s", out.name) ;
   printf("Enter priority : ") ;
   status=scanf("%d", &tmp) ;
   out.priority = (u_short)tmp ;
   printf("Enter length : ") ;
   status=scanf("%d", &out.length) ;
   getchar() ;
   printf("Enter tuple : ") ;
   buff = (char *)malloc(out.length) ;
   st=fgets(buff, out.length, stdin) ;
				/* print data sent to TSH */
   printf("\n\nTo TSH :\n") ;
   printf("\nname : %s", out.name) ;
   printf("\npriority : %d", out.priority) ;
   printf("\nlength : %d", out.length) ;
   printf("\ntuple : %s\n", buff) ;

   out.priority = htons(out.priority) ;
   out.length = htonl(out.length) ;
				/* send data to TSH */
   if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
       perror("\nOpPut::writen\n") ;
       getchar() ;
       free(buff) ;
       return ;
    }
				/* send tuple to TSH */
   if (!writen(tshsock, buff, ntohl(out.length)))
    {
       perror("\nOpPut::writen\n") ;
       getchar() ;
       free(buff) ;
       return ;
    }
				/* read result */
   if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpPut::readn\n") ;
       getchar() ;
       return ;
    }
				/* print result from TSH */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in.status)) ;
   printf("\nerror : %d\n", ntohs(in.error)) ;
   getchar() ;
}


void OpGet(char *pattern, char *buffer)
{
   tsh_get_it out ;
   tsh_get_ot1 in1 ;
   tsh_get_ot2 in2 ;
   struct in_addr addr ;
   int sd, sock ;
   char *buff ;

   status=system("clear") ;
   printf("TSH_OP_GET") ;
   printf("\n----------\n") ;
				/* obtain tuple name/wild card */
   printf("\nEnter tuple name [wild cards ?, * allowed] : ") ;
   status=scanf("%s", out.expr) ;
				/* obtain port for return data if tuple not available */
   // This line has to revise for clusters. out.host = gethostid() ;	
   out.host = inet_addr("127.0.0.1");
   if ((sd = get_socket()) == -1)
    {
       perror("\nOpGet::get_socket\n") ;
       getchar() ; getchar() ;
       return ;
    }
   if (!(out.port = bind_socket(sd, 0)))
    {
       perror("\nOpGet::bind_socket\n") ;
       getchar() ; getchar() ;
       return ;
    }
   addr.s_addr = out.host ;
				/* print data  sent to TSH */
   printf("\n\nTo TSH :\n") ;
   printf("\nexpr : %s", out.expr) ;
   printf("\nhost : %s", inet_ntoa(addr)) ;
   printf("\nport : %d\n", (out.port)) ;
				/* send data to TSH */
   if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
       perror("\nOpGet::writen\n") ;
       getchar() ; getchar() ;
       close(sd) ;
       return ;
    }
				/* find out if tuple available */
   if (!readn(tshsock, (char *)&in1, sizeof(in1)))
    {
       perror("\nOpGet::readn\n") ;
       getchar() ; getchar() ;
       close(sd) ;
       return ;
    }
				/* print whether tuple available in TSH */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in1.status)) ;
   printf("\nerror : %d\n", ntohs(in1.error)) ;
				/* if tuple is available read from the same */
   if (ntohs(in1.status) == SUCCESS) /* socket */
      sock = tshsock ;
   else				/* get connection in the return port */
      sock = get_connection(sd, NULL) ;
				/* read tuple details from TSH */
   if (!readn(sock, (char *)&in2, sizeof(in2)))
    {
       perror("\nOpGet::readn\n") ;
       getchar() ; getchar() ;
       close(sd) ;
       return ;
    }				/* print tuple details from TSH */
   printf("\nname : %s", in2.name) ;
   printf("\npriority : %d", ntohs(in2.priority)) ;
   printf("\nlength : %d", ntohl(in2.length)) ;
   buff = (char *)malloc(ntohl(in2.length)) ;
				/* read, print  tuple from TSH */
   if (!readn(sock, buff, ntohl(in2.length)))
      perror("\nOpGet::readn\n") ;
   else
      printf("\ntuple : %s\n", buff) ;

   close(sd) ;
   close(sock) ;
   free(buff) ;
   getchar() ; getchar() ;
}


void OpExit()
{
   tsh_exit_ot in ;
   
   status=system("clear") ;
   printf("TSH_OP_EXIT") ;
   printf("\n-----------\n") ;
				/* read TSH response */
   if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpExit::readn\n") ;
       getchar() ;  getchar() ;
       return ;
    }
				/* print TSH response */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in.status)) ;
   printf("\nerror : %d\n", ntohs(in.error)) ;
   getchar() ;  getchar() ;
}


int connectTsh(u_short port)
{
   struct hostent *host ;
   short tsh_port ;
   u_long tsh_host ;
   int sock ;

   // use local host 
   tsh_host = inet_addr("127.0.0.1");
   /*
   if ((host = gethostbyname("localhost")) == NULL)
	{
	   perror("connectTsh::gethostbyname\n") ;
	   exit(1) ;
	}
   tsh_host = *((long *)host->h_addr_list[0]) ;
   */
   tsh_port = htons(port);
				/* get socket and connect to TSH */
   if ((sock = get_socket()) == -1)
    {
       perror("connectTsh::get_socket\n") ;
       exit(1) ;
    }
   if (!do_connect(sock, tsh_host, tsh_port))
    {
       perror("connectTsh::do_connect\n") ;
       exit(1) ;
    }      
   return sock ;
}

char* read_input() {
   int buffer_size = INITIAL_SIZE;
   int position = 0;
   char *buffer = malloc(sizeof(char) * buffer_size);  // Allocating initial buffer

   if (!buffer) {
       perror("malloc failed");
       exit(1);
   }

   while (1) {
       // Read a character
       int c = getchar();

       // If EOF or newline is encountered, stop reading
       if (c == EOF || c == '\n') {
           buffer[position] = '\0';
           return buffer;
       } else {
           buffer[position] = c;
       }

       position++;

       // If we reach the end of the buffer, reallocate more space
       if (position >= buffer_size) {
           buffer_size += INITIAL_SIZE;  // Increase buffer size
           buffer = realloc(buffer, buffer_size);  // Reallocate memory
           if (!buffer) {
               perror("realloc failed");
               exit(1);
           }
       }
   }
}