#include "main.h"
#include <stdlib.h>
#include "stm32f4xx.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"
#include "stm32_ub_graphic2d.h"
#include "stm32_ub_lcd_ili9341.h"
#include "misc.h"
#define GRAPH_MINX  10
#define GRAPH_MAXX 220
#define GRAPH_MINY  10
#define GRAPH_MAXY 300
#define CH1_COLOR RGB_COL_BLUE
#define CH2_COLOR RGB_COL_CYAN
#define CH3_COLOR RGB_COL_GREEN
#define CH4_COLOR RGB_COL_RED
#define GRAPH_COLOUR RGB_COL_WHITE
#define GRAPH_AMPL_DIV GRAPH_MAXX/10
#define GRAPH_TIME_DIV GRAPH_MAXY/10

DMA_InitTypeDef            DMA_InitStructure;
ADC_InitTypeDef            ADC_InitStructure;
ADC_CommonInitTypeDef      ADC_CommonInitStructure;
GPIO_InitTypeDef           GPIO_InitStructure;

volatile uint16_t ADC1Value[2] = {0};
volatile uint16_t ADC2Value = 0;
volatile uint16_t ADC3Value = 0;
volatile uint32_t counter0,counter1,counter2 = 0;
volatile unsigned index = 10;

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
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
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
  ADC_RegularChannelConfig(ADC1,  ADC_Channel_5, 1, ADC_SampleTime_3Cycles);
  ADC_RegularChannelConfig(ADC1,  ADC_Channel_7, 2, ADC_SampleTime_3Cycles);
  ADC_RegularChannelConfig(ADC2, ADC_Channel_13, 1, ADC_SampleTime_3Cycles);
  ADC_RegularChannelConfig(ADC3,  ADC_Channel_4, 1, ADC_SampleTime_3Cycles);

  NVIC_InitTypeDef      NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
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

void DrawGraph() {
	if(index==310){
		index=10;
		//while(1);
	}
	UB_Graphic2D_DrawPixelNormal(ADC1Value[1]/19+10,index,CH1_COLOR);
	UB_Graphic2D_DrawPixelNormal(ADC1Value[2]/19+10,index,CH2_COLOR);
	UB_Graphic2D_DrawPixelNormal(ADC2Value/19+10,index,CH2_COLOR);
	UB_Graphic2D_DrawPixelNormal(ADC3Value/19+10,index,CH3_COLOR);
	index++;;
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

void DrawScale(void){
	uint16_t x,y;
	UB_Graphic2D_DrawRectDMA(GRAPH_MINX,GRAPH_MINY,GRAPH_MAXX,GRAPH_MAXY,GRAPH_COLOUR);
	UB_Graphic2D_DrawLineNormal(GRAPH_MAXX/2+GRAPH_MINX,GRAPH_MINY,GRAPH_MAXX/2+GRAPH_MINX,GRAPH_MAXY+GRAPH_MINY,GRAPH_COLOUR);
	UB_Graphic2D_DrawLineNormal(GRAPH_MINX,GRAPH_MAXY/2+GRAPH_MINY,GRAPH_MAXX+GRAPH_MINX,GRAPH_MAXY/2+GRAPH_MINY,GRAPH_COLOUR);
	for(x=GRAPH_MINX+GRAPH_AMPL_DIV;x<GRAPH_MAXX;x+=GRAPH_AMPL_DIV){
		for(y=GRAPH_MINY+GRAPH_TIME_DIV;y<GRAPH_MAXY;y+=GRAPH_TIME_DIV){
			UB_Graphic2D_DrawPixelNormal(x,y,GRAPH_COLOUR);
		}
	}
}

int main(void){
	SystemInit();
	UB_LCD_Init();
	UB_LCD_LayerInit_Fullscreen();
	UB_LCD_SetLayer_1();
	UB_LCD_FillLayer(RGB_COL_BLACK);
	UB_LCD_SetLayer_2();
	UB_LCD_FillLayer(RGB_COL_BLACK);
	DrawScale();
	ADC_Config();
	while(1){
		/* Update screen with data ADC Samples for ever */
		DrawGraph();
	}
}
