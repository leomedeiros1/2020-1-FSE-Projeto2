#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <bcm2835.h>

#include "gpio_utils.h"
#include "tcp_utils.h"

pthread_t tcp_client_thread;
pthread_t tcp_server_thread;

void *handleTCPclient(void *args);
void *handleTCPserver(void *args);

int startThreads();

void safeExit(int signal);

int inpt[8], outp[6];

int main(){   
    // Add signals to safe exit
    signal(SIGKILL, safeExit);
    signal(SIGSTOP, safeExit);
    signal(SIGINT, safeExit);
    signal(SIGTERM, safeExit);

    // Initialize bcm2835
    if (init_bcm2835() < 0){
        fprintf(stderr, "Erro na inicialização do bcm2835\n");
        exit(1);
    }

    // Initialize tcp
    int t_err;
    if((t_err=init_tcp_server())){
        fprintf(stderr, "Erro na inicialização do tcp (%d)\n", t_err);
        exit(2);
    }

    get_gpio_all(inpt, outp);
    startThreads();

    pthread_join(tcp_server_thread, NULL);

    safeExit(0);
    return 0;
}

void safeExit(int signal){
    // Finish threads

    // Turn actuators off

    close_tcp();

    if(signal){
        printf("Execução abortada pelo signal: %d\n", signal);
    }else{
        printf("Execução finalizada pelo usuário\n");
    }

    exit(signal);
}

int startThreads(){
    if(pthread_create(&tcp_client_thread, NULL, handleTCPclient, NULL)){
        fprintf(stderr, "ERRO: Falha na criacao de thread(1)\n");
        exit(-1);
    }

    if(pthread_create(&tcp_server_thread, NULL, handleTCPserver, NULL)){
        fprintf(stderr, "ERRO: Falha na criacao de thread(2)\n");
        exit(-2);
    }

    return 0;
}

void *handleTCPclient(void *args){ // polling -> alarm
    while(1){
        // read gpio inpt]
        int val;
        if((val = get_gpio_inpt(inpt)) != 0){
            // send_int
            tcp_send_int(val);
        }else{
            tcp_send_int(0x0);
        }
        // bme_get_temp e hum
        // tcp_send_double temp
        // tcp_send_double hum
        usleep(200000);
    }

    return NULL;
}

void *handleTCPserver(void *args){
    printf("TCP Server up\n");
    while(1){
        printf("TCP Server: Esperando nova conexao\n");
        if(tcp_wait_client()){
            continue;
        }
        printf("TCP Server: Client connected\n");

        int comm; 
        if(tcp_recv_int(&comm)){
            continue;
        }
        printf("TCP Server: Comando recebido 0x%x\n", comm);
        //trata
        if(comm == 0xFF){
            tcp_send_arr(inpt, sizeof(inpt));
            tcp_send_arr(outp, sizeof(inpt));
        }else{
            set_device(comm, outp);
        }

        tcp_close_tmp_client();
    }
    return NULL;
}