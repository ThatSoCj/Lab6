#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "tshlib.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> \"<command>\"\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    char *command = argv[2];
    int cmdlen = strlen(command);

    u_short op = htons(TSH_OP_SHELL);
    tsh_shell_it req;
    tsh_shell_ot resp;
    int sock = connectTsh(port);

    // Send the shell opcode
    if (!writen(sock, (char *)&op, sizeof(op))) {
        perror("launch::writen opcode");
        return 1;
    }
    // Send the command length header
    req.length = htonl(cmdlen);
    if (!writen(sock, (char *)&req, sizeof(req))) {
        perror("launch::writen header");
        return 1;
    }
    // Send the command string
    if (!writen(sock, command, cmdlen)) {
        perror("launch::writen command");
        return 1;
    }
    // Read the response
    if (!readn(sock, (char *)&resp, sizeof(resp))) {
        perror("launch::readn response");
        return 1;
    }
    // Print the results
    printf("From TSH (OPSHELL):\n");
    printf("status : %d\n", ntohs(resp.status));
    printf("error  : %d\n", ntohs(resp.error));
    printf("Stdout : %s\n", resp.stdout);
    return 0;
}
