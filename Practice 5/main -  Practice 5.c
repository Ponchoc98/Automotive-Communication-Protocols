#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_can.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/can.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/rom_map.h"

#define CANBAUD 125000

volatile bool g_bRXFlag = 0;
volatile bool g_bErrFlag = 0;
volatile unsigned int uiErrorCount = 0;

tCANMsgObject sCANTxMessage;
tCANMsgObject sCANRxMessage;
uint32_t ui32TxMsgData;
uint8_t pui4TxMsgData[5];
uint8_t pui4RxMsgData[5];
volatile unsigned int i=0;

void CANConf(void);
void InitConsole(void);
void CANIntHandler(void);
void SimpleDelay(void);


void
main(void)
{
    /*pui4TxMsgData[0] = 'A';
    pui4TxMsgData[1] = 'B';
    pui4TxMsgData[2] = 'C';
    pui4TxMsgData[3] = 'D';
    pui4TxMsgData[4] = 'E';*/

    pui4TxMsgData[0] = 0x01;
    pui4TxMsgData[1] = 0x02;
    pui4TxMsgData[2] = 0x03;
    pui4TxMsgData[3] = 0x04;
    pui4TxMsgData[4] = 0x05;
    unsigned int uIdx;


    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    InitConsole();
    CANConf();

    while(1)
    {
        if(g_bRXFlag)
        {
            HWREG(CAN0_BASE + CAN_O_IF1MCTL) |= CAN_IF1MCTL_UMASK;
            HWREG(CAN0_BASE + CAN_O_IF1MSK1) = 0x03;
            CANMessageGet(CAN0_BASE, 1, &sCANRxMessage, 0);
            sCANRxMessage.pui8MsgData  = pui4RxMsgData;
            g_bRXFlag = 0;

            if(sCANRxMessage.ui32Flags & MSG_OBJ_DATA_LOST)
            {
                UARTprintf("CAN message loss detected\n");
            }

            UARTprintf("\nReceived Msg ID=0x%02X len=%u data= ", sCANRxMessage.ui32MsgID, sCANRxMessage.ui32MsgLen);

            for(uIdx = 0; uIdx < sCANRxMessage.ui32MsgLen; uIdx++)
            {
                UARTprintf("%c", pui4RxMsgData[uIdx]);
            }

            UARTprintf("\n\n\n");
        }

        if((CANStatusGet(CAN0_BASE, CAN_STS_TXREQUEST) & 2) == 0)
        {
            UARTprintf("Sending msg: %c%c%c%c%c \n\n", pui4TxMsgData[0],pui4TxMsgData[1],pui4TxMsgData[2],pui4TxMsgData[3],pui4TxMsgData[4]);
            CANMessageSet(CAN0_BASE, 2, &sCANTxMessage, MSG_OBJ_TYPE_TX);

            if(g_bErrFlag)
            {
                UARTprintf(" error - cable connected?\n");
            }
                else
                {
                    UARTprintf(" total count = %u\n", i);

                }
        }

        SimpleDelay();

    }
}



void
InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}


void SimpleDelay(void)
{
    SysCtlDelay(16000000 / 3);
}


void CANIntHandler(void)
{
    uint32_t ui32Status;
    ui32Status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);
    if(ui32Status == CAN_INT_INTID_STATUS)
    {
        ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
        g_bErrFlag = 1;
        uiErrorCount++;
    }

        else if(ui32Status == 1)
        {
            CANIntClear(CAN0_BASE, 1);
            g_bRXFlag = 1;
            g_bErrFlag = 0;
            i++;
        }

            else
            {
                // do nothing
            }
}

void CANConf(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinConfigure(GPIO_PE4_CAN0RX);
    GPIOPinConfigure(GPIO_PE5_CAN0TX);
    GPIOPinTypeCAN(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);
    CANInit(CAN0_BASE);
    CANBitRateSet(CAN0_BASE, SysCtlClockGet(), CANBAUD);
    CANIntRegister(CAN0_BASE, CANIntHandler);
    CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR);

    IntEnable(INT_CAN0);
    HWREG(CAN0_BASE + CAN_O_CTL) |= CAN_CTL_TEST; //Enable test
    HWREG(CAN0_BASE + CAN_O_TST) |= CAN_TST_LBACK; //Enable loopback  + Silent
    HWREG(CAN0_BASE + CAN_O_IF1MCTL) |= CAN_IF1MCTL_UMASK;
    HWREG(CAN0_BASE + CAN_O_IF1MSK1) = 0x03;
    //HWREG(CAN0_BASE + CAN_O_IF2MCTL) |= CAN_IF2MCTL_UMASK;
    CANEnable(CAN0_BASE);
    sCANRxMessage.ui32MsgID = 1;
    sCANRxMessage.ui32MsgIDMask = 0x03; //*************************************************************************************************************Rx
    sCANRxMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
    sCANRxMessage.ui32MsgLen = 8;
    CANMessageSet(CAN0_BASE, 1, &sCANRxMessage, MSG_OBJ_TYPE_RX);
    ui32TxMsgData = 0;
    sCANTxMessage.ui32MsgID = 1;
    sCANTxMessage.ui32MsgIDMask = 0; //*************************************************************************************************************Tx
    sCANTxMessage.ui32Flags = MSG_OBJ_NO_FLAGS;
    sCANTxMessage.ui32MsgLen = sizeof(pui4TxMsgData);
    sCANTxMessage.pui8MsgData = pui4TxMsgData;
}
