/*
 ***************************************************************************************************************
 ***************************************************************************************************************
 ***************************************************************************************************************

 File:		  	   tcpClientRAW.c
 Modified By:     ControllersTech.com
 Updated:    	   29-Jul-2021

 ***************************************************************************************************************
 Copyright (C) 2017 ControllersTech.com

 This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
 of the GNU General Public License version 3 as published by the Free Software Foundation.
 This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
 or indirectly by this software, read more about this on the GNU General Public License.

 ***************************************************************************************************************
 */

/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of and a contribution to the lwIP TCP/IP stack.
 *
 * Credits go to Adam Dunkels (and the current maintainers) of this software.
 *
 * Christiaan Simons rewrote this file to get a more stable  application.
 *
 **/

/* This file was modified by ST */
#include<stdlib.h>
#include "tcpClientRAW.h"

#include "lwip/tcp.h"
#include "cmsis_os.h"
#include<string.h>
#define BUFFSIZE 614400
extern uint8_t testsram[BUFFSIZE];
extern uint8_t abc[960];
extern volatile int echo_run;
extern int resend_no;
extern  int all_circle,left_bytes;

char introduction[4];
/*  protocol states */
enum tcp_client_states {
	ES_NONE = 0, ES_CONNECTED, ES_RECEIVING, ES_CLOSING
};
int circle_time=0;

/* structure for maintaining connection infos to be passed as argument
 to LwIP callbacks*/
struct tcp_client_struct {
	u8_t state; /* current connection state */
	u8_t retries;
	struct tcp_pcb *pcb; /* pointer on the current tcp_pcb */
	struct pbuf *p; /* pointer on the received/to be transmitted pbuf */
};

/* This callback will be called, when the client is connected to the server */
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);

/* This callback will be called, when the client receive data from the server */
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
		err_t err);

/* This callback will be called, when the server Polls for the Client */
static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb);
void resend(int counter);
/* This callback will be called, when the server acknowledges the data sent by the client */
static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

/* A Function to send the data to the server */
static void tcp_client_send(struct tcp_pcb *tpcb, struct tcp_client_struct *es);

/* Function to close the connection */
static void tcp_client_connection_close(struct tcp_pcb *tpcb,
		struct tcp_client_struct *es);

/* This is the part where we are going to handle the incoming data from the server */
static void tcp_client_handle(struct tcp_pcb *tpcb,
		struct tcp_client_struct *es);
void echo();
void send_poolsize(int counter);

volatile int counter = 0;
volatile int len;

extern TIM_HandleTypeDef htim1;

/* create a struct to store data */
static struct tcp_client_struct *esTx = 0;
static struct tcp_pcb *pcbTx = 0;
int numarr[2];

void echo(void) {
	len = 952;
	int step=952;
	counter = 0;
	int circles_end=(int)(614400/len);
	int last_circle_bytes=614400%len;
	if(last_circle_bytes!=0)circles_end+=1;
	circle_time++;
	int persize=960;
	numarr[0]=circle_time;
	osDelay(5);
	while (counter < circles_end) {

		//the last one of the circles
		if(counter==(circles_end-1)&&last_circle_bytes!=0)persize=last_circle_bytes+8;

		esTx->p = pbuf_alloc(PBUF_RAW, persize, PBUF_POOL);
		numarr[1]=counter;

//		if(counter >=643 &&counter <645){
//			numarr[1]=2000;
//
//		}

		memcpy(abc,numarr, 8);

		memcpy(abc+8,testsram+step*counter,len);

		if(esTx->p!=NULL){

		//pbuf_take(esTx->p,abc, len);
		pbuf_take(esTx->p,abc, persize);
		tcp_client_send(pcbTx, esTx);

//		if(esTx->p!=NULL)

		pbuf_free(esTx->p);

		counter++;

		}
	}

	echo_run =0;


}



void send_poolsize(int counter) {
	len = 1400-8;
	int step=1400-8;
	int counter_end =counter+ 3;

	if (counter_end>all_circle)
		counter_end=all_circle;
	circle_time++;
	int persize=1400;
	numarr[0]=circle_time;
//	printf("counter=%d,end=%d\r\n",counter,counter_end);

	while (counter < counter_end) {

		//the last one of the circles
		if(counter==(all_circle-1)&&left_bytes!=0)persize=left_bytes+8;

		esTx->p = pbuf_alloc(PBUF_RAW, persize, PBUF_POOL);
		numarr[1]=counter;

		memcpy(abc,numarr, 8);

		memcpy(abc+8,testsram+step*counter,len);

		if(esTx->p!=NULL){

		pbuf_take(esTx->p,abc, persize);

		tcp_client_send(pcbTx, esTx);

		pbuf_free(esTx->p);
		counter++;

		}
	}

}

void resend(int counter) {
	len = 952;
	int step=952;

	numarr[0]=circle_time;
	int circles_end=(int)(BUFFSIZE/len);
	int persize=952;
	int last_circle_bytes=BUFFSIZE%len;
		//the last one of the circles
		if(counter==(circles_end-1)&&last_circle_bytes!=0)persize=last_circle_bytes+8;
		esTx->p = pbuf_alloc(PBUF_RAW, persize, PBUF_POOL);
		numarr[1]=counter;
		printf("counter =%d\r\n",counter);
		memcpy(abc,numarr, 8);
		memcpy(abc+8,testsram+step*counter,len);

		if(esTx->p!=NULL){

		//pbuf_take(esTx->p,abc, len);


		pbuf_take(esTx->p,abc, persize);


		tcp_client_send(pcbTx, esTx);

//		if(esTx->p!=NULL)
		pbuf_free(esTx->p);
		}


}


//void echo(void) {
//
//
//
//	 len = 960;
//	int step=960;
//	counter = 0;
//	int circles_end=(int)(614400/len);
//	int last_circle_bytes=614400%len;
//	if(last_circle_bytes!=0)circles_end+=1;
//	printf("circle end:%d\r\n",circles_end);
//	while (counter < circles_end) {
//
//		//the last one of the circles
//		if(counter==(circles_end-1)&&last_circle_bytes!=0)len=last_circle_bytes;
//
//		esTx->p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
//
//
//
//
//		if(esTx->p!=NULL){
//		osDelay(1);
//
//		pbuf_take(esTx->p,testsram+step*counter, len);
//
//		osDelay(5);
//
//		tcp_client_send(pcbTx, esTx);
//		counter++;
//
//		}
//		//if(esTx->p!=NULL)
//		pbuf_free(esTx->p);
//	}
//
//
//
//}

/* IMPLEMENTATION FOR TCP CLIENT

 1. Create TCP block.
 2. connect to the server
 3. start communicating
 */

void tcp_client_init(void) {
	/* 1. create new tcp pcb */
	struct tcp_pcb *tpcb;

	tpcb = tcp_new();

	/* 2. Connect to the server */
	ip_addr_t destIPADDR;
	IP_ADDR4(&destIPADDR, 192, 168, 1, 7);
	tcp_connect(tpcb, &destIPADDR, 12345, tcp_client_connected);
	 printf("hello world 5\r\n");

}

/** This callback is called, when the client is connected to the server
 * Here we will initialise few other callbacks
 * and in the end, call the client handle function
 */
static err_t tcp_client_connected(void *arg, struct tcp_pcb *newpcb, err_t err) {
	err_t ret_err;
	struct tcp_client_struct *es;

	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	/* allocate structure es to maintain tcp connection information */
	es = (struct tcp_client_struct*) mem_malloc(
			sizeof(struct tcp_client_struct));
	if (es != NULL) {
		es->state = ES_CONNECTED;
		es->pcb = newpcb;
		es->retries = 0;
		es->p = NULL;

		/* pass newly allocated es structure as argument to newpcb */
		tcp_arg(newpcb, es);

		/* initialize lwip tcp_recv callback function for newpcb  */
		tcp_recv(newpcb, tcp_client_recv);

		/* initialize lwip tcp_poll callback function for newpcb */
		tcp_poll(newpcb, tcp_client_poll, 0);

		/* initialize LwIP tcp_sent callback function */
		//tcp_sent(newpcb, tcp_client_sent);

		/* handle the TCP data */
		tcp_client_handle(newpcb, es);

		ret_err = ERR_OK;
	} else {
		/*  close tcp connection */
		tcp_client_connection_close(newpcb, es);
		/* return memory error */
		ret_err = ERR_MEM;
	}
	return ret_err;
}

/** This callback is called, when the client receives some data from the server
 * if the data received is valid, we will handle the data in the client handle function
 */
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
		err_t err) {
	struct tcp_client_struct *es;
	err_t ret_err;

	LWIP_ASSERT("arg != NULL", arg != NULL);

	es = (struct tcp_client_struct*) arg;

	/* if we receive an empty tcp frame from server => close connection */
	if (p == NULL) {
		/* remote host closed connection */
		es->state = ES_CLOSING;
		if (es->p == NULL) {
			/* we're done sending, close connection */
			tcp_client_connection_close(tpcb, es);
		} else {
			/* we're not done yet */
//      /* acknowledge received packet */
//      tcp_sent(tpcb, tcp_client_sent);
			/* send remaining data*/
//      tcp_client_send(tpcb, es);
		}
		ret_err = ERR_OK;
	}
	/* else : a non empty frame was received from server but for some reason err != ERR_OK */
	else if (err != ERR_OK) {
		/* free received pbuf*/
		if (p != NULL) {
			es->p = NULL;
			pbuf_free(p);
		}
		ret_err = err;
	} else if (es->state == ES_CONNECTED) {
		/* store reference to incoming pbuf (chain) */
		//es->p = p;
		struct pbuf *ptmp=p;
		char num[3];
		while(ptmp!=NULL){

			for(int i=0;i<p->len;i++){

				//printf("%c",*((char *)p->payload+i));
				num[i]=*((char *)p->payload+i);
			}
			ptmp=p->next;
		}
		resend_no=atoi(num);
//
		if(resend_no==6666){
			echo_run=1;
		}else if(resend_no>=(all_circle+1)){
			//printf("send pool_size %d\r\n",all_circle+1);
			resend_no-=all_circle+1;
			send_poolsize(resend_no);

		}
		else{
			//resend_no=receive;
			printf("send resend_no %d\r\n",all_circle+1);

			resend(resend_no);
		}

		// tcp_sent has already been initialized in the beginning.
//    /* initialize LwIP tcp_sent callback function */
//    tcp_sent(tpcb, tcp_client_sent);



		/* Acknowledge the received data */
		tcp_recved(tpcb, p->tot_len);

		/* handle the received data */
		pbuf_free(p);

		ret_err = ERR_OK;
	} else if (es->state == ES_CLOSING) {
		/* odd case, remote side closing twice, trash data */
		tcp_recved(tpcb, p->tot_len);
		es->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
	} else {
		/* unknown es->state, trash data  */
		tcp_recved(tpcb, p->tot_len);
		es->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
	}
	return ret_err;
}

static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb) {
	err_t ret_err;
	struct tcp_client_struct *es;

	es = (struct tcp_client_struct*) arg;
	if (es != NULL) {
		if (es->p != NULL) {
			// tcp_sent has already been initialized in the beginning.
//      tcp_sent(tpcb, tcp_client_sent);
			/* there is a remaining pbuf (chain) , try to send data */
//      tcp_client_send(tpcb, es);
		} else {
			/* no remaining pbuf (chain)  */
			if (es->state == ES_CLOSING) {
				/*  close tcp connection */
				tcp_client_connection_close(tpcb, es);
			}
		}
		ret_err = ERR_OK;
	} else {
		/* nothing to be done */
		tcp_abort(tpcb);
		ret_err = ERR_ABRT;
	}
	return ret_err;
}

/** This callback is called, when the server acknowledges the data sent by the client
 * If there is no more data left to sent, we will simply close the connection
 */
static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
	struct tcp_client_struct *es;

	LWIP_UNUSED_ARG(len);
	//	HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_9);

	es = (struct tcp_client_struct*) arg;
	es->retries = 0;

	if (es->p != NULL) {
		// tcp_sent has already been initialized in the beginning.
		/* still got pbufs to send */
//    tcp_sent(tpcb, tcp_client_sent);

//    tcp_client_send(tpcb, es);
	} else {
		/* if no more data to send and client closed connection*/
		if (es->state == ES_CLOSING)
			tcp_client_connection_close(tpcb, es);
	}
	return ERR_OK;
}

/** A function to send the data to the server
 */
static void tcp_client_send(struct tcp_pcb *tpcb,struct tcp_client_struct*es) {
	  struct pbuf *ptr;
	  err_t wr_err = ERR_OK;



	    /* get pointer on pbuf from es structure */
	    ptr = es->p;
	  //  taskENTER_CRITICAL();
		   // taskEXIT_CRITICAL();

	    /* enqueue data for transmission */

	//NVIC_DisableIRQ(ETH_IRQn);
//
	    wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);


		//HAL_NVIC_EnableIRQ(ETH_IRQn);

	//	NVIC_DisableIRQ(ETH_IRQn);
//
	    	wr_err=tcp_output(tpcb);
//
//


		//	HAL_NVIC_EnableIRQ(ETH_IRQn);


//			if(counter==300){
//						memset(abc,0,5);
//						memcpy(abc,(uint8_t *)(ptr->payload),5);
//
//						printf("circle time %d : %d,%d,%d,%d,%d,\r\n",circle_time, abc[0],abc[1],abc[2],abc[3],abc[4]);
//					}
		  //  taskEXIT_CRITICAL();




		  //  taskEXIT_CRITICAL();



}




//
//static void tcp_client_send(struct tcp_pcb *tpcb,struct tcp_client_struct*es) {
//	  struct pbuf *ptr;
//	  err_t wr_err = ERR_OK;
//
//	  while ((wr_err == ERR_OK) &&
//	         (es->p != NULL) &&
//	         (es->p->len <= tcp_sndbuf(tpcb)))
//	  {
//
//	    /* get pointer on pbuf from es structure */
//	    ptr = es->p;
//	    taskENTER_CRITICAL();
//	    /* enqueue data for transmission */
//	NVIC_DisableIRQ(ETH_IRQn);
////
//	    wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
//		 //   taskEXIT_CRITICAL();
//
//		//HAL_NVIC_EnableIRQ(ETH_IRQn);
//	//	NVIC_DisableIRQ(ETH_IRQn);
//	   // taskENTER_CRITICAL();
//
//			tcp_output(tpcb);
//		  //  taskEXIT_CRITICAL();
//
//			HAL_NVIC_EnableIRQ(ETH_IRQn);
//
//
//		    taskEXIT_CRITICAL();
//
//	    if (wr_err == ERR_OK)
//	    {
//	      u16_t plen;
//	      u8_t freed;
//
//	      plen = ptr->len;
//
//	      /* continue with next pbuf in chain (if any) */
//	      es->p = ptr->next;
//
//	      if(es->p != NULL)
//	      {
//	        /* increment reference count for es->p */
//	        pbuf_ref(es->p);
//	      }
//
//	     /* chop first pbuf from chain */
//	      do
//	      {
//	        /* try hard to free pbuf */
//	        freed = pbuf_free(ptr);
//	      }
//	      while(freed == 0);
//	     /* we can read more data now */
//	    tcp_recved(tpcb, plen);
//	   }
//	   else if(wr_err == ERR_MEM)
//	   {
//	      /* we are low on memory, try later / harder, defer to poll */
//	     es->p = ptr;
//	   }
//	   else
//	   {
//	     /* other problem ?? */
//	   }
//	  }
//}




static void tcp_client_connection_close(struct tcp_pcb *tpcb,
		struct tcp_client_struct *es) {

	/* remove all callbacks */
	tcp_arg(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, 0);

	/* delete es structure */
	if (es != NULL) {
		mem_free(es);
	}

	/* close tcp connection */
	tcp_close(tpcb);
}

/* Handle the incoming TCP Data */

static void tcp_client_handle(struct tcp_pcb *tpcb,
		struct tcp_client_struct *es) {
	/* get the Remote IP */

	/* Extract the IP */

//	esTx->state = es->state;
//	esTx->pcb = es->pcb;
//	esTx->p = es->p;
	esTx = es;
	pcbTx = tpcb;

}
