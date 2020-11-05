#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <linux_userspace.c>
#include <bcm2835.h>

#include "gpio_utils.h"
#include "tcp_utils.h"

static const char I2C_PATH[] = "/dev/i2c-1";

struct bme280_dev dev;

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

    // Initialize BME280
    struct identifier id;
    if((id.fd = open(I2C_PATH, O_RDWR)) < 0) {
        fprintf(stderr, "Falha na abertura do canal I2C %s\n", I2C_PATH);
        exit(3);
    }
    id.dev_addr = BME280_I2C_ADDR_PRIM;
    if(ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0) {
        fprintf(stderr, "Falha na comunicaçaõ I2C\n");
        exit(4);
    }
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;
    dev.intf_ptr = &id;
    int8_t rslt = bme280_init(&dev);
    if(rslt != BME280_OK) {
        fprintf(stderr, "Falha na inicialização do dispositivo(codigo %+d).\n", rslt);
        exit(5);
    }

    get_gpio_all(inpt, outp);
    startThreads();

    pthread_join(tcp_server_thread, NULL);

    safeExit(0);
    return 0;
}

void safeExit(int signal){
    // Finish threads
    pthread_cancel(tcp_client_thread);
    pthread_cancel(tcp_server_thread);
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
    printf("TCP Client up");
    while(1){
        
        // read gpio inpt]
        int val;
        if((val = get_gpio_change(inpt, outp)) != 0){
            tcp_send_int(val);
            printf("TCP Client: Encontrei uma mudança de estado\n");
        }else{
            tcp_send_int(0xFF);
            printf("TCP Client: Nada parar ver por aqui\n");
        }
        // bme_get_temp e hum
        // tcp_send_double temp
        // tcp_send_double hum
        float temp, hum;
        int rslt = get_sensor_data_forced_mode(&dev, &temp, &hum);
        if (rslt == BME280_OK){
            tcp_send_float(temp);
            tcp_send_float(hum);
        }
        // usleep(2000000);
        sleep(2);
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
            tcp_send_arr(outp, sizeof(outp));
        }else{
            set_device(comm, outp);
        }

        tcp_close_tmp_client();
    }
    return NULL;
}