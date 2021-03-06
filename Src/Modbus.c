#include "Modbus.h"
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "ModbusTCP_IP.h"
#include "A24AA64.h"
#include "i2c.h"



uint16_t CRC16 (const uint8_t *nData, uint16_t wLength)
{
static const uint16_t wCRCTable[] = {
0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };
uint8_t nTemp;
uint16_t wCRCWord = 0xFFFF;
while (wLength--)
{
nTemp = *nData++ ^ wCRCWord;
wCRCWord >>= 8;
wCRCWord ^= wCRCTable[nTemp];
}
return wCRCWord;
}


void ModbusWrite(uint8_t* SendBuffor,uint8_t len){
	volatile uint16_t tempCRC;
	HAL_GPIO_WritePin(USART1_DE_GPIO_Port,USART1_DE_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(USART1_REn_GPIO_Port,USART1_REn_Pin,GPIO_PIN_SET);
	tempCRC=CRC16(SendBuffor,len);
	SendBuffor[len]=(tempCRC&0x00FF);
	SendBuffor[len+1]=(tempCRC>>0x08);

	HAL_UART_Transmit_DMA(&huart1,SendBuffor,len+2);
	HAL_Delay(2);
	//SS_RS_485_LED_R_OFF();
}

void Modbus_Main() {
	if(Modbus_Flaga_DMA_Rx==true){
		Modbus_Buffor_Tab[Modbus_index] = Modbus_Buffor_Temp;
		Modbus_index++;
		if(Modbus_Buffor_Tab[0]!=Modbus_Address_Device) {Modbus_index=0;Modbus_Flaga_DMA_Rx=false;return ;} //Czyszczenie bufora

		//if(Modbus_Buffor_Tab[1]!=Modbus_Function_Read && Modbus_index >2 ) {Modbus_index=0;Modbus_Flaga_DMA_Rx=false;SS_RS_485_LED_G_OFF();return ;}
		//if(Modbus_Buffor_Tab[1]!=Modbus_Function_Write&& Modbus_index >2 ) {Modbus_index=0;Modbus_Flaga_DMA_Rx=false;SS_RS_485_LED_G_OFF();return ;}

		if (Modbus_Buffor_Tab[1] == Modbus_Function_Write && Modbus_index >= 11) { //Ramka Write
			uint16_t DataAdd = (Modbus_Buffor_Tab[2] << 8)	| Modbus_Buffor_Tab[3]; //The Data Address of the first register
			uint16_t NrReg2W = (Modbus_Buffor_Tab[4] << 8)	| Modbus_Buffor_Tab[5]; //The number of registers to write
			uint8_t n = 9 + 2*NrReg2W - 2;
			volatile uint16_t tCRC16 = CRC16(Modbus_Buffor_Tab, n);
			if (((tCRC16 & 0xFF) == Modbus_Buffor_Tab[n])	&& ((tCRC16 >> 8) == Modbus_Buffor_Tab[n + 1])) {
				uint8_t BuforACK[8];
				memcpy(BuforACK,Modbus_Buffor_Tab,6);

				ModbusWrite(BuforACK, 6);
				uint8_t i=0;
				for(i=0;i<NrReg2W;i++){
					//Modbus_Register1[Modbus_Data_OFFSET + DataAdd +i]=(Modbus_Buffor_Tab[7+2*i]<<8)|Modbus_Buffor_Tab[8+2*i];
					ModbusTCP.Register[Modbus_Data_OFFSET + DataAdd + i]=(Modbus_Buffor_Tab[7+2*i]<<8)|Modbus_Buffor_Tab[8+2*i];
				}

				uint16_t war1=A24AA64.IP[0] << 8| A24AA64.IP[1];
				uint16_t war2=A24AA64.IP[2] << 8| A24AA64.IP[3];
				uint16_t war3;

				if (((A24AA64.IP[0] << 8	| A24AA64.IP[1]) != ModbusTCP.Register[Modbus_Offset_IP])|| ((A24AA64.IP[2] << 8| A24AA64.IP[3])!= ModbusTCP.Register[Modbus_Offset_IP + 1]))
				{
					uint8_t Buffor[4];
					Buffor[0]=ModbusTCP.Register[20]>>8;Buffor[1]=ModbusTCP.Register[20]&0xFF;
					Buffor[2]=ModbusTCP.Register[21]>>8;Buffor[3]=ModbusTCP.Register[21]&0xFF;
					HAL_I2C_Mem_Write(&hi2c1,A24AA64_Address,Modbus_Offset_IP,2,Buffor,4,100);
					HAL_Delay(100);
					HAL_NVIC_SystemReset();
				}
				war1=A24AA64.SubMask[0] << 8| A24AA64.SubMask[1];
				war2=A24AA64.SubMask[2] << 8| A24AA64.SubMask[3];

				if ((war1 != ModbusTCP.Register[Modbus_Offset_SubMask])|| (war2 != ModbusTCP.Register[Modbus_Offset_SubMask + 1]))
				{
					uint8_t Buffor[4];
					Buffor[0]=ModbusTCP.Register[30]>>8;Buffor[1]=ModbusTCP.Register[30]&0xFF;
					Buffor[2]=ModbusTCP.Register[31]>>8;Buffor[3]=ModbusTCP.Register[31]&0xFF;
					HAL_I2C_Mem_Write(&hi2c1,A24AA64_Address,35,2,Buffor,4,100);
					HAL_Delay(100);
					HAL_NVIC_SystemReset();
				}
				war1=A24AA64.Gateway[0] << 8| A24AA64.Gateway[1];
				war2=A24AA64.Gateway[2] << 8| A24AA64.Gateway[3];

				if ((war1 != ModbusTCP.Register[Modbus_Offset_Gateway])|| (war2 != ModbusTCP.Register[Modbus_Offset_Gateway + 1]))
				{
					uint8_t Buffor[4];
					Buffor[0]=ModbusTCP.Register[40]>>8;Buffor[1]=ModbusTCP.Register[40]&0xFF;
					Buffor[2]=ModbusTCP.Register[41]>>8;Buffor[3]=ModbusTCP.Register[41]&0xFF;
					HAL_I2C_Mem_Write(&hi2c1,A24AA64_Address,40,2,Buffor,4,100);
					HAL_Delay(100);
					HAL_NVIC_SystemReset();
				}
				war1=A24AA64.MAC[0] << 8| A24AA64.MAC[1];
				war2=A24AA64.MAC[2] << 8| A24AA64.MAC[3];
				war3=A24AA64.MAC[4] << 8| A24AA64.MAC[5];

				if ((war1 != ModbusTCP.Register[Modbus_Offset_Mac])|| (war2 != ModbusTCP.Register[Modbus_Offset_Mac + 1])|| (war3 != ModbusTCP.Register[Modbus_Offset_Mac + 2]))
				{
					uint8_t Buffor[6];
					Buffor[0]=ModbusTCP.Register[50]>>8;Buffor[1]=ModbusTCP.Register[50]&0xFF;
					Buffor[2]=ModbusTCP.Register[51]>>8;Buffor[3]=ModbusTCP.Register[51]&0xFF;
					Buffor[4]=ModbusTCP.Register[52]>>8;Buffor[5]=ModbusTCP.Register[52]&0xFF;
					HAL_I2C_Mem_Write(&hi2c1,A24AA64_Address,50,2,Buffor,6,100);
					HAL_Delay(100);
					HAL_NVIC_SystemReset();
				}
				war1=A24AA64.DNS[0] << 8| A24AA64.DNS[1];
				war2=A24AA64.DNS[2] << 8| A24AA64.DNS[3];

				if ((war1 != ModbusTCP.Register[Modbus_Offset_DNS])|| (war2 != ModbusTCP.Register[Modbus_Offset_DNS + 1]))
				{
					uint8_t Buffor[4];
					Buffor[0]=ModbusTCP.Register[60]>>8;Buffor[1]=ModbusTCP.Register[60]&0xFF;
					Buffor[2]=ModbusTCP.Register[61]>>8;Buffor[3]=ModbusTCP.Register[61]&0xFF;
					HAL_I2C_Mem_Write(&hi2c1,A24AA64_Address,60,2,Buffor,4,100);
					HAL_Delay(100);
					HAL_NVIC_SystemReset();
				}
				Modbus_index = 0;
			}
		}

		if (Modbus_index == 8 && Modbus_Buffor_Tab[1]==Modbus_Function_Read) //Ramka Read
		{
			volatile uint16_t tCRC16 = CRC16(Modbus_Buffor_Tab, 6);
			if (((tCRC16 & 0xFF) == Modbus_Buffor_Tab[6]) && ((tCRC16 >> 8) == Modbus_Buffor_Tab[7])) {
				uint8_t BuforACK[60];
				uint16_t DataAdd = (Modbus_Buffor_Tab[2] << 8)	| Modbus_Buffor_Tab[3]; //The Data Address of the first register
				uint16_t NrReg2R = (Modbus_Buffor_Tab[4] << 8)	| Modbus_Buffor_Tab[5]; //The total number of registers requested
				uint8_t n = 3+2*NrReg2R;


				memcpy(BuforACK,Modbus_Buffor_Tab,2);
				BuforACK[2] = 2*NrReg2R;

				for(uint8_t i=0;i<NrReg2R;i++){
						BuforACK[3+2*i] = ModbusTCP.Register[Modbus_Data_OFFSET + DataAdd + i]>>8;
						BuforACK[4+2*i] = ModbusTCP.Register[Modbus_Data_OFFSET + DataAdd + i]&0xFF;
				}
				ModbusWrite(BuforACK, n);
			}
			Modbus_index = 0;
		}
		Modbus_Flaga_DMA_Rx=false;
	}
}
