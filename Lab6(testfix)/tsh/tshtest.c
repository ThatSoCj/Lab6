/*.........................................................................*/
/*                  TSHTEST.C ------> TSH test program                     */
/*                                                                         */
/*                  By N. Isaac Rajkumar [April '93]                       */
/*                  February '13, updated by Justin Y. Shi                 */
/*.........................................................................*/

#include "tshtest.h"
void OpRead(void);
#define INITIAL_SIZE 10
int status;

int main(int argc, char **argv)
{
	static void (*op_func[])() = 
	{
		OpPut, OpGet, OpRead, OpShell, OpExit
    } ;
	u_short this_op ;
   
	if (argc < 2)
    {
       printf("Usage : %s port\n", argv[0]) ;
       return 1 ;
    }
	while (TRUE)
    {
       this_op = drawMenu() + TSH_OP_MIN - 1 ;
       if (this_op >= TSH_OP_MIN && this_op <= TSH_OP_MAX)
	   {
		   this_op = htons(this_op) ;
		   tshsock = connectTsh(atoi(argv[1])) ;
		   // Send this_op to TSH
		   if (!writen(tshsock, (char *)&this_op, sizeof(this_op)))
			{
			   perror("main::writen\n") ;
			   exit(1) ;
			}
			printf("sent tsh op \n");
		   // Response processing
		   (*op_func[ntohs(this_op) - TSH_OP_MIN])() ;
		   close(tshsock) ;
	   }			/* validate operation & process */
       else
	  return 0 ;
    }
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

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

#define SHELL_BUF_SIZE 1024

void OpShell()
{
    tsh_shell_it req;
    tsh_shell_ot resp;
    char *commandLine;
    int commandLength;

    // 1) clear and header
    system("clear");
    printf("TSH_OP_SHELL\n");
    printf("----------\n");

    // 2) flush leftover newline from menu
    {
        int _c;
        while ((_c = getchar()) != '\n' && _c != EOF) {}
    }

    // 3) prompt for command
    printf("\nEnter command string> ");
    commandLine = read_input();
    commandLength = strlen(commandLine);
    if (commandLength > 0 && commandLine[commandLength-1]=='\n') {
        commandLine[--commandLength] = '\0';
    }

    // 4) send request header
    req.length = htonl(commandLength);
    if (!writen(tshsock, (char *)&req, sizeof(req))) {
        perror("OpShell::writen header");
        free(commandLine);
        return;
    }
    // send the command bytes
    if (!writen(tshsock, commandLine, commandLength)) {
        perror("OpShell::writen command");
        free(commandLine);
        return;
    }

    // 5) read back the fixed‐size response struct
    if (!readn(tshsock, (char *)&resp, sizeof(resp))) {
        perror("OpShell::readn response");
        free(commandLine);
        return;
    }

    // 6) display results
    printf("\n\nFrom TSH (OPSHELL):\n");
    printf("status : %d\n", ntohs(resp.status));
    printf("error  : %d\n", ntohs(resp.error));
    printf("Stdout : %s\n", resp.stdout);

    // 7) pause before returning
    printf("\nPress ENTER to return to menu...");
    getchar();
    free(commandLine);
}

void OpPut()
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


void OpGet()
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
   printf("match request: expr='%s' from host %u port %d\n",
          out.expr,
          ntohl(out.host),
          ntohs(out.port));
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
   if (ntohs(in1.status) != SUCCESS) {
       /* close resources */
       close(sd);
       /* flush any leftover newline */
       {
           int _c;
           while ((_c = getchar()) != '\n' && _c != EOF) { }
       }
       printf("\nPress ENTER to return to menu...");
       getchar();
       return;
   }
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

   close(sd);
   close(sock);
   free(buff);
   /* flush any leftover newline */
   {
       int _c;
       while ((_c = getchar()) != '\n' && _c != EOF) { }
   }
   printf("\nPress ENTER to return to menu...");
   getchar();
}


void OpRead()
{
    tsh_get_it out;
    tsh_get_ot1 in1;
    tsh_get_ot2 in2;
    struct in_addr addr;
    int sd, sock;
    char *buff;

    status = system("clear");
    printf("TSH_OP_READ\n");
    printf("----------\n");
    /* obtain tuple name [wild cards ?, * allowed] */
    printf("\nEnter tuple name [wild cards ?, * allowed] : ");
    status = scanf("%s", out.expr);
    printf("read request: expr='%s' from host %u port %d\n",
           out.expr,
           ntohl(out.host),
           ntohs(out.port));
    /* set up return port like in OpGet */
    out.host = inet_addr("127.0.0.1");
    if ((sd = get_socket()) == -1) { perror("OpRead::get_socket"); getchar(); return; }
    if (!(out.port = bind_socket(sd, 0))) { perror("OpRead::bind_socket"); getchar(); close(sd); return; }

    addr.s_addr = out.host;
    printf("\n\nTo TSH :\n");
    printf("\nexpr : %s", out.expr);
    printf("\nhost : %s", inet_ntoa(addr));
    printf("\nport : %d\n", out.port);

    /* send READ opcode prior to payload (already done in main) */
    if (!writen(tshsock, (char *)&out, sizeof(out))) { perror("OpRead::writen"); getchar(); close(sd); return; }

    /* receive availability */
    if (!readn(tshsock, (char *)&in1, sizeof(in1))) { perror("OpRead::readn"); getchar(); close(sd); return; }
    printf("\n\nFrom TSH :\n");
    printf("\nstatus : %d", ntohs(in1.status));
    printf("\nerror  : %d\n", ntohs(in1.error));
    if (ntohs(in1.status) != SUCCESS) {
        close(sd);
        /* flush any leftover newline */
        {
            int _c;
            while ((_c = getchar()) != '\n' && _c != EOF) { }
        }
        printf("\nPress ENTER to return to menu...");
        getchar();
        return;
    }

    /* choose correct socket */
    sock = (ntohs(in1.status) == SUCCESS ? tshsock : get_connection(sd, NULL));

    /* read tuple metadata */
    if (!readn(sock, (char *)&in2, sizeof(in2))) { perror("OpRead::readn2"); close(sd); return; }
    printf("\nname     : %s", in2.name);
    printf("\npriority : %d", ntohs(in2.priority));
    printf("\nlength   : %d", ntohl(in2.length));
    buff = malloc(ntohl(in2.length) + 1);
    if (!readn(sock, buff, ntohl(in2.length))) { perror("OpRead::readn3"); free(buff); close(sd); return; }
    buff[ntohl(in2.length)] = '\0';
    printf("\ntuple    : %s\n", buff);

    close(sd);
    close(sock);
    free(buff);
    /* wait for user to press ENTER */
    {
        int _c;
        while ((_c = getchar()) != '\n' && _c != EOF) {}  /* flush leftover */
    }
    printf("\nPress ENTER to return to menu...");
    getchar();
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


u_short drawMenu()
{
   int choice ;
				/* draw menu of user options */
   status=system("clear") ;
   printf("\n\n\n\t\t\t---------") ;
   printf("\n\t\t\tMAIN MENU") ;
   printf("\n\t\t\t---------") ;
 printf("\n\t\t\t 1. Put") ;
printf("\n\t\t\t 2. Get") ;
printf("\n\t\t\t 3. Read") ;
printf("\n\t\t\t 4. Shell") ;
printf("\n\t\t\t 5. Exit (TSH)") ;
printf("\n\t\t\t 6. Quit from this program") ;
       
   printf("\n\n\n\t\t\tEnter Choice : ") ;

   status=scanf("%d", &choice) ;	/* return user choice */
   {
       int _c;
       while ((_c = getchar()) != '\n' && _c != EOF) { }
   }
   return choice ;
}
