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

#define HEX_ALARM_CODE 0xFF00

static const char CSV_DATA_PATH[] = "./data.csv";

pthread_t keyboard_thread;
pthread_t tcp_server_thread;
pthread_t alarm_thread;

int inpt[8], outp[6];
float temp=0.0f, hum=0.0f, ref_temp=0.0f;
// int test=0; // 
int alarm_mode=0; // 0 off, 1 on
int temp_mode=0; // 0 manual, 1 automatico

void *watchKeyboard(void *args);
void *handleTCPserver(void *args);
void *handleAlarm(void *args);

void printMenu(WINDOW *menuWindow);
void print_sensors(WINDOW *sensorsWindow);

int startThreads(WINDOW *inputWindow, WINDOW *sensorsWindow);

int send_command(int command);
void csv_write(int command);

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
    pthread_cancel(tcp_server_thread);
    pthread_cancel(alarm_thread);

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
    pthread_cancel(alarm_thread);

    // finish tcp
    close_tcp();

    // finish curses
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

    if(pthread_create(&alarm_thread, NULL, handleAlarm, NULL)){
        endwin();
        fprintf(stderr, "ERRO: Falha na criacao de thread(3)\n");
        exit(-3);
    }

    return 0;
}

void *watchKeyboard(void *args){
    usleep(200000);
    WINDOW *inputWindow = (WINDOW *) args;
    // wclear(inputWindow);
    int op_code;
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);
    int b_clear=0;
    while((op_code = getch()) != KEY_F(1)){
        switch(op_code){
            case KEY_F(2):{
                echo();

                int tmp;
                mvwprintw(inputWindow, 1, 1, "Insira o ID da lâmpada que seja alterar                                       ");
                mvwprintw(inputWindow, 2, 1, "> ");
                wscanw(inputWindow, "%d", &tmp);

                if(tmp>4){
                    mvwprintw(inputWindow, 1, 1, "ID inválido                                                                   ");
                    mvwprintw(inputWindow, 2, 1, "                                                                              ");
                    b_clear=0;
                }

                int to_send = 0;
                to_send+=tmp-1;
                // to_send += 0x10 * (tmp[0]);
                // to_send |= 0x0F & tmp[1];        

                if(send_command(to_send)){
                    ;
                }else{
                    b_clear=1;
                }
                
                noecho();
                break;
            }
            case KEY_F(3):{
                echo();
                int tmp;
                mvwprintw(inputWindow, 1, 1, "Insira o ID do aparelho que seja alterar                                       ");
                mvwprintw(inputWindow, 2, 1, "> ");
                wscanw(inputWindow, "%d", &tmp);

                if(tmp>4){
                    mvwprintw(inputWindow, 1, 1, "ID inválido                                                                   ");
                    mvwprintw(inputWindow, 2, 1, "                                                                              ");
                    b_clear=0;
                }

                int to_send = 3;
                to_send+=tmp;

                if(send_command(to_send)){
                    //faio
                }else{
                    b_clear=1;
                }
                noecho();
                break;
            }
            case KEY_F(5):{
                // alarm = 1 xD
                int check_s=1;
                if(alarm_mode == 0){
                    for(int i=2; i<8; ++i){ // Sensores de abertura
                        if(inpt[i] != 0){
                            check_s=0;
                            mvwprintw(inputWindow, 1, 1, "Necessário ter todas as portas e janelas fechadas para ativar o alarme  ", (alarm_mode ? "ON" : "OFF"));
                            break;
                        }
                    }
                }
                if(check_s){
                    alarm_mode = 1-alarm_mode;
                    mvwprintw(inputWindow, 1, 1, "Alarme alterado para: %s                                                       ", (alarm_mode ? "ON" : "OFF"));
                    csv_write(HEX_ALARM_CODE);
                }   
                b_clear=0;
                break;
            }
            case KEY_F(4):{
                // definir temp
                if(temp_mode){
                    temp_mode=0;
                    mvwprintw(inputWindow, 1, 1, "Modo de controle da temperatura alterado para: %s                              ", (temp_mode ? "ON" : "OFF"));
                    b_clear=0;
                }else{
                    mvwprintw(inputWindow, 1, 1, "Insira a nova temperatura desejada                                             ");
                    mvwprintw(inputWindow, 2, 1, "> ");
                    wscanw(inputWindow, "%f", &ref_temp);
                    temp_mode=1;
                }
                break;
            }
        }  
        if(b_clear) wclear(inputWindow);
        box(inputWindow, 0, 0);
        wrefresh(inputWindow);
    }
    return NULL;
}

void *handleTCPserver(void *args){
    WINDOW *sensorsWindow = (WINDOW *) args;
    // wclear(sensorsWindow);
    box(sensorsWindow, 0, 0);
    wrefresh(sensorsWindow);
    

    tcp_send_int(0xFF);
    tcp_wait_client();
    tcp_recv_arr(inpt, sizeof(inpt));
    tcp_wait_client();
    tcp_recv_arr(outp, sizeof(outp));
    while(1){
        // ++test;
        print_sensors(sensorsWindow);
        int f;
        if((f=tcp_wait_client())){
            mvwprintw(sensorsWindow, 10, 1, "Erro no tcp: (2): %s",  f);
            wrefresh(sensorsWindow);
            continue;
        }
        data_comm comm;
        if(tcp_recv_data_comm(&comm) == 0){
            int command = comm.command;
            if(command == 0xFF){
                // Nada a mudar;
                ;
            }
            else if(command & 0x10){
                // if(alarm) make_some_noise();
                if(alarm_mode) mvwprintw(sensorsWindow, 7, 10, "ABC ");
                inpt[command & 0x0F] = 1 - inpt[command & 0x0F];
            }else{
                outp[command & 0x0F] = 1 - outp[command & 0x0F];
            }

            if(comm.temp > 0.0f && comm.temp <50.0f)
                temp = comm.temp;
            if(comm.hum > 0.0f && comm.hum <100.0f)
                hum = comm.hum;
        }

        if(temp_mode){
            if(temp < ref_temp){
                if(outp[4])  // ligado?
                    send_command(0x04); // desliga
                if(outp[5])
                    send_command(0x05);
            }else{
                if(!outp[4]) // desligado?
                    send_command(0x04); // liga
                if(!outp[5])
                    send_command(0x05);
            }
        }

        tcp_close_tmp_client();

        // usleep(200000);
    }
    return NULL;
}

void *handleAlarm(void *args){
    while(1){
        int play_alarm=0;
        if(alarm_mode == 1){
            for(int i=0; i<8; ++i){
                if(inpt[i]) play_alarm=1;
            }
        }
        if(play_alarm){
            system("omxplayer alarme.mp3 > /dev/null 2>&1");
        }
        usleep(500000);
    }
}

int send_command(int command){
    int ret_val=0;
    ret_val = tcp_send_int(command);
    csv_write(command);
    return ret_val;
}

void csv_write(int command){
    // Open csv
    FILE *arq;
    arq = fopen(CSV_DATA_PATH, "r+");
    if(arq){
        fseek(arq, 0, SEEK_END);
    }
    else{
        arq = fopen(CSV_DATA_PATH, "a");
        // Header
        fprintf(arq, "Comando, Data e Hora\n");
    }

    if(arq){
        time_t rawtime;
        struct tm * timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        if(command == HEX_ALARM_CODE){
            fprintf(arq, "Ativa/Desativar Alarme, %s", asctime (timeinfo));
        }else{
            fprintf(arq, "Ativa/Desativar dispositivo 0x%02X, %s", command, asctime (timeinfo));

        }
    }
    else{
        endwin();
        printf("Não foi possivel abrir o arquivo para csv.\n");
        exit(-1);
    }

    // Close CSV
    fclose(arq); 
    return;
}

void printMenu(WINDOW *menuWindow){
    // wclear(menuWindow);
    box(menuWindow, 0, 0);
    wrefresh(menuWindow);
    mvwprintw(menuWindow, 1, 1, "Lista de comandos disponíveis:");
    mvwprintw(menuWindow, 2, 1, "F2 - Alterar uma lampada");
    mvwprintw(menuWindow, 3, 1, "F3 - Alterar um ar-condicionado");
    mvwprintw(menuWindow, 4, 1, "F4 - Alterar modo de funcionamento do ar-condicionado");
    mvwprintw(menuWindow, 5, 1, "F5 - Alterar alarme");

    mvwprintw(menuWindow, 6, 1, "F1 ou CTRL+C - Sair");
    wrefresh(menuWindow);
}

void print_sensors(WINDOW *sensorsWindow){
    box(sensorsWindow, 0, 0);
    wrefresh(sensorsWindow);
    mvwprintw(sensorsWindow, 1, 1, "Lampada Cozinha (1):...........%s ", (outp[0] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 2, 1, "Lampada Sala (2):..............%s ", (outp[1] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 3, 1, "Lampada Quarto1 (3):...........%s ", (outp[2] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 4, 1, "Lampada Quarto2 (4):...........%s ", (outp[3] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 5, 1, "Ar-condicionado Quarto1 (1):...%s ", (outp[4] ? "ON" : "OFF"));
    mvwprintw(sensorsWindow, 6, 1, "Ar-condicionado Quarto2 (2):...%s ", (outp[5] ? "ON" : "OFF"));
    
    char tmp_str[255] = "";
    sprintf(tmp_str, "Automatico - %f °C", ref_temp);
    mvwprintw(sensorsWindow, 7, 1, "Controle do ar-condicionado:...%s    ", temp_mode ? tmp_str : "Manual                       ");


    if(temp != 0.0f){
        mvwprintw(sensorsWindow, 8, 1, "Temperatura :..................%.2f ", temp);
    }else{
        mvwprintw(sensorsWindow, 8, 1, "Temperatura :...Aguardando servidor ");
    }

    if(hum != 0.0f){
        mvwprintw(sensorsWindow, 9, 1, "Humidade :.....................%.2f ", hum);
    }else{
        mvwprintw(sensorsWindow, 9, 1, "Humidade :......Aguardando servidor ");
    }

    
    mvwprintw(sensorsWindow, 10, 1, "Alarme:........................%s ", (alarm_mode ? "ON" : "OFF"));

    // mvwprintw(sensorsWindow, 10, 50, "DB: %d", test);

    wrefresh(sensorsWindow);
}
