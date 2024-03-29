/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_uart.h"
#include "fsl_debug_console.h"
#include "MK64F12.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* UART instance and clock */
#define DEMO_UART          	UART1
#define DEMO_UART_CLKSRC   	UART1_CLK_SRC
#define SLAVE_UART_CLKSRC	UART4_CLK_SRC
#define SLAVE_UART			UART4

#define DEMO_UART_CLK_FREQ CLOCK_GetFreq(UART1_CLK_SRC)

#define DELAY_TIME         100000U
#define TRANSFER_SIZE     4u//256U    /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 9600 /*! Transfer baudrate - 115200 */

#define ID_MASK			   0x1
#define MAX_SIZE		   9U

typedef struct
{
	uint8_t ID;
}lin_header_t;

typedef struct
{
	uint8_t length;
	uint8_t data1;
	uint8_t data2;
	uint8_t data3;
	uint8_t data4;
	uint8_t data5;
	uint8_t data6;
	uint8_t data7;
	uint8_t data8;
	uint8_t checksum;
}lin_response_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* UART user callback */
void UART_UserCallback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData);

void LIN_Set_ID(uint8_t id, uint8_t length_control,lin_header_t* header);
void LIN_init_response(lin_response_t* response);
uint8_t LIN_Get_Checksum(lin_response_t cks);
uint8_t LIN_lookup_response(lin_response_t* response, uint8_t id);
void LIN_master(uint8_t id, uint8_t length);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t transferRxData[TRANSFER_SIZE] = {0U};
uint8_t transferTxData[TRANSFER_SIZE] = {0U};
uart_handle_t g_uartHandle;
volatile bool isTransferCompleted = false;

lin_response_t messages;
lin_header_t head;
uint8_t LIN_slave_mailbox[TRANSFER_SIZE];
uint8_t g_message_length;
uint8_t LIN_master_mailbox[MAX_SIZE];
uint8_t g_uart_master;
uint8_t g_uart_slave;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* PUART user callback */
void UART_UserCallback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_UART_TxIdle == status)
    {
        isTransferCompleted = true;
    }
}
/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0U, errCount = 0U;
    status_t status = 0;
    uart_config_t config;
    uart_transfer_t sendXfer;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    BOARD_InitDebugConsole();
    PRINTF("This is UART hardware flow control example on one board.\r\n");
    PRINTF("This example will send data to itself and will use hardware flow control to avoid the overflow.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is: \r\n");
    PRINTF("      UART_TX    --     UART_RX    \r\n");
    PRINTF("      UART_RTS   --     UART_CTS   \r\n");
    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUART_ParityDisabled;
     * config.stopBitCount = kUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 0;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    UART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;
   // config.enableRxRTS  = true;
    //config.enableTxCTS  = true;

    UART_Init(DEMO_UART, &config, DEMO_UART_CLK_FREQ);
    g_uart_master = 1;//UART1
    g_uart_slave = 4;//UART4

    UART_TransferCreateHandle(DEMO_UART, &g_uartHandle, UART_UserCallback, NULL);

    /* Set up the transfer data */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        transferTxData[i] = i % 256U;
        transferRxData[i] = 0U;
    }

    sendXfer.data     = (uint8_t *)transferTxData;
    sendXfer.dataSize = TRANSFER_SIZE;
    UART_TransferSendNonBlocking(DEMO_UART, &g_uartHandle, &sendXfer);

    /* Delay for some time to let the RTS pin dessart. */
    for (i = 0U; i < DELAY_TIME; i++)
    {
        __NOP();
    }

    status = UART_ReadBlocking(DEMO_UART, transferRxData, TRANSFER_SIZE);
    if (kStatus_Success != status)
    {
        PRINTF(" Error occurred when UART receiving data.\r\n");
    }
    /* Wait for the transmit complete. */
    while (!isTransferCompleted)
    {
    }

    while (1)
    {
    }
}

void LIN_master(uint8_t id, uint8_t length)
{

	/**Send Synch Break*/
	uint8_t LIN_master_transferTx[TRANSFER_SIZE] = {0U};
	/**Synch Break*/
	LIN_master_transferTx[0] = 0x0;
	LIN_master_transferTx[1] = 0x1;
	/**Synch*/
	LIN_master_transferTx[2] = 0x55;
	/**ID*/
	LIN_Set_ID(id, length ,&head);
	LIN_master_transferTx[3] = head.ID;

	uint8_t dummy = UART_WriteBlocking(UART1, LIN_master_transferTx, TRANSFER_SIZE);
}

void LIN_master_receive()
{
	/**Wait for response*/
	uint8_t id_lenght = (( head.ID >> 2 )&0x3);
	uint8_t message_length_master;
	if((id_lenght == 0x0) || (id_lenght == 0x1 ))
	{
		message_length_master = 0x2;
	}
	else if(id_lenght == 0x2)
	{
		message_length_master = 0x4;
	}
	else
	{
		message_length_master = 0x8;
	}
	/***/
	lin_response_t master_received;
	uint8_t* data = &master_received.data1;
	uint8_t status = UART_ReadBlocking(UART1, data, message_length_master);
	for(int i = 0;i < message_length_master;i++,data++)
	{
		*data = LIN_master_mailbox[i];
	}
	uint8_t checksum = LIN_Get_Checksum(master_received);
	if( ( LIN_master_mailbox[message_length_master] ) == checksum )
	{
		printf("SUCCESS !!!!!!!!!\n\r");
	}
	else
	{
		printf("SUCCESSn't !!!!!!!!!\n\r");
	}
}


void LIN_Set_ID(uint8_t id, uint8_t length_control,lin_header_t* header)
{

	uint8_t parity_even = (( (id>>3) & ID_MASK ) ^ ( (id>>2) & ID_MASK ) ^ ( (id>>1) & ID_MASK ) ^
						  ( (length_control>>1) & ID_MASK ) )&0x1;
	uint8_t parity_odd = (~(( (id>>2) & ID_MASK ) ^ ( (id>>0) & ID_MASK ) ^ ( (length_control>>1) & ID_MASK ) ^
						 ( (length_control>>0) & ID_MASK ) ) )&0x1;
	header->ID = ((id << 4 )&0xF0) | ((length_control << 2)&0xC) | ((parity_even << 1)&0x2) |
				 ((parity_odd)&0x1);
}

uint8_t LIN_Get_Checksum(lin_response_t cks)
{
	uint8_t temp_cks1 = cks.data1 + cks.data2;
	if( (cks.data1 + cks.data2) > 0xFF )
	{
		temp_cks1 = temp_cks1 + 0x1;
	}
	uint8_t temp_cks2 = temp_cks1 + cks.data3;
	if( (temp_cks1 + cks.data3) > 0xFF )
	{
		temp_cks2 = temp_cks2 + 0x1;
	}
	uint8_t temp_cks3 = temp_cks2 + cks.data4;
	if( (temp_cks2 + cks.data4) > 0xFF )
	{
		temp_cks3 = temp_cks3 + 0x1;
	}
	uint8_t temp_cks4 = temp_cks3 + cks.data5;
	if( (temp_cks3 + cks.data5) > 0xFF )
	{
		temp_cks4 = temp_cks4 + 0x1;
	}
	uint8_t temp_cks5 = temp_cks4 + cks.data6;
	if( (temp_cks4 + cks.data6) > 0xFF )
	{
		temp_cks5 = temp_cks5 + 0x1;
	}
	uint8_t temp_cks6 = temp_cks5 + cks.data7;
	if( (temp_cks5 + cks.data7) > 0xFF )
	{
		temp_cks6 = temp_cks6 + 0x1;
	}
	uint8_t temp_cks7 = temp_cks6 + cks.data8;
	if( (temp_cks6 + cks.data8) > 0xFF )
	{
		temp_cks7 = temp_cks7 + 0x1;
	}
	temp_cks7 = ~(temp_cks7)+1;
	return temp_cks7;
}

void LIN_handler(uint8_t uart_channel)
{
	uint8_t data;
	static uint8_t amount = 0;
	if(uart_channel == g_uart_master)
	{
		printf("IN MASTER %x\n",data);
		LIN_master_mailbox[amount] = data;
		amount++;
		if(g_message_length+1 == amount)
		{
			amount = 0;
			LIN_master_receive();
		}
	}
	if(uart_channel == g_uart_slave)
	{
		printf("IN SLAVE %x \n", data);
		LIN_slave_mailbox[amount] = data;
		amount++;
		if(TRANSFER_SIZE == amount)
		{
			amount = 0;
			LIN_slave();
		}
	}
}

void LIN_init_response(lin_response_t* response)
{
	response->data1 = 0;
	response->data2 = 0;
	response->data3 = 0;
	response->data4 = 0;
	response->data5 = 0;
	response->data6 = 0;
	response->data7 = 0;
	response->data8 = 0;
}

uint8_t LIN_lookup_response(lin_response_t* response, uint8_t id)
{
	uint8_t* dato = 0;
	switch(id)
	{
	case 0x1:
		dato = &(response->data1);

		break;
	case 0x2:
		dato = &(response->data1);

		break;
	case 0x3:
		dato = &(response->data1);

		break;
	default:
		return 0;
	}
	return 1;
}
