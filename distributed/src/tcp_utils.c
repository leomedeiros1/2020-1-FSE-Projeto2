#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "tcp_utils.h"

#include "gpio_utils.h"

#define DESTINATION_PORT 10026
#define DESTINATION_IP "192.168.0.53"

#define LOCAL_PORT 10126
#define LOCAL_IP "192.168.0.52"

// Client
int client_socket;
struct sockaddr_in client_addr;
struct sockaddr_in target_addr;

// Server
int server_socket;
int tmp_client_socket;
struct sockaddr_in server_addr;
struct sockaddr_in tmp_client_addr;

unsigned int tmp_client_len;

int init_tcp_server(){
    // Init server
    unsigned short server_port = LOCAL_PORT;
    // char *server_ip = LOCAL_IP;

    if((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(server_port);

    if(bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
		return -2;
    }

    if(listen(server_socket, 10) < 0){
        return -3;
    }

    return 0;
}

int init_tcp_client(){
    // Init client
    if((client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        return -4;
    }

    memset(&target_addr, 0, sizeof(target_addr)); // Zerando a estrutura de dados
	target_addr.sin_family = AF_INET;
	target_addr.sin_addr.s_addr = inet_addr(DESTINATION_IP);
	target_addr.sin_port = htons(DESTINATION_PORT);

    return 0;
}

int tcp_send_int(int val){
    init_tcp_client();
    if(connect(client_socket, (struct sockaddr *) &target_addr, 
							sizeof(target_addr)) < 0)
    {
        return -5;
    }
    if(send(client_socket, &val, sizeof(val), 0) < sizeof(val)){
        return -1;
    }
    // recv?
    close(client_socket);
    return 0;
}

int tcp_wait_client(){
    tmp_client_len = sizeof(tmp_client_addr);
    if((tmp_client_socket = accept(server_socket, 
        (struct sockaddr *) &tmp_client_addr, 
        &tmp_client_len)) < 0)
    {
        // continue;
        printf("Erro no acc\n");
        return -1;
    }

    return 0;
}

int tcp_recv_int(int *val){
    int recv_size = sizeof(val);
    if((recv_size = recv(tmp_client_socket, val, sizeof(data_comm), 0)) < 0){
        // continue;
        return -1;
    }
    return 0;
}

int tcp_send_arr(int arr[], int len){
    init_tcp_client();
    if(connect(client_socket, (struct sockaddr *) &target_addr, 
							sizeof(target_addr)) < 0)
    {
        return -5;
    }
    if(send(client_socket, &arr, len, 0) < len){
        return -1;
    }
    // recv?
    close(client_socket);
    return 0;
}

void tcp_close_tmp_client(){
    close(tmp_client_socket);
}

void close_tcp(){
    close(client_socket);
    close(server_socket);
    close(tmp_client_socket);
}