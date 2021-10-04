#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"


void sendOne(){
    int x=0;
    for(x =0;x<31;x++){
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(65);
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 0x0);
        SysCtlDelay(65);
    }


}

void sendZero(){
    SysCtlDelay(4478);
}


void main(void)
4{
    SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_OSC|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_1);

    while(1)
    {
        sendZero(); // Start
        sendOne();
        sendZero();
        sendOne();
        sendZero();
        sendOne();
        sendZero();
        sendOne();
        sendZero();
        sendOne(); // Stop
        SysCtlDelay(1333333);
    }

}
