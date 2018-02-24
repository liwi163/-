#include "gheader.h"

u8 ResetStatus;

bdata bit m_b10ms;
bdata bit m_b100ms;
bdata bit m_b500ms;
bdata bit m_b1000ms;

#ifdef UART0
u8 RS_buf[RS_BUF_MAX];
data u8 RS_in=0,RS_out=0;
bit RS_Xbusy=0;
#endif

/*
*************************************************************************
���		liwei
�ļ�		main
����		���������
*************************************************************************
*/
void InitMcuReg(void)
{

	rRC_LADJ = rRC_CALIBRATION_DATA;
	ResetStatus = rRESET_FLG;
	rRESET_FLG |= 0x80;
	
	rRC_CALIB_EN = 0x80;	
	rSOURCE_CLK_SLT = 0xA0;		      //�ڲ�12M����
	rSYS_CTL &=~0x80;				//�����˲�����
	rWDT_CTL |=0xA0;				//�رտ��Ź�
	
	rSYS_CTL &=~0x80;				//Enable reset have digital filter
	rLVD_CTL = 0xA0;		
	EA = 1;							//ȫ���ж�

//	rSOURCE_CLK_SLT|=0x03;               // 1T

}



/*
*************************************************************************
���		liwei
�ļ�		main
����		���������
*************************************************************************
*/
void InitMcuPort(void)
{
	GPIOA1_PIN_OE=0;
	GPIOA2_PIN_OE=0;
	GPIOA3_PIN_OE=0;

	GPIOA1_PIN_PHN=0; /*����*/
	GPIOA2_PIN_PHN=0; /*����*/
	GPIOA3_PIN_PHN=0; /*����*/
	
	//GPIOB1_PIN=0;
	//GPIOB2_PIN=0;
	//GPIOB3_PIN=0;

	GPIOA4_PIN_OE=1;
	GPIOA5_PIN_OE=1;

	GPIOA5_PIN=0;
	GPIOA4_PIN=0;

}




/*
*************************************************************************
���		liwei
�ļ�		InitSysTimer
����		ϵͳʱ�ӳ�ʼ��ʹ�ö�ʱ��0
*************************************************************************
*/

void InitSysTimer(void)
{
	TMOD|=0x01;						// ��ʱ��0ʹ��16λ��ʱ��
	TL0= SYSTMR_DATAL;					// ���붨ʱ����ʼֵ
	TH0= SYSTMR_DATAH;
	ET0= 1;							// ����ʱ�ж�1
	TR0= 1;								// ������ʱ��
}


/*
*************************************************************************
���		liwei
�ļ�		Timer0INT
����		��ʱ��0�ж�ִ�к���
*************************************************************************
*/

void Timer0INT(void) interrupt 1
{
	TR0=0;
	TL0= SYSTMR_DATAL;					// ���붨ʱ����ʼֵ
	TH0= SYSTMR_DATAH;
	TR0=1;
	tic_01++;
	tic_pc++;

	if((tic_01%10)==0)
		{
			m_b10ms=1;
		}
	
	if((tic_01%100)==0){
			m_b100ms=1;
		}	

	if((tic_01%500)==0){
			m_b500ms=1;
		}		
	if((tic_01%1000)==0){
			m_b1000ms=1;
		}
	
}

bit Flg_KeyScanDly = 1;

void SysTimer(void)
{
	if(m_b10ms)
		{
			m_b10ms=0;
			Flg_KeyScanDly = ~Flg_KeyScanDly;
			if (Flg_KeyScanDly) KeyScan();
		}

	if(m_b1000ms)
		{
			m_b1000ms=0;
			ShowQCicon();
			EepSavePar();
		}
	
	if(m_b500ms)
		{
			m_b500ms=0;
			DisplayAll();
			led0=~led0;
			//ReadADC(ADC_CH0);
			//ReadADC(ADC_CH2);
			//ReadADC(ADC_CH3);
		}
	
}



#ifdef UART0

/*
*************************************************************************
���		liwei
�ļ�		InitUartPort
����		�����жϺ���
			����ʹ��UART 0��Ϊ���ڵ��Զ˿�
			ʹ��һ��ר�õĶ�ʱ������������
			������
			����0ʹ�õ�IO��λ: TX0 B ·�� 22pin
										   RX0 B ·�� 21pin
				
*************************************************************************
*/

void SelectUart0Baudrate(BYTE BitRateIndex)
{
	switch(BitRateIndex)
	{
		case BIT_RATE600:
			rSBRG0H = 0xCE;
			rSBRG0L = 0x20;
			break;
		case BIT_RATE1200:
			rSBRG0H = 0xA7;
			rSBRG0L = 0x10;
			break;
		case BIT_RATE2400:
			rSBRG0H = 0x93;
			rSBRG0L = 0x88;
			break;
		case BIT_RATE4800:
			rSBRG0H = 0x89;
			rSBRG0L = 0xC4;
			break;
		case BIT_RATE9600:
			rSBRG0H = 0x84;
			rSBRG0L = 0xE2;
			break;
		case BIT_RATE14400:
			rSBRG0H = 0x83;
			rSBRG0L = 0x40;
			break;
		case BIT_RATE19200:
			rSBRG0H = 0x82;
			rSBRG0L = 0x71;
			break;
		case BIT_RATE38400:
			rSBRG0H = 0x81;
			rSBRG0L = 0x38;		
			break;
		case BIT_RATE57600:
			rSBRG0H = 0x80;
			rSBRG0L = 0xD0;			
			break;
		case BIT_RATE115200:
			rSBRG0H = 0x80;
			rSBRG0L = 0x68;	
			break;
		case BIT_RATE230400:
			rSBRG0H = 0x80;
			rSBRG0L = 0x34;				
			break;
		default:
			rSBRG0H = 0x80;
			rSBRG0L = 0x68;
			break;
	}	
}


/*
*************************************************************************
���		liwei
�ļ�		InitUartPort
����		�����жϺ���
			����ʹ��UART 0��Ϊ���ڵ��Զ˿�
			����0ʹ�ö�ʱ��1��Ϊ�ɱ�Ƶ�ʷ�����
			����0ʹ�õ�IO��λ: TX0 B ·�� 22pin
									     RX0 B ·�� 21pin
				
*************************************************************************
*/

void InitUartPort(void)
{
	GPIOB7_PIN_OE=0;  // rx ����
	GPIOC0_PIN_OE=1;  // tx ���
	rGPIOB_FUN1|=0x40; // b7����ΪRX ����λ����ΪRXBʱc0���Զ�����ΪTXB

	//SelectUart0Baudrate(BIT_RATE9600);
	SelectUart0Baudrate(BIT_RATE115200);

	
	SCON0=0x50;  // ģʽ1
	
	ES = 1;				/*�򿪴����ж�*/ 

}

/*
*******************************************************************************
���		liwei
�ļ�		uart0_int
����		�����жϴ���


*******************************************************************************

*/

void uart0_int(void) interrupt 4 using 1
{
	if( RI ) {					//--- Receive interrupt ----
		RI = 0;
		RS_buf[RS_in++] = SBUF0;
		if( RS_in>=RS_BUF_MAX ) RS_in = 0;
	}

	if( TI ) {					//--- Transmit interrupt ----
		TI = 0;
		RS_Xbusy=0;
	}
}

/*
*******************************************************************************
���		liwei
�ļ�		RS_ready
����		�����жϴ���


*******************************************************************************
*/
BYTE RS_ready(void)
{
	if( RS_in == RS_out ) return 0;
	else return 1;
}

/*
*******************************************************************************
���		liwei
�ļ�		RS_rx
����		���պ���


*******************************************************************************
*/

BYTE RS_rx(void)
{
	BYTE	ret;
	ES = 0;
	ret = RS_buf[RS_out];
	RS_out++;
	if(RS_out >= RS_BUF_MAX) 
		RS_out = 0;
	ES = 1;

	return ret;
}



/*
*******************************************************************************
���		liwei
�ļ�		RS_rx
����		���ͺ���


*******************************************************************************

*/
void RS_tx(BYTE tx_buf)
{
	while(RS_Xbusy);

	ES = 0;			// To protect interrupt between SBUF and RS_Xbusy when it's slow
	SBUF0 = tx_buf;
	RS_Xbusy=1;
	ES = 1;
}


#endif
