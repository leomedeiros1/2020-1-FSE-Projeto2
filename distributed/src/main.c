#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <bcm2835.h>

#include <gpio_utils.h>
#include <tcp_utils.h>

pthread_t tcp_client_thread;
pthread_t tcp_server_thread;

void *TCPclient(void *args){return NULL;}
void *HandleTCPserver(void *args){return NULL;}

int startThreads();

void safeExit(int signal);

int main(){   
    // Add signals to safe exit
    signal(SIGKILL, safeExit);
    signal(SIGSTOP, safeExit);
    signal(SIGINT, safeExit);
    signal(SIGTERM, safeExit);

    if (init_bcm2835() < 0){
        fprintf(stderr, "Erro na inicialização do bcm2835\n");
        exit(1);
    }

    startThreads();

    pthread_join(tcp_server_thread, NULL);

    safeExit(0);
    return 0;
}

void safeExit(int signal){
    // Finish threads

    // Turn actuators off

    if(signal){
        printf("Execução abortada pelo signal: %d\n", signal);
    }else{
        printf("Execução finalizada pelo usuário\n");
    }

    exit(signal);
}

int startThreads(){
    if(pthread_create(&tcp_client_thread, NULL, TCPclient, NULL)){
        fprintf(stderr, "ERRO: Falha na criacao de thread(1)\n");
        exit(-1);
    }

    if(pthread_create(&tcp_server_thread, NULL, HandleTCPserver, NULL)){
        fprintf(stderr, "ERRO: Falha na criacao de thread(1)\n");
        exit(-1);
    }

    return 0;
}