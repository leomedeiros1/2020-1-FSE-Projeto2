#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include <tcp_utils.h>

#define MIN_ROWS 24
#define MIN_COLS 90

// Commands
#define CMD_EXIT 48     // 0
#define CMD_1    49     // 1
#define CMD_2    50     // 2
#define CMD_3    51     // 3

pthread_t keyboard_thread;
pthread_t tcp_server_thread;

int inpt[8], outp[6];

void *watchKeyboard(void *args);
void *handleTCPserver(void *args);

void printMenu(WINDOW *menuWindow);

int startThreads(WINDOW *inputWindow, WINDOW *sensorsWindow);

void safeExit(int signal);

int main(){
    // Add signals to safe exit
    signal(SIGKILL, safeExit);
    signal(SIGSTOP, safeExit);
    signal(SIGINT, safeExit);
    signal(SIGTERM, safeExit);


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

    if(pthread_create(&tcp_server_thread, NULL, handleTCPserver, (void *) sensorsWindow)){
        endwin();
        fprintf(stderr, "ERRO: Falha na criacao de thread(2)\n");
        exit(-2);
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

                int tmp[2];
                mvwprintw(inputWindow, 1, 1, "Insira ... qual lamp");
                mvwprintw(inputWindow, 2, 1, "> ");
                wscanw(inputWindow, "%d", &tmp[0]);
                mvwprintw(inputWindow, 1, 1, "Insira ... 0 = desliga, 1 liga");
                mvwprintw(inputWindow, 2, 1, "                               ");
                mvwprintw(inputWindow, 2, 1, "> ");
                wscanw(inputWindow, "%d", &tmp[1]);

                int to_send = 0;
                to_send += 0x10 * (tmp[0]);
                to_send |= 0x0F & tmp[1]; 
                // 0 0 => 0x10
                // 1 1 => 0x21

                if(tcp_send_int(to_send)){
                    ;// ?
                }
                
                noecho();
            }
            case CMD_2:{
                break;
            }
            case CMD_3:{
                // alarm = 1 xD
                break;
            }
        }  
        wclear(inputWindow);
        box(inputWindow, 0, 0);
        wrefresh(inputWindow);
    }
    return NULL;
}

void *handleTCPserver(void *args){
    WINDOW *sensorsWindow = (WINDOW *) args;
    box(sensorsWindow, 0, 0);
    

    //tcp_send(0xFF)
    //tcp_recv_arr(inpt, sizeof(inpt))
    //tcp_recv_arr(outp, sizeof(outp))
    while(1){
        // print_sensors(sensorsWindow);
        if(tcp_wait_client()){
            continue;
        }
        int comm; 
        if(tcp_recv_int(&comm) == 0){
            //ALAAAARM
            // usa o valor de comm
        }
        
        // tcp_recv_double(&temp);
        // tcp_recv_double(&hum);

        // tcp_close_tmp_client();

        usleep(200000);
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
