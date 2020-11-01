#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#define MIN_ROWS 24
#define MIN_COLS 90

// Commands
#define CMD_EXIT 48     // 0
#define CMD_1    49     // 1

pthread_t keyboard_thread;
pthread_t tcp_server_thread;

void *watchKeyboard(void *args);
void *watchTCP(void *args);

void printMenu(WINDOW *menuWindow);

int startThreads(WINDOW *inputWindow, WINDOW *sensorsWindow);

void safeExit(int signal);

int main(){
    // Add signals to safe exit
    signal(SIGKILL, safeExit);
    signal(SIGSTOP, safeExit);
    signal(SIGINT, safeExit);
    signal(SIGTERM, safeExit);

    // Initialize BME280
    // struct identifier id;
    // if((id.fd = open(I2C_PATH, O_RDWR)) < 0) {
    //     endwin();
    //     fprintf(stderr, "Falha na abertura do canal I2C %s\n", I2C_PATH);
    //     exit(2);
    // }
    // id.dev_addr = BME280_I2C_ADDR_PRIM;
    // if(ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0) {
    //     endwin();
    //     fprintf(stderr, "Falha na comunicaçaõ I2C\n");
    //     exit(3);
    // }
    // dev.intf = BME280_I2C_INTF;
    // dev.read = user_i2c_read;
    // dev.write = user_i2c_write;
    // dev.delay_us = user_delay_us;
    // dev.intf_ptr = &id;
    // int8_t rslt = bme280_init(&dev);
    // if(rslt != BME280_OK) {
    //     endwin();
    //     fprintf(stderr, "Falha na inicialização do dispositivo(codigo %+d).\n", rslt);
    //     exit(4);
    // }

    // Initialize bcm2835
    // if(!bcm2835_init()){
    //     fprintf(stderr, "Erro na inicialização do bcm2835\n");
    //     exit(5);
    // };
    // bcm2835_gpio_fsel(RPI_V2_GPIO_P1_18, BCM2835_GPIO_FSEL_OUTP);
    // bcm2835_gpio_fsel(RPI_V2_GPIO_P1_16, BCM2835_GPIO_FSEL_OUTP);

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
    refresh();

    // Verify window size
    int rows, columns;
    getmaxyx(stdscr, rows, columns);
    while(rows < MIN_ROWS || columns < MIN_COLS){
        mvprintw(0, 0, "Seu terminal é muito pequeno para usar este programa, por favor reajuste");
        mvprintw(1, 0, "Minimos: %d linhas e %d colunas", MIN_ROWS, MIN_COLS);
        mvprintw(2, 0, "Atual: %d linhas e %d colunas", rows, columns);
        refresh();
        usleep(500000);
        getmaxyx(stdscr, rows, columns);
        clear();
        refresh();
    }

    WINDOW *sensorsWindow = newwin(LINES - 12, COLS, 0, 0);
    WINDOW *menuWindow = newwin(8, COLS, LINES - 12, 0);
    WINDOW *inputWindow = newwin(4, COLS, LINES - 4, 0);

    printMenu(menuWindow);

    startThreads(inputWindow, sensorsWindow);

    pthread_join(keyboard_thread, NULL);

    delwin(sensorsWindow);
    delwin(menuWindow);
    delwin(inputWindow);

    safeExit(0);
    return 0;
}

void safeExit(int signal){
    // Finish threads

    // SEND MSG to Distributed: Turn actuators off

    echo();
    endwin();

    if(signal){
        printf("Execução abortada pelo signal: %d\n", signal);
    }else{
        printf("Execução finalizada pelo usuário\n");
    }

    exit(signal);
}

int startThreads(WINDOW *inputWindow, WINDOW *sensorsWindow){
    if(pthread_create(&keyboard_thread, NULL, watchKeyboard, (void *) inputWindow)){
        endwin();
        fprintf(stderr, "ERRO: Falha na criacao de thread(1)\n");
        exit(-1);
    }

    if(pthread_create(&tcp_server_thread, NULL, watchTCP, (void *) inputWindow)){
        endwin();
        fprintf(stderr, "ERRO: Falha na criacao de thread(1)\n");
        exit(-1);
    }

    return 0;
}

void *watchKeyboard(void *args){
    WINDOW *inputWindow = (WINDOW *) args;
    int op_code;
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);
    while((op_code = getch()) != CMD_EXIT){
        switch(op_code){
            case CMD_1:{
                echo();

                int tmp;
                mvwprintw(inputWindow, 1, 1, "Insira ...");
                mvwprintw(inputWindow, 2, 1, "> ");
                wscanw(inputWindow, "%d", &tmp);
                
                noecho();
                break;
            }
        }  
        wclear(inputWindow);
        box(inputWindow, 0, 0);
        wrefresh(inputWindow);
    }
    return NULL;
}

void *watchTCP(void *args){
    while(1){
        sleep(1);
    }
    return NULL;
}

void printMenu(WINDOW *menuWindow){
    box(menuWindow, 0, 0);
    wrefresh(menuWindow);
    mvwprintw(menuWindow, 1, 1, "Lista de comandos disponíveis:");
    mvwprintw(menuWindow, 2, 1, "1 - Definir temperatura de referência");
    mvwprintw(menuWindow, 6, 1, "0 ou CTRL+C - Sair");
    wrefresh(menuWindow);
}
