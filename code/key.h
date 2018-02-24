#ifndef __KEY_H__
#define __KEY_H__

/*
********************************************************************************
*	����ģ�������
********************************************************************************
*/
#define KEY_BUF_SIZE				10			// ���̻�������С��������Ҫ�ı䣬�����ֽ���
#define KEY_RPT_START_DLY		20			// ���������ظ�����ʾʱ�䣬��λ�ǰ���ɨ���ʱ����
#define KEY_RPT_DLY				10			// �����ظ���ʱ��������λ�ǰ���ɨ���ʱ����

/*
********************************************************************************
*	��������������
********************************************************************************
*/
extern bit Flg_KeyLock;					// ����������־��= 1ʱ�������а���ɨ��
extern bit Flg_KeyOnePress;				// ��û�а�������(Ӳ��)ʱ���ñ�־������
extern uint8 IR_RptCount;

#define KeyLock()				Flg_KeyLock = 1	// ��������
#define KeyUnLock()			Flg_KeyLock = 0	// ���̽���


#define 	ADC_CH0		0x00
#define 	ADC_CH1		0x01
#define 	ADC_CH2		0x02
#define 	ADC_CH3		0x03
#define 	ADC_CH4		0x04
#define 	ADC_CH5		0x05
#define 	ADC_CH6		0x06
#define 	ADC_CH7		0x07
#define 	ADC_CH8		0x08
#define 	ADC_CH9		0x09
#define 	ADC_CH10		0x0A
#define 	ADC_CH11		0x0B
#define 	ADC_CH12		0x0C
#define 	ADC_CH13		0x0D
#define 	ADC_CH14		0x0E
#define 	ADC_CH15		0x0F

#define VREF_FROM_VDD		0x00
#define VREF_FROM_AREF		0x01
#define VREF_FROM_BANDGAP	0x02

typedef enum
{
// �����Ǻ���ң�ض���
	IRCODE_POWER=0x00,			// ��Դ����





//�����Ƿ�ѹ��������
	KEYCODE_NOPRESS,  // 19
	KEYCODE_POWER,     //20
	KEYCODE_SCAN,
	KEYCODE_SEEKUP,     //21
	KEYCODE_SEEKDOWN,  // 22
	KEYCODE_TUNEUP, // 23
	KEYCODE_TUNEDOWN,
	KEYCODE_MUTE,
	KEYCODE_STER_MONO, // 23
	KEYCODE_VOLUP,  // 22
	KEYCODE_VOLDOWN,
	KEYCODE_TEST,
	KEYCODE_FM_AM

}INPUTCODE;


void   KeyInit(void);
uint8  KeyGetKey(void);
uint16 KeyGetKeyDownTime(void);
void   KeyDownTimeClear(void);
bit    KeyHit(void);
void   KeyScan(void);
void KeyBufIn(uint8 key);
void ADC_Init(void);
WORD API_AverageADCData(BYTE ADC_Channel);



#endif
