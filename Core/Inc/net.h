#ifndef NET_H
#define NET_H

#include "main.h"
#include "stdint.h"
#include "stm32f4xx_hal_uart.h"
#include "lwip/tcp.h"
//#include "lwip.h"

void net_command();
void tcp_client_connect();
void UART6_RxCpltCallback(void);

#endif //NET_H