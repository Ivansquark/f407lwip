#ifndef NET_H
#define NET_H

//#include "main.h"
#include "stdint.h"
//#include "netif.h"
//#include "stm32f4xx_hal_uart.h"
//#include "lwip/tcp.h"
//#include "lwip.h"
//#include "lwipopts.h"
//#include "err.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "lwip.h"
#include "lwip/tcp.h"

enum client_states {
  ES_NOT_CONNECTED = 0,
  ES_CONNECTED,
  ES_RECEIVED,
  ES_CLOSING,
};
/*!< structure thats represent LwIP structure of tcp connection>*/
struct client_struct{
    enum client_states state; /* connection status */
    struct tcp_pcb *pcb; /* pointer on the current tcp structure */
    struct pbuf* p_tx; /* pointer on pbuf to be transmitted */
};

void net_command();
void tcp_client_connect();
void UART6_RxCpltCallback(void);
static void tcp_client_disconnect(struct tcp_pcb *tpcb, struct client_struct * cs);
static void tcp_client_send(struct tcp_pcb* tpcb, struct client_struct * cs);
void sendstring(char* buf_str);
#endif //NET_H