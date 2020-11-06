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

pthread_t keyboard_thread;
pthread_t tcp_server_thread;

int inpt[8], outp[6];
float temp=999, hum=999;
int test=0;

void *watchKeyboard(void *args);
void *handleTCPserver(void *args);

void printMenu(WINDOW *menuWindow);
void print_sensors(WINDOW *sensorsWindow);

int startThreads(WINDOW *inputWindow, WINDOW *sensorsWindow);

void safeExit(int signal);

int main(){
    // Add signals to safe exit
    signal(SIGKILL, safeExit);
    signal(SIGSTOP, safeExit);
    signal(SIGINT, safeExit);
    signal(SIGTERM, safeExit);

    // Initialize tcp
    int t_err;
    if((t_err=init_tcp_server())){
        fprintf(stderr, "Erro na inicialização do tcp (%d)\n", t_err);
        exit(2);
    }

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
    pthread_cancel(keyboard_thread);
    pthread_cancel(tcp_server_thread);
    // SEND MSG to Distributed: Turn actuators off

    close_tcp();

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
    while((op_code = getch()) != KEY_F(1)){
        switch(op_code){
            case KEY_F(2):{
                echo();

                int tmp;
                mvwprintw(inputWindow, 1, 1, "Insira ... qual lamp     ");
                mvwprintw(inputWindow, 2, 1, "> ");
                wscanw(inputWindow, "%d", &tmp);

                int to_send = 0;
                to_send+=tmp;
                // to_send += 0x10 * (tmp[0]);
                // to_send |= 0x0F & tmp[1];        

                if((op_code=tcp_send_int(to_send))){
                    mvwprintw(inputWindow, 1, 1, "Falha no envio do comando (%d)\n", op_code);
                    mvwprintw(inputWindow, 2, 1, "> ");
                    wscanw(inputWindow, "%d", &tmp);
                }
                
                noecho();
                break;
            }
            case KEY_F(3):{
                echo();
                int tmp;
                mvwprintw(inputWindow, 1, 1, "Insira ... qual ar      ");
                mvwprintw(inputWindow, 2, 1, "> ");
                wscanw(inputWindow, "%d", &tmp);

                int to_send = 4;
                to_send+=tmp;

                if((op_code=tcp_send_int(to_send))){
                    //faio
                }
                noecho();
                break;
            }
            case KEY_F(4):{
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
    wclear(sensorsWindow);
    box(sensorsWindow, 0, 0);
    wrefresh(sensorsWindow);
    

    tcp_send_int(0xFF);
    tcp_wait_client();
    tcp_recv_arr(inpt, sizeof(inpt));
    tcp_wait_client();
    tcp_recv_arr(outp, sizeof(outp));
    while(1){
        ++test;
        print_sensors(sensorsWindow);
        int f;
        if((f=tcp_wait_client())){
            continue;
            mvwprintw(sensorsWindow, 10, 1, "Erro no tcp: (2): %s",  f));
            wrefresh(sensorsWindow);
        }
        data_comm comm;
        if(tcp_recv_data_comm(&comm) == 0){
            int command = comm.command;
            if(command & 0xF0){
                //ALAAAARM
                inpt[command & 0x0F] = 1 - inpt[command & 0x0F];
            }else if(command != 0xFF){
                outp[command & 0x0F] = 1 - outp[command & 0x0F];
            }
            // if(comm.temp > 0.0f && comm.temp <50.0f)
                temp = comm.temp;
            // if(comm.hum > 0.0f && comm.hum <100.0f)
                hum = comm.hum;
        }

        tcp_close_tmp_client();

        // usleep(200000);
    }
    return NULL;
}

void printMenu(WINDOW *menuWindow){
    box(menuWindow, 0, 0);
    wrefresh(menuWindow);
    mvwprintw(menuWindow, 1, 1, "Lista de comandos disponíveis:");
    mvwprintw(menuWindow, 2, 1, "F2 - Alterar uma lampada");
    mvwprintw(menuWindow, 3, 1, "F3 - Alterar um ar-condicionado");
    mvwprintw(menuWindow, 4, 1, "F4 - Alterar alarme");

    mvwprintw(menuWindow, 6, 1, "F1 ou CTRL+C - Sair");
    wrefresh(menuWindow);
}

void print_sensors(WINDOW *sensorsWindow){
    box(sensorsWindow, 0, 0);
    wrefresh(sensorsWindow);
    mvwprintw(sensorsWindow, 1, 1, "Lampada Cozinha (1):..........%s ", (outp[0] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 2, 1, "Lampada Sala (2):.............%s ", (outp[1] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 3, 1, "Lampada Quarto1 (3):..........%s ", (outp[2] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 4, 1, "Lampada Quarto2 (4):..........%s ", (outp[3] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 5, 1, "Ar-condicionado Quarto1 (1):..%s ", (outp[4] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 6, 1, "Ar-condicionado Quarto2 (2):..%s ", (outp[5] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 7, 1, "DB: %d", test);

    mvwprintw(sensorsWindow, 8, 1, "Temperatura :.................%.2f ", temp);
    mvwprintw(sensorsWindow, 9, 1, "Humidade :....................%.2f ", hum);
    // mvwprintw(sensorsWindow, 10, 1, "Alarme: (2): %s", (alarm_bool ? "ON" : "OFF"));

    wrefresh(sensorsWindow);
}
