#include "net.h"
//#include "err.h"


static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb);

extern UART_HandleTypeDef huart6;
uint8_t ip[4] = {10,42,0,1};
 uint16_t port = 55555;
uint8_t IsConnected = 0;
struct tcp_pcb *tcp_client_pcb;
struct client_struct *cs; //status state structure
//extern volatile uint32_t message_count=0;
uint8_t data[100]; //buffer for data


void net_command() {
    //tcp_client_connect();
    //char* str1 = "Try to connect\n";
    //sendStr(str1);
}

static err_t tcp_client_connected(void* arg, struct tcp_pcb* tpcb, err_t err) {
    char* str1 = "callback connect\n";
    sendStr(str1);
    /*!<allocate memory for client struct>*/
    cs = (struct client_struct *)mem_malloc(sizeof(struct client_struct)); 
    if (cs != NULL) {
        cs->state = ES_CONNECTED;
        cs->pcb = tpcb;
        /*!< allocate memory for struct pbuf (headers) PBUF_POOL- for Rx >*/
        cs->p_tx = pbuf_alloc(PBUF_TRANSPORT, strlen((char*)data) , PBUF_POOL); //tx must be PBUF_RAM
        if (cs->p_tx) {
          /* copy data to pbuf */
          char* str = "Hello PC!!!";
          for(uint8_t i=0;i<strlen(str);i++){
              data[i] = str[i];
          }        
          /*!< initial fills pbuf with data >*/  
          pbuf_take(cs->p_tx, (char*)data, strlen((char*)data));         
          /* pass newly allocated cs structure as argument to tpcb connects pcb to client struct*/
          tcp_arg(tpcb, cs);
          /*!< Sets callback functions on tcp events >*/
          /* initialize LwIP tcp_recv callback function */
          tcp_recv(tpcb, tcp_client_recv);
          /* initialize LwIP tcp_sent callback function */
          tcp_sent(tpcb, tcp_client_sent);
          /* initialize LwIP tcp_poll callback function */
          tcp_poll(tpcb, tcp_client_poll, 1);
          /* send data */
          tcp_client_send(tpcb,cs);
          return ERR_OK;
        }
    }
    else {
      tcp_client_disconnect(tpcb, cs);
      return ERR_MEM;
    }
    return err;
}

void tcp_client_connect() {
    ip_addr_t DestIPaddr;
    /*!< create new TCP PCB (memory allocation) >*/
    tcp_client_pcb = tcp_new(); //@return a new tcp_pcb that initially is in state CLOSED
    if(tcp_client_pcb != NULL) {
        IP4_ADDR(&DestIPaddr,ip[0],ip[1],ip[2],ip[3]);
        /*!< Connects to remote TCP host >*/
        tcp_connect(tcp_client_pcb,&DestIPaddr,port,tcp_client_connected);
        char* str1 = "Try to connect\n";
        sendStr(str1);
    }
}
void tcp_client_discon() {
    tcp_client_disconnect(tcp_client_pcb,cs);
}
static void tcp_client_disconnect(struct tcp_pcb *tpcb, struct client_struct * cs) {
    /*!< sets callback functions to NULL >*/
    tcp_recv(tcp_client_pcb, NULL);
    tcp_sent(tcp_client_pcb, NULL);
    tcp_poll(tcp_client_pcb, NULL,0);
    if (cs != NULL) {
    mem_free(cs);
    }
    /*!< close connection >*/
    tcp_close(tcp_client_pcb);
    char* str1 = "\ndisconnectedd\n";
    sendStr(str1);
}

/*!<callback function from LwIP that calls when client receives data>*/
/* pbuf* p - Main packet buffer struct */
static err_t tcp_client_recv(void *arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {
    struct client_struct* es;
    err_t errt; 
    es = (struct client_struct *)arg;
    if (p == NULL) {
        es->state = ES_CLOSING;
        if(es->p_tx == NULL) {
            tcp_client_disconnect(tpcb, es);
        }
        errt = ERR_OK;
    } else if(err != ERR_OK) { //if gets not empty packet with error state    
        if (p != NULL) {
            pbuf_free(p);
        }
        errt = err;
    } else if(es->state == ES_CONNECTED) {    
        tcp_recved(tpcb, p->tot_len);
        es->p_tx = p;
        uint8_t str1[100];
        for(uint8_t i=0;i<es->p_tx->len;i++){
            str1[i] = *((uint8_t*)es->p_tx->payload + i);
        }
        str1[es->p_tx->len] = '\n';
        str1[es->p_tx->len+1] = '\0';
        sendStr(str1);
    } else if (es->state == ES_RECEIVED) { //if successfully receive    
        errt = ERR_OK;
    } else { //ACK
        /* Acknowledge data reception */
        tcp_recved(tpcb, p->tot_len);
        /* free pbuf and do nothing */
        pbuf_free(p);
        errt = ERR_OK;
    }
    return errt;   
}
//----------------------------------------------------------
/*!<callback function from LwIP that calls when client sends data>*/
static void tcp_client_send(struct tcp_pcb* tpcb, struct client_struct * es) {
    struct pbuf *ptr;
    err_t wr_err = ERR_OK;
    while ((wr_err == ERR_OK) && (es->p_tx != NULL) && (es->p_tx->len <= tcp_sndbuf(tpcb))) {
        ptr = es->p_tx;
        /*!< push data to queue from which its transmits to remote host (server) >*/
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1); //1: without PUSH, 2: with PUSH
        if (wr_err == ERR_OK) {
            es->p_tx = ptr->next;
            if(es->p_tx != NULL) {
                pbuf_ref(es->p_tx);
            }
            pbuf_free(ptr);
        } else if(wr_err == ERR_MEM) {
            es->p_tx = ptr; //buf is busy
        } else {
            /* other problem ?? */
        }
    }
}
//----------------------------------------------------------
/*!<This function calls after buf transmit to server>*/
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len){
    struct client_struct *es;
    LWIP_UNUSED_ARG(len);
    es = (struct client_struct *)arg;
    if(es->p_tx != NULL) { // if buf not empty sends it to server
        tcp_client_send(tpcb, es);
    }
    return ERR_OK;
}
//----------------------------------------------------------
/*!<function the monitoring net>*/
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb) {
    err_t ret_err=0;
    struct client_struct *es;
    es = (struct client_struct*)arg;
    if (es != NULL) {
        //if (es->p_tx != NULL) {
        //
        //} else {
        //    if(es->state == ES_CLOSING) {
        //        tcp_client_disconnect(tpcb, es);
        //    } else {
        //        tcp_abort(tpcb);
        //        ret_err = ERR_ABRT;
        //    }
        //    
        //}
        ret_err = ERR_OK;
    }
  return ret_err;
}

void sendstring(char* buf_str) {
    //tcp_sent(tcp_client_pcb, tcp_client_sent);
    //tcp_client_send(tcp_client_pcb,cs);
    pbuf_take(cs->p_tx, (char*)buf_str, strlen((char*)buf_str));
    tcp_client_send(tcp_client_pcb,cs);
    //tcp_write(tcp_client_pcb, (void*)buf_str, strlen(buf_str)-1, 1);
    //tcp_output(tcp_client_pcb);    
}

void UART6_RxCpltCallback(void)
{
    char* str1 = "RxCallback\n";
    sendStr(str1);
    tcp_client_connect();
}
