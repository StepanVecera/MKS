/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */


#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TCPECHO_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )
#define CMD_BUFFER_LEN 512


static void http_client(char *s, uint16_t size)
{
	struct netconn *client;
	struct netbuf *buf;
	ip_addr_t ip;
	uint16_t len = 0;
	IP_ADDR4(&ip, 147,229,144,124);
	const char *request = "GET /ip.php HTTP/1.1\r\n"
			"Host: www.urel.feec.vutbr.cz\r\n"
			"Connection: close\r\n"
			"\r\n\r\n";
	client = netconn_new(NETCONN_TCP);
	if (netconn_connect(client, &ip, 80) == ERR_OK) {
		netconn_write(client, request, strlen(request), NETCONN_COPY);
		// Receive the HTTP response
		s[0] = 0;
		while (len < size && netconn_recv(client, &buf) == ERR_OK) {
			len += netbuf_copy(buf, &s[len], size-len);
			s[len] = 0;
			netbuf_delete(buf);
		}
	} else {
		sprintf(s, "Chyba pripojeni\n");
	}
	netconn_delete(client);
}



static void telnet_process_command(char *cmd, struct netconn *conn){
	char *token;
	char output_msg_buffer[CMD_BUFFER_LEN];

	if(strlen(cmd)==0){
		return;
	}

	// HELLO
	token = strtok(cmd, " ");
	if (strcasecmp(token, "HELLO") == 0) {
		sprintf(output_msg_buffer, "Komunikace OK\n\r");
		// netconn_write(conn, output_msg_buffer, strlen(output_msg_buffer), NETCONN_COPY);
		// LED1
	} else if (strcasecmp(token, "LED1") == 0) {
		token = strtok(NULL, " ");
		if (strcasecmp(token, "ON") == 0) {
			HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
			sprintf(output_msg_buffer, "Done\n\r");

		} else if (strcasecmp(token, "OFF") == 0) {
			HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
			sprintf(output_msg_buffer, "Done\n\r");
		}
	}
	// LED2
	else if (strcasecmp(token, "LED2") == 0) {
		token = strtok(NULL, " ");
		if (strcasecmp(token, "ON") == 0) {
			HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
			sprintf(output_msg_buffer, "Done\n\r");
		} else if (strcasecmp(token, "OFF") == 0) {
			HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
			sprintf(output_msg_buffer, "Done\n\r");
		}
	}

	// LED3
	else if (strcasecmp(token, "LED3") == 0) {
		token = strtok(NULL, " ");
		if (strcasecmp(token, "ON") == 0) {
			HAL_GPIO_WritePin(led3_GPIO_Port, led3_Pin, GPIO_PIN_SET);
			sprintf(output_msg_buffer, "Done\n\r");
		} else if (strcasecmp(token, "OFF") == 0) {
			HAL_GPIO_WritePin(led3_GPIO_Port, led3_Pin, GPIO_PIN_RESET);
			sprintf(output_msg_buffer, "Done\n\r");
		}
	}
	// CLIENT
	else if (strcasecmp(token, "CLIENT") == 0) {

		http_client(output_msg_buffer, CMD_BUFFER_LEN);
	}


	// STATUS
	else if (strcasecmp(token, "STATUS") == 0) {
		uint8_t LED1 = HAL_GPIO_ReadPin(led1_GPIO_Port, led1_Pin);
		uint8_t LED2 = HAL_GPIO_ReadPin(led2_GPIO_Port, led2_Pin);
		uint8_t LED3 = HAL_GPIO_ReadPin(led3_GPIO_Port, led3_Pin);
		char LED1_text[18];
		char LED2_text[18];
		char LED3_text[18];
		if (LED1 == 0) {
			strcpy(LED1_text, "OFF");
		} else if (LED1 == 1) {
			strcpy(LED1_text, "ON");
		}
		if (LED2 == 0) {
			strcpy(LED2_text, "OFF");
		} else if (LED2 == 1) {
			strcpy(LED2_text, "ON");
		}
		if (LED3 == 0) {
			strcpy(LED3_text, "OFF");
		} else if (LED3 == 1) {
			strcpy(LED3_text, "ON");

		}
		sprintf(output_msg_buffer, "LED 1 STATUS:'%s' LED 2 STATUS:'%s' LED 3 STATUS:'%s'\n\r", LED1_text, LED2_text, LED3_text);
	}
	netconn_write(conn, output_msg_buffer, strlen(output_msg_buffer), NETCONN_COPY);
}



static void telnet_byte_available(uint8_t c, struct netconn *conn)
{
	static uint16_t cnt;
	static char data[CMD_BUFFER_LEN];
	if (cnt < CMD_BUFFER_LEN && c >= 32 && c <= 127) data[cnt++] = c;
	if (c == '\n' || c == '\r') {
		data[cnt] = '\0';
		telnet_process_command(data, conn);
		cnt = 0;
	}
}




/*-----------------------------------------------------------------------------------*/
static void telnet_thread(void *arg)
{
	struct netconn *conn, *newconn;
	err_t err, accept_err;
	struct netbuf *buf;
	uint8_t *data;

	u16_t len;

	LWIP_UNUSED_ARG(arg);

	/* Create a new connection identifier. */
	conn = netconn_new(NETCONN_TCP);

	if (conn!=NULL)
	{
		/* Bind connection to well known port number 23. */
		err = netconn_bind(conn, NULL, 23);

		if (err == ERR_OK)
		{
			/* Tell connection to go into listening mode. */
			netconn_listen(conn);

			while (1)
			{
				/* Grab new connection. */
				accept_err = netconn_accept(conn, &newconn);

				/* Process the new connection. */
				if (accept_err == ERR_OK)
				{

					while (netconn_recv(newconn, &buf) == ERR_OK)
					{
						do
						{
							netbuf_data(buf, (void**)&data, &len);
							while (len--) telnet_byte_available(*data++, newconn);
						}
						while (netbuf_next(buf) >= 0);

						netbuf_delete(buf);
					}

					/* Close connection and discard connection identifier. */
					netconn_close(newconn);
					netconn_delete(newconn);
				}
			}
		}
		else
		{
			netconn_delete(newconn);
		}
	}
}
/*-----------------------------------------------------------------------------------*/

void telnet_init(void)
{
	sys_thread_new("telnet_thread", telnet_thread, NULL, DEFAULT_THREAD_STACKSIZE, TCPECHO_THREAD_PRIO);
}

/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
