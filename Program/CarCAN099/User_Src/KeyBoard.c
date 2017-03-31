#include "KeyBoard.h" 

//��Ҫ����
//#define BOARD_099B 1


//��һλ���ƵĿ�����. Todo: д�ø�����ͨ���ԡ�{ID_ESP,ID_AVM,ID_PDC,ID_HUD,ID_NVS}
#define SP_BIT_EMS_STARTSTOP 50

#ifdef BOARD_099
KeyIO_t indKeyIO[IND_KEY_NUM] = {
											{GPIOA,GPIO_Pin_4,18,0,0,0,0},		//KEY1 AVM_Skey ; 
											{GPIOA,GPIO_Pin_5,19, GPIOB,GPIO_Pin_11,6,ID_AVM},	//KEY2 LDWS Skey��LED: LDWS Skey LED
											{GPIOA,GPIO_Pin_6,17, 0,0,0,0},		//KEY3 NVS Skey; 
											{GPIOA,GPIO_Pin_7,15, GPIOB,GPIO_Pin_10,2,ID_ESP},	//KEY4 ESC Off  LED: ESC LED
											{GPIOB,GPIO_Pin_0,16,	GPIOB,GPIO_Pin_2,15,ID_ESP},	//KEY5 HDC 			LED: HDC LED ??????
											{GPIOB,GPIO_Pin_1,26,0,0},		//KEY6 DVD NAVI	
											{GPIOC,GPIO_Pin_15,6,GPIOB,GPIO_Pin_3,30,ID_PDC},		//KEY7 Front Radar: 
											{GPIOC,GPIO_Pin_14,7,GPIOB,GPIO_Pin_5,21,ID_PDC},		//KEY8 BSA Switch			LED: BSA LED
											{GPIOC,GPIO_Pin_13,20,GPIOB,GPIO_Pin_7,1,ID_HUD},		//KEY9 HUD Skey: 		LED: HUD SKey LED
											{GPIOB,GPIO_Pin_6,21,0,0},		//KEY10 HUD Up
											{GPIOB,GPIO_Pin_4,22,0,0},		//KEY11 HUD Down
											{GPIOA,GPIO_Pin_15,SP_KEY_STARTSTOP,GPIOB,GPIO_Pin_9,SP_BIT_EMS_STARTSTOP,ID_EMS},		//KEY12 StartStopSwith (A:NC)	LED: StartStop LED (A:NC)
											};
#endif

#ifdef BOARD_099A
KeyIO_t indKeyIO[IND_KEY_NUM] = {
											{GPIOA,GPIO_Pin_4,18,0,0,0,0},		//KEY1 AVM_Skey ; 
											{GPIOA,GPIO_Pin_5,19, GPIOB,GPIO_Pin_11,6,ID_AVM},	//KEY2 LDWS Skey��LED: LDWS Skey LED
											{GPIOA,GPIO_Pin_6,17, 0,0,0,0},		//KEY3 NVS Skey; 
											{GPIOA,GPIO_Pin_7,15, GPIOB,GPIO_Pin_10,2,ID_ESP},	//KEY4 ESC Off  LED: ESC LED
											{GPIOB,GPIO_Pin_0,16,	GPIOB,GPIO_Pin_2,15,ID_ESP},	//KEY5 HDC 			LED: HDC LED ??????
											{GPIOB,GPIO_Pin_1,26,0,0},		//KEY6 DVD NAVI	
											{GPIOC,GPIO_Pin_15,6,GPIOB,GPIO_Pin_3,30,ID_PDC},		//KEY7 Front Radar: 
											{GPIOC,GPIO_Pin_14,7,GPIOB,GPIO_Pin_5,21,ID_PDC},		//KEY8 BSA Switch			LED: BSA LED
											{GPIOC,GPIO_Pin_13,20,GPIOB,GPIO_Pin_7,1,ID_HUD},		//KEY9 HUD Skey: 		LED: HUD SKey LED
											{GPIOB,GPIO_Pin_6,21,0,0},		//KEY10 HUD Up
											{GPIOB,GPIO_Pin_4,22,0,0},		//KEY11 HUD Down
											//{GPIOB,GPIO_Pin_6,9,0,0},		//KEY12 StartStopSwith (A:NC)	LED: StartStop LED (A:NC)
											};
#endif

//��ֵ����
volatile u16 keysBuff; 
//����״̬���棬����ֵ�Ӱ��±���ɿ�ʱ������״̬��ת��
volatile u16 swBuff;
volatile char dirBuff;
u16 encNum=0;
											
static void ADCInit(void);

//
void KeyAndLEDInit(void)
{
	//��������  PA8 -Deactivate PB14 - Enable
	GPIO_InitTypeDef GPIO_InitStructure;
	u8 ki; 
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
	
	for(ki=0;ki < IND_KEY_NUM;ki++)
	{
		//Key
		GPIO_InitStructure.GPIO_Pin = indKeyIO[ki].GPIO_Pin;	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;       //�������������򸡿ա�GPIO_��                                    Mode_IN_FLOATING
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(indKeyIO[ki].GPIO_Port, &GPIO_InitStructure);
		
		//LED
		if(indKeyIO[ki].LED_Port!=0)
		{
			GPIO_InitStructure.GPIO_Pin = indKeyIO[ki].LED_Pin;	
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       //Tobe confirmed.
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(indKeyIO[ki].LED_Port, &GPIO_InitStructure);
		}
	}
}

//ÿ10ms����һ�Σ���ü�ʱ��ֵ�����ÿ���������߼�����״̬����һ�ν��з�ת
//���ش����ź�
u8 KeyScan(void)
{
	//static u8 indKeyStatePre[IND_KEY_NUM]={2};
	static u8 ks[IND_KEY_NUM]={0};
	u8 trig=0;
//	static u8 indKeyStatePrePre[4]={2,2,2,2};
  u8 i;
/*
	//��ȡ��ǰ��״̬
  for(i=0;i<IND_KEY_NUM;i++)
	{
		u8 tempVal=0;
		tempVal = GPIO_ReadInputDataBit(indKeyIO[i].GPIO_Port,indKeyIO[i].GPIO_Pin);
		if(indKeyStatePre[i] == tempVal )  //���ζ�ֵһ���������˵���е����
		{
			if(tempVal == 0)	//Press down
			  keysBuff |= ((u16)1)<<i;
			else							//Release 
				keysBuff &= ~(((u16)1)<<i);
		}
		indKeyStatePre[i] = tempVal;
//		indKeyStatePrePre[i] = indKeyStatePre[i];
	}
*/
	for(i=0;i<IND_KEY_NUM;i++)
	{
		u8 tempVal=0;
		tempVal = GPIO_ReadInputDataBit(indKeyIO[i].GPIO_Port,indKeyIO[i].GPIO_Pin);
		switch(ks[i])
		{
			case 0: //idle up
				if(tempVal==0)
				{
					#ifdef SP_KEY_STARTSTOP
					if(indKeyIO[i].bitn!=SP_KEY_STARTSTOP)
					{
						ks[i]=1;
						keysBuff |= ((u16)1)<<i;
					}
					else
					{	//���������������·�ת
						if((keysBuff & (((u16)1)<<i)) ==0)
							keysBuff |= ((u16)1)<<i;
						else
							keysBuff &=~(((u16)1)<<i);
						
						ks[i]=1;
					}
					#else
					ks[i]=1;
					keysBuff |= ((u16)1)<<i;
					#endif
				}
				break;
			case 1:	//down
				if(tempVal==1)
				{
					#ifdef SP_KEY_STARTSTOP
					if(indKeyIO[i].bitn!=SP_KEY_STARTSTOP)
					{
						ks[i] = 0;
						keysBuff &= ~(((u16)1)<<i);
					}
					else
					{	//���ⰴ���������������ɿ����� ��ס״̬
						ks[i]=0;
					}
					#else
					ks[i] = 0;
					keysBuff &= ~(((u16)1)<<i);
					#endif
					//key release��revert sw state
					if((swBuff & (((u16)1)<<i)) ==0)
						swBuff |= ((u16)1)<<i;
					else
						swBuff &=~(((u16)1)<<i);
					trig=1;
				}
				break;
			//case 2:	//release
			//	break;
		}
	}
	/*
	//Start stop key����������
	#ifdef SP_KEY_STARTSTOP
		if(swBuff & ((u16)1 << 9))
			keysBuff |= (((u16)1)<<9);
		else
			keysBuff &=~(((u16)1)<<9);
	#endif
	*/
	
	//EncoderRead();
	
	return trig;
}
//���ݿ���״̬������LED����
void LEDUpdate(u8 *datRec, u32 id)
{
	u8 i;
	
	
	for(i=0;i<IND_KEY_NUM;i++)
	{
		if(indKeyIO[i].rID == id)
		{
			if(indKeyIO[i].LED_Port !=0 )
			{
				u8 Bn, bn;
				Bn = indKeyIO[i].rbitn / 8;	//�ڵڼ����ֽ�
				bn = indKeyIO[i].rbitn % 8;	//�ڼ�λ
				
				if((datRec[Bn] & (((u8)1)<<bn)) ==0)	//����off
					GPIO_ResetBits(indKeyIO[i].LED_Port, indKeyIO[i].LED_Pin);
				else
					GPIO_SetBits(indKeyIO[i].LED_Port, indKeyIO[i].LED_Pin);
				//��һλ���ƵĿ������Ĵ���
				if(indKeyIO[i].rbitn == SP_BIT_EMS_STARTSTOP)
				{
					if((datRec[Bn] & (((u8)0x0f)<<bn)) ==0)	//����off
						GPIO_ResetBits(indKeyIO[i].LED_Port, indKeyIO[i].LED_Pin);
					else
						GPIO_SetBits(indKeyIO[i].LED_Port, indKeyIO[i].LED_Pin);
				}
			}
		}
	}
}




void EncoderInit(void)
{
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		TIM_ICInitTypeDef TIM_ICInitStructure;

		/* Encoder unit connected to TIM3, 4X mode */    
		GPIO_InitTypeDef GPIO_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;

		/* TIM3 clock source enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		/* Enable GPIOA, clock */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

		GPIO_StructInit(&GPIO_InitStructure);
		/* Configure PA.06,07 as encoder input */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		/* Enable the TIM3 Update Interrupt 
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);*/

		/* Timer configuration in Encoder mode */
		TIM_DeInit(TIM3);
		TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

		TIM_TimeBaseStructure.TIM_Prescaler = 0x0; // No prescaling 
		TIM_TimeBaseStructure.TIM_Period = 0xffff;	//TOBE Confirmed 
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

		TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, 
																 TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
		TIM_ICStructInit(&TIM_ICInitStructure);
		TIM_ICInitStructure.TIM_ICFilter = 6;
		TIM_ICInit(TIM3, &TIM_ICInitStructure);

		/*// Clear all pending interrupts
		TIM_ClearFlag(TIM3, TIM_IT_CC1);TIM_ClearFlag(TIM3, TIM_IT_CC2);
		TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);	//��ʱ�����ж� TIM_IT_Update
		TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);	
		*/
		//Reset counter
		TIM3->CNT = 0;

		// ENC_Clear_Speed_Buffer();

		TIM_Cmd(TIM3, ENABLE); 
}


//�ж���ִ�У�������ѯ�����ǰ�ť���ж���ִ�нϺ��ʡ�
char EncoderRead(void)
{
	static u8 isSetup=0;
	static u16 encNumPre=0;
	char dir=0;
		
	if(!isSetup)
	{
		encNum = TIM_GetCounter(TIM3);
		isSetup=1;
		return 0;
	}
	
	encNumPre = encNum;
	encNum = TIM_GetCounter(TIM3);
	
	if((encNum>encNumPre )|| (encNumPre - encNum > 0xffff/2))	//decrease
	{
		dir=2;
	}
	else if((encNum < encNumPre )|| (encNum - encNumPre > 0xffff/2))
	{
		dir=1;
	}
	else
		dir=0;
	
	return dir;
		
		
	
	
}