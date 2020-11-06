#ifndef TCP_UTILS
#define TCP_UTILS

typedef struct data_comm{
    int command;
    float temp, hum;
}data_comm;

int init_tcp_server();
int init_tcp_client();
int tcp_wait_client();
int tcp_recv_int(int *val);
int tcp_send_int(int val);
int tcp_recv_double(double *val);
int tcp_recv_float(float *val);
int tcp_recv_arr(int arr[], int len);
int tcp_recv_data_comm(data_comm *data);
void tcp_close_tmp_client();
void close_tcp();

#endif