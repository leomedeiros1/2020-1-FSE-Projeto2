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
int tcp_send_int(int val);
int tcp_recv_int(int *val);
int tcp_send_data_comm(data_comm *data);
int tcp_send_arr(int arr[], int len);
int tcp_send_double(double val);
int tcp_send_float(float val);
int tcp_wait_client();
void tcp_close_tmp_client();
void close_tcp();

#endif