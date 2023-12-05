#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <signal.h>

#include "server/server.hpp"

void    exitFunction(int sig){
    if (sig == SIGINT)
        std::cerr << "Thanks for using IRC SERVER ! See you next time !" << std::endl;
    return ;
}
int main(int argc, char **argv) {
    signal(SIGINT, exitFunction);
    if (argc != 3) {
        std::cerr << "please enter the port value and the password" << std::endl;
        return 1;
    }
    char *endptr;
    long int port = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1] || *endptr != '\0') {
        std::cerr << "please enter a valide port" << std::endl;
        return 1;
    }
    Server test(port, argv[2]);
    test.run();
	return 0;
}