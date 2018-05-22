#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#define SCLPin GPIO_PIN_1
#define SDAPin GPIO_PIN_0
#define slaveAddr 0x68

int8_t TEMP_H, TEMP_L, ACCEL_X_H, ACCEL_X_L;
int8_t ACCEL_Y_H, ACCEL_Y_L, ACCEL_Z_H, ACCEL_Z_L;
uint8_t WHO_AM_I;
uint32_t g_ui32SysClock;
float TEMP, ACCEL_X, ACCEL_Y, ACCEL_Z;

void sendByte(uint8_t REG, uint8_t DATA){
    I2CMasterSlaveAddrSet(I2C2_BASE, slaveAddr, false);     //false = write ; true = read
    I2CMasterDataPut(I2C2_BASE, REG);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C2_BASE));
    I2CMasterSlaveAddrSet(I2C2_BASE, slaveAddr, false);
    I2CMasterDataPut(I2C2_BASE, DATA);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C2_BASE));
}

int8_t readByte(uint8_t REG){
    I2CMasterSlaveAddrSet(I2C2_BASE, slaveAddr, false);     //false = write ; true = read
    I2CMasterDataPut(I2C2_BASE, REG);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C2_BASE));
    I2CMasterSlaveAddrSet(I2C2_BASE, slaveAddr, true);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    while(I2CMasterBusy(I2C2_BASE));
    return I2CMasterDataGet(I2C2_BASE);
}

int main(void)
{
    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                        SYSCTL_OSC_MAIN |
                                        SYSCTL_USE_PLL |
                                        SYSCTL_CFG_VCO_480), 120000000);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C2));

    GPIOPinTypeI2C(GPIO_PORTL_BASE, SDAPin);
    GPIOPinTypeI2CSCL(GPIO_PORTL_BASE, SCLPin);
    GPIOPinConfigure(GPIO_PL0_I2C2SDA);
    GPIOPinConfigure(GPIO_PL1_I2C2SCL);

    I2CMasterInitExpClk(I2C2_BASE, g_ui32SysClock, true);

    sendByte(0x6B, 0x01);                   //Sai do modo sleep (PWR_MGMT)
    SysCtlDelay(g_ui32SysClock/12000000);   //Aguarda inicialização
    sendByte(0x1C, 0x00);                   //ACCEL_CONFIG
    WHO_AM_I = readByte(0x75);

    while(1){
        SysCtlDelay(g_ui32SysClock/130000);
        TEMP_H = readByte(0x41);
        TEMP_L = readByte(0x42);
        TEMP = (TEMP_H << 8) + TEMP_L;
        TEMP = TEMP/340 + 36.53;

        ACCEL_X_H = readByte(0x3B);
        ACCEL_X_L = readByte(0x3C);
        ACCEL_X = (ACCEL_X_H << 8) + ACCEL_X_L;
        ACCEL_X = ACCEL_X/16384;

        ACCEL_Y_H = readByte(0x3D);
        ACCEL_Y_L = readByte(0x3E);
        ACCEL_Y = (ACCEL_Y_H << 8) + ACCEL_Y_L;
        ACCEL_Y = ACCEL_Y/16384;

        ACCEL_Z_H = readByte(0x3F);
        ACCEL_Z_L = readByte(0x40);
        ACCEL_Z = (ACCEL_Z_H << 8) + ACCEL_Z_L;
        ACCEL_Z = ACCEL_Z/16384;
    }
}
