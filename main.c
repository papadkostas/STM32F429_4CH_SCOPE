/*******************************************************************************
  * @file    main.c
  * @author  Kostas Papadopoulos
  * @version V1.0.1
  * @date    17-October-2015
  * @brief   STM32F429 Discovey - ADC measurements using DMA
  ******************************************************************************
  * ADCx CH xx    Pin
  * ADC1 CH 10 -> PC0
  * ADC1 CH 11 -> PC1
  * ADC1 CH 12 -> PC2
  * ADC1 CH 13 -> PC3
  ******************************************************************************
  * PLL System Clock prescaler settings
  * PLL_M      8
  * PLL_N      288
  * PLL_P      2
  * HSE Clock input 8MHz
  * System Clock Frequency (MHz) = (HSE Clock/PLL_M)*PPL_N/PPL_P = 144MHz
  ******************************************************************************
  * ADC Maximum Clock frequency = 36MHz
  * Prescaler options 2/4/6/8
  * ADC Clock = PCLK2/prescaler = 72MHz/2 = 36MHz / 27.7nsec
  * Sample Time = Sample Cycles * ADC period
  * Convertion Time = 12 Cycles *
  * ADC Conversion Time = Sample Time + Conversion Time =
  *****************************************************************************
  */

#include "misc.h"
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_gpio.h"



DMA_InitTypeDef            DMA_InitStructure;
ADC_InitTypeDef            ADC_InitStructure;
ADC_CommonInitTypeDef      ADC_CommonInitStructure;
GPIO_InitTypeDef           GPIO_InitStructure;

volatile uint16_t ADC1Value[2] = {0};
volatile uint16_t ADC2Value = 0;
volatile uint16_t ADC3Value = 0;
volatile uint32_t counter0,counter1,counter2 = 0;


void ADC_Config(void){
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOF, &GPIO_InitStructure);

  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC1Value;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC1->DR));
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);

  DMA_InitStructure.DMA_Channel = DMA_Channel_1;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC2Value;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC2->DR));
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_Init(DMA2_Stream2, &DMA_InitStructure);

  DMA_InitStructure.DMA_Channel = DMA_Channel_2;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC3Value;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC3->DR));
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_Init(DMA2_Stream1, &DMA_InitStructure);

  /* DMA2_Stream0 enable */
  DMA_Cmd(DMA2_Stream0, ENABLE);
  DMA_Cmd(DMA2_Stream1, ENABLE);
  DMA_Cmd(DMA2_Stream2, ENABLE);

  /* DMA2_Stream0 Interupt enable */
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
  DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE);
  DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);


  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;
  ADC_Init(ADC1, &ADC_InitStructure);
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC2, &ADC_InitStructure);
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC3, &ADC_InitStructure);

  /* ADC1 regular channels configuration */
  ADC_RegularChannelConfig(ADC1,  ADC_Channel_5, 1, ADC_SampleTime_28Cycles);
  ADC_RegularChannelConfig(ADC1,  ADC_Channel_7, 2, ADC_SampleTime_28Cycles);
  ADC_RegularChannelConfig(ADC2,  ADC_Channel_13, 1, ADC_SampleTime_3Cycles);
  ADC_RegularChannelConfig(ADC3,  ADC_Channel_4, 1, ADC_SampleTime_3Cycles);

  NVIC_InitTypeDef      NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  ADC_DMACmd(ADC2, ENABLE);
  ADC_DMACmd(ADC3, ENABLE);

  /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);
  ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  ADC_Cmd(ADC2, ENABLE);
  ADC_Cmd(ADC3, ENABLE);


  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConv(ADC1);
  ADC_SoftwareStartConv(ADC2);
  ADC_SoftwareStartConv(ADC3);
}

void DMA2_Stream0_IRQHandler(void){
	DMA_ClearITPendingBit(DMA2_Stream0, DMA_FLAG_TCIF0);
	counter0++;
}
void DMA2_Stream1_IRQHandler(void){
	DMA_ClearITPendingBit(DMA2_Stream1, DMA_FLAG_TCIF1);
	counter1++;
}
void DMA2_Stream2_IRQHandler(void){
	DMA_ClearITPendingBit(DMA2_Stream2, DMA_FLAG_TCIF2);
	counter2++;
}

int main(void){
	SystemInit();
	ADC_Config();
	while (1){}
}
