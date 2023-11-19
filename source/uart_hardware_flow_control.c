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

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* UART instance and clock */
#define DEMO_UART          UART1
#define DEMO_UART_CLKSRC   UART1_CLK_SRC
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
    g_uart_master = master;
    g_uart_slave = slave;

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

    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        if (transferTxData[i] != transferRxData[i])
        {
            errCount++;
        }
    }
    if (errCount)
    {
        PRINTF("Data not matched! Transfer error.\r\n");
    }
    else
    {
        PRINTF("Data matched! Transfer successfully.\r\n");
    }
    /* Deinit the UART. */
    UART_Deinit(DEMO_UART);

    while (1)
    {
    }
}
