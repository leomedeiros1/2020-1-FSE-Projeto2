#ifndef TCP_UTILS
#define TCP_UTILS

typedef struct data_comm{
    int sensor_presence[2];
    int sensor_open[2];
    int control_air[2];
    int control_lamp[4];
}data_comm;

int init_tcp_server();
int init_tcp_client();
int tcp_wait_client();
int tcp_recv_int(int *val);
int tcp_send_int(int val);
int tcp_recv_double(double *val);
int tcp_recv_arr(int arr[], int len);
void close_tcp();

#endif