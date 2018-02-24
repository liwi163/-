#include "gheader.h"


/*
********************************************************************************
*		�������״̬����״̬
********************************************************************************
*/
#define KEY_STATE_UP					1      /* Key scanning states used in KeyScan()                   */
#define KEY_STATE_DEBOUNCE				2
#define KEY_STATE_RPT_START_DLY		3
#define KEY_STATE_RPT_DLY				4

/*
********************************************************************************
*		����ģ����ʹ�õľ�̬����
********************************************************************************
*/
static uint8  	KeyBuf[KEY_BUF_SIZE];
static uint8 	KeyBufInLx;
static uint8 	KeyBufOutLx;
static uint8 	KeyScanState;
static uint8 	KeyRptStartDlyCtr;
static uint8 	KeyRptDlyCtr;
static uint16   KeyDownTmr;


/*
********************************************************************************
*		����ģ���ȫ�ֱ�������
********************************************************************************
*/
bit Flg_KeyLock=0;
bit Flg_KeyOnePress = 0;
uint8 IR_RptCount;

/*
********************************************************************************
*		����ģ����ʹ�õľ�̬���ܺ�������
********************************************************************************
*/
 void KeyBufIn(u8 key);
static u8 KeyGetKeyCode(void);
static u8 KeyIsDown(void);


/*
********************************************************************************  
* 	�� �� ��: KeyBufIn
* 	��������: ����������뵽���̻�������
* 	��    ��: key	:�����Ĵ���
* 	��    ��: ��
********************************************************************************
*/
 void KeyBufIn(u8 key)
{
	u8 tmp;
	tmp = KeyBufInLx + 1;
	if (tmp == KEY_BUF_SIZE) tmp = 0;
	if (tmp == KeyBufOutLx)  return;		//���βָ���1�Ժ����ͷָ�룬���ʾ�����������
	KeyBufInLx = tmp;
	KeyBuf[KeyBufInLx] = key;
}

/*
********************************************************************************  
* 	�� �� ��: KeyIsDown
* 	��������: �ж���û�а�������ȥ
* 	��    ��: ��
* 	��    ��: 0		: û�а�������
*             1		: �а�������
********************************************************************************
*/
static u8 KeyIsDown(void)
{
	if (KeyGetKeyCode() == KEYCODE_NOPRESS)  {
		Flg_KeyOnePress = 0;
		return 0;
		}
	
	else  {
		if (KeyDownTmr < 0xFFFF)  KeyDownTmr++;
		return 1;
		}
}

/*
*************************************************************************
���		liwei
�ļ�		ADC_Init
����		���������
*************************************************************************
*/

void ADC_Init(void)
{


	GPIOB6_PIN_OE=1;
	GPIOB6_PIN_PHN=1;
	rGPIOB_FUN1|=0x30;  // B6����ΪADC ���



	GPIOB5_PIN_OE=1;
	GPIOB5_PIN_PHN=1;
	rGPIOB_FUN1|=0x0c;  // B5����ΪADC ���



	GPIOB4_PIN_OE=1;
	GPIOB4_PIN_PHN=1;
	rGPIOB_FUN1|=0x03;  // B4����ΪADC ���


	GPIOB3_PIN_OE=1;
	GPIOB3_PIN_PHN=1;
	rGPIOB_FUN2|=0xc0;  // B3����ΪADC ���


	GPIOB2_PIN_OE=1;
	GPIOB2_PIN_PHN=1;
	rGPIOB_FUN2|=0x30;  // B2����ΪADC ���

	GPIOB1_PIN_OE=1;
	GPIOB1_PIN_PHN=1;
	rGPIOB_FUN2|=0x0c;  // B1����ΪVREF ADC �ο���ѹ


	GPIOB0_PIN_OE=1;
	GPIOB0_PIN_PHN=1;
	rGPIOB_FUN2|=0x03;  // B0����ΪADC ���

	GPIOA7_PIN_OE=1;
	GPIOA7_PIN_PHN=1;
	rGPIOA_FUN1|=0xc0;  //A7����ΪADC ���

	
	rADC_CTL = 0x80;      // ����ת��ģʽ
	//ADC converter speed 125KHz
	rADC_SEL = 0x80 +VREF_FROM_VDD;
	//Select AD Channel
	//rADC_ENCH = ADC_CH1;		//Select ADC channe	
	
	//Enable ADC power
	//rADC_CTL &= 0x7F;			//Enable low speed adc(bit7=0) 	

	Delayms(1);


	//Enable Continuous mode
	//rADC_CTL |= 0x20;			
}


WORD DRV_ReadAnalogChannel(BYTE AD_Channel)
{
	WORD AdcWdata;
	BYTE AdcBdata; 
	
	//rADC_SEL = 0x80;
	rADC_SEL = 0x80 + 0;//VREF_FROM_AREF;//VREF_FROM_VDD;


	
	rADC_ENCH = AD_Channel;		//Select ADC channe	
	//rADC_TMPS &= 0xFE;			//Disable EN_ADC_TMPS
	//select VerF
	//rADC_VREF = VREF_FROM_VDD;
	rADC_CTL &= 0x7F;			//Enable low speed adc(bit7=0) 	
	_nop_();
	rADC_CTL |= 0x40;			//=1:Start ADC converter
	do
	{
	_nop_();
	}
	while(rADC_CTL & 0x40);	//1=>0:convert finish
	AdcWdata = 0;
	#if 0
	//For ADC Tool
	PMCTL_DATA[7] = AD_Channel;
	PMCTL_DATA[8] = rADC_DATAH;
	PMCTL_DATA[9] = rADC_DATAL & 0x0F;
	#endif
	AdcBdata = rADC_DATAL;
	AdcBdata = AdcBdata & 0x0F;
	AdcWdata = rADC_DATAH;
	AdcWdata = AdcWdata << 4;
	AdcWdata = AdcWdata + AdcBdata;


	//DRV_Printf("Show rADC_DATAH = %d \r\n",rADC_DATAH);
	//DRV_Printf("Show rADC_DATAL = %d \r\n",rADC_DATAL);	
	return AdcWdata;		
}


WORD API_AverageADCData(BYTE ADC_Channel)
{
#define ADC_SUM_TIME		16	
#define	ADC_SORTING_SIZE	0x03	
	
BYTE R7;

WORD ADC_TotalData;

	ADC_TotalData = 0;
	for(R7=0;R7<ADC_SUM_TIME;R7++)
	{		
		//Delay(10);
		ADC_TotalData = ADC_TotalData + DRV_ReadAnalogChannel(ADC_Channel);	
	}
	ADC_TotalData = ADC_TotalData / ADC_SUM_TIME;	
	return ADC_TotalData;

}










/*
********************************************************************************  
* 	�� �� ��		: KeyGetKeyCode
* 	��������	: ͨ��ADCֵȷ�������Ǹ����ܰ���
* 	��    ��		: ��
* 	��    ��		: ������
*            				
********************************************************************************
*/
//#define	EN_DEBUG_MSG

u8 KeyGetKeyCode(void)
{

		if(GPIOA1_PIN==0)
			{
				return 	KEYCODE_VOLDOWN;
			}
		else if(GPIOA2_PIN==0)
			{
				return 	KEYCODE_VOLUP;
			}
		else if(GPIOA3_PIN==0)
			{
				return 	KEYCODE_MUTE;
			}
		else
			{
				return KEYCODE_NOPRESS;
			}
			


#if 0	
       u8 i=0,keytemp;
	uint16 KeyIn0;
//	KeyIn2 = GetADCValue(ADC_CH2);
	KeyIn0= GetADCValue(ADC_CH0);
	#if defined(EN_DEBUG_MSG)
	
	if(KeyIn0>0)
	Printf("\r\nKeyIn0 ADC Value is: %02bx", (u8)(KeyIn0>>4) );

	#endif
	
	if ((KeyIn0<10)) return    KEYCODE_NOPRESS;


	keytemp=(u8)(KeyIn0>>4);
	for (i = 0; i < sizeof(KeycodeArry0); i++)  {
	if ((keytemp >= KeycodeArry0[i][0]) && (keytemp <= KeycodeArry0[i][1]))
	break;
	}	
	if (i >= 6) return	 KEYCODE_NOPRESS;
	else return  KeycodeArry0[i][2];

#endif
		

	}

/*
********************************************************************************  
* 	�� �� ��: KeyGetKey
* 	��������: �Ӽ��̻������ж��������룬����������ǿ��򷵻�0xff
* 	��    ��: ��
* 	��    ��: �����Ĵ���
********************************************************************************
*/
uint8 KeyGetKey(void)
{
	if (KeyBufOutLx == KeyBufInLx )  return 0xFF;
	KeyBufOutLx++;
	if (KeyBufOutLx == KEY_BUF_SIZE)  KeyBufOutLx = 0;
	return KeyBuf[KeyBufOutLx];
}




/*
********************************************************************************  
* 	�� �� ��: KeyInit
* 	��������: ����ģ���ʼ����Ҫ��ȷʹ�ü���ģ��Ĺ��ܣ�Ҫ���ȵ��ó�ʼ��
* 	��    ��: ��
* 	��    ��: 0		: ���̻�������û�а���
*             1		: ���̻��������а���
********************************************************************************
*/
void KeyInit(void)
{
	uint8 i;
	
	KeyBufInLx 		= KEY_BUF_SIZE - 1;
	KeyBufOutLx 	= KEY_BUF_SIZE - 1;
	KeyScanState 	= KEY_STATE_UP;
	KeyDownTmr 	= 0;
	//IrAckCount	= 0;
	for (i=0; i< KEY_BUF_SIZE; i++) KeyBuf[i] = 0xFF;
	Flg_KeyLock		= 0;
}
/*
********************************************************************************  
* 	�� �� ��: KeyHit
* 	��������: Ӧ�ó�����øú����жϼ��̻��������ް���
* 	��    ��: ��
* 	��    ��: ��
********************************************************************************
*/
bit KeyHit(void)
{
	if (KeyBufOutLx == KeyBufInLx )  return 0;
	else  return 1;
}



/*
********************************************************************************  
* 	�� �� ��: KeyGetKeyDownTime
* 	��������: ���ذ�������ȥ��ʱ�䣬���ʱ����ɨ��ʱ�����ļ���ֵ
* 	��    ��: ��
* 	��    ��: ��������ȥ��ʱ��
********************************************************************************
*/
uint16 KeyGetKeyDownTime(void)
{
	return KeyDownTmr;
}

void KeyDownTimeClear(void)
{
	KeyDownTmr = 0;
}



/*
********************************************************************************  
* 	�� �� ��: KeyScan
* 	��������: ������������ģ��ĺ��ģ������������״̬���Ĵ���
* 	��    ��: ��
* 	��    ��: ��������ȥ��ʱ��
********************************************************************************
*/
void KeyScan(void)
{
	if (Flg_KeyLock)  return;						// �������������������κδ���
	
	switch (KeyScanState)  {
		case  KEY_STATE_UP:							// ����û�а��£�Ҫ�鿴��û�а���
			if (KeyIsDown())  {						// ����Ƿ��а�������
				IR_RptCount = 0;
				KeyScanState = KEY_STATE_DEBOUNCE;	// �а������£��ı䰴��ɨ���״̬
				//KeyDownTmr = 0;					// �������µ�ʱ������
				}
			KeyDownTmr = 0;
			break;

		case  KEY_STATE_DEBOUNCE:					// �����Ѱ��£���ȡɨ���룬���뻺����
			if (KeyIsDown())  {						// ����Ƿ��а�������
				KeyBufIn(KeyGetKeyCode());			// ��ȡɨ���룬�����뻺����
				KeyRptStartDlyCtr = KEY_RPT_START_DLY;	// �����Զ��ظ�����
				KeyScanState = KEY_STATE_RPT_START_DLY;	// ת������ɨ��״̬
				}

			else  KeyScanState = KEY_STATE_UP;		// û�м����£��ı䰴��ɨ��״̬
			break;

		case  KEY_STATE_RPT_START_DLY:				// �ȴ������Զ��ظ�
			if (KeyIsDown())  {					// ��鰴���Ƿ�һֱ����
				if (KeyRptStartDlyCtr)  {			// ����Ƿ���Ҫ��ʱ
					KeyRptStartDlyCtr--;
					if (KeyRptStartDlyCtr == 0)  {	// �����ʱʱ�䵽��û��
						KeyBufIn(KeyGetKeyCode());
						KeyRptDlyCtr = KEY_RPT_DLY;	// ������һ���ظ�����ʱ���
						KeyScanState = KEY_STATE_RPT_DLY;
						}
					}
				}
			
			else  KeyScanState = KEY_STATE_DEBOUNCE;	// ��û�а��£��ı�ɨ��״̬
			break;

		case  KEY_STATE_RPT_DLY:					// �ȴ���һ���ظ�
			if (KeyIsDown())  {						// ��鰴���Ƿ�һֱ����
				if (KeyRptDlyCtr)  {				// ����Ƿ���Ҫ��ʱ
					KeyRptDlyCtr--;
					if (KeyRptDlyCtr == 0)  {		// �����ʱʱ�䵽��û��
						KeyBufIn(KeyGetKeyCode());
						KeyRptDlyCtr = KEY_RPT_DLY;	// ������һ���ظ�����ʱ���
						}
					}
				}

			else  KeyScanState = KEY_STATE_DEBOUNCE;	// ��û�а��£��ı�ɨ��״̬
			break;
		}
}


