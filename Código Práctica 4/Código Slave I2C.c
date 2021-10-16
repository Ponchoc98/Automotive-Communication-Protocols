#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "driverlib/pwm.h"

//static uint32_t g_ui32DataRx;
static uint32_t Boton;
static uint32_t ValIntensity;
static uint32_t ValFlashing;
int x = 0;


//initialize I2C module 0
//Slightly modified version of TI's example code
void InitI2C0(void)
{
    //enable I2C module 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    //reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    //enable GPIO peripheral that contains I2C 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    //I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
    I2CSlaveEnable(I2C0_BASE);
    I2CSlaveInit(I2C0_BASE, 0x3C);

    //clear I2C FIFOs
    HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
}

void I2C0SlaveIntHandler(void)
{
    if(x == 0)
    {
        // Clear the I2C0 interrupt flag.
        I2CSlaveIntClear(I2C0_BASE);
        // Read the data from the slave.
        Boton = I2CSlaveDataGet(I2C0_BASE);
        I2CSlaveStatus(I2C0_BASE);
        x++;
    }
    else if (x == 1)
    {
        if(Boton == 0x00)
        {
            // Clear the I2C0 interrupt flag.
            I2CSlaveIntClear(I2C0_BASE);
            // Read the data from the slave.
            ValIntensity = I2CSlaveDataGet(I2C0_BASE);
            I2CSlaveStatus(I2C0_BASE);
            x=0;
        }
        else if(Boton == 0x01)
        {
            // Clear the I2C0 interrupt flag.
            I2CSlaveIntClear(I2C0_BASE);
            // Read the data from the slave.
            ValFlashing = I2CSlaveDataGet(I2C0_BASE);
            I2CSlaveStatus(I2C0_BASE);
            x=0;
        }

    }
}

void FlashingSpeed(void)
{
    if(ValFlashing==1)
    {
        PWMGenDisable(PWM1_BASE, PWM_GEN_2);
        SysCtlDelay((SysCtlClockGet() / 3) /32 );
        PWMGenEnable(PWM1_BASE, PWM_GEN_2);
        SysCtlDelay((SysCtlClockGet() / 3) /32 );
    }
    else if(ValFlashing==2)
    {
        PWMGenDisable(PWM1_BASE, PWM_GEN_2);
        SysCtlDelay((SysCtlClockGet() / 3) /16 );
        PWMGenEnable(PWM1_BASE, PWM_GEN_2);
        SysCtlDelay((SysCtlClockGet() / 3) /16 );
    }
    else if(ValFlashing==3)
    {
        PWMGenDisable(PWM1_BASE, PWM_GEN_2);
        SysCtlDelay((SysCtlClockGet() / 3) /8 );
        PWMGenEnable(PWM1_BASE, PWM_GEN_2);
        SysCtlDelay((SysCtlClockGet() / 3) /8 );
    }
    else if(ValFlashing==4)
    {
        PWMGenDisable(PWM1_BASE, PWM_GEN_2);
        SysCtlDelay((SysCtlClockGet() / 3) /4 );
        PWMGenEnable(PWM1_BASE, PWM_GEN_2);
        SysCtlDelay((SysCtlClockGet() / 3) /4 );
    }
}

void Intensity(void)
{
    if(ValIntensity==1)
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, (PWMGenPeriodGet(PWM1_BASE, PWM_GEN_2) / 520));
        //PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 100);
    }
    else if(ValIntensity==2)
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, (PWMGenPeriodGet(PWM1_BASE, PWM_GEN_2) / 128));
    }
    else if(ValIntensity==3)
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, (PWMGenPeriodGet(PWM1_BASE, PWM_GEN_2) / 64));
    }
    else if(ValIntensity==4)
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, (PWMGenPeriodGet(PWM1_BASE, PWM_GEN_2) / 2));
    }
}

void BotonEleccion(void)
{
    if(Boton == 0x01)
    {
        FlashingSpeed();
    }
    else if(Boton == 0x00)
    {
        Intensity();
    }
}

void GenPWM(void)
{
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinConfigure(GPIO_PF1_M1PWM5);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 64000);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 100);
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT , true);
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);
}

void main(void)
{
    //uint32_t lectura;
    //uint32_t ReadI2C;
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    InitI2C0();
    GenPWM();

    IntEnable(INT_I2C0);
    I2CSlaveIntEnableEx(I2C0_BASE, I2C_SLAVE_INT_DATA);
    I2CIntRegister(I2C0_BASE, I2C0SlaveIntHandler);
    IntMasterEnable();

    while(1)
    {
        BotonEleccion();
    }


}
