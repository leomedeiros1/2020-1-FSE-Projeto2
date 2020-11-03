#ifndef TCP_UTILS
#define TCP_UTILS

typedef struct data_comm{
    int sensor_presence[2];
    int sensor_open[2];
    int control_air[2];
    int control_lamp[4];
}data_comm;

int init_tcp();
void *handleTCPserver(void *args);
int tcp_send_int(int val);
void close_tcp();

#endif