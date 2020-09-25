#include "net.h"
#include "netif.h"

extern UART_HandleTypeDef huart6;
uint8_t ip[4] = {10,42,0,1};
 uint16_t port = 55555;
uint8_t IsConnected = 0;
struct tcp_pcb *tcp_client_pcb;
//extern volatile uint32_t message_count=0;
extern uint8_t data[100];


void net_command() {
    tcp_client_connect();
    char* str1 = "Try to connect\n";
    sendStr(str1);
}

static err_t tcp_client_connected(void* arg, struct tcp_pcb* tpcb, err_t err) {
    char* str1 = "callback connect\n";
    sendStr(str1);
    return err;
}

void tcp_client_connect() {
    ip_addr_t DestIPaddr;
    tcp_client_pcb = tcp_new(); //@return a new tcp_pcb that initially is in state CLOSED
    if(tcp_client_pcb != NULL) {
        IP4_ADDR(&DestIPaddr,ip[0],ip[1],ip[2],ip[3]);
        tcp_connect(tcp_client_pcb,&DestIPaddr,port,tcp_client_connected);
        char* str1 = "Try to connect\n";
        sendStr(str1);
    }
}
void tcp_client_disconnect() {
    tcp_recv(tcp_client_pcb, NULL);
    tcp_sent(tcp_client_pcb, NULL);
    tcp_poll(tcp_client_pcb, NULL,0);
    tcp_close(tcp_client_pcb);
    char* str1 = "\ndisconnect\n";
    sendStr(str1);
}

void UART6_RxCpltCallback(void)
{
    char* str1 = "RxCallback\n";
    sendStr(str1);
    tcp_client_connect();
}
