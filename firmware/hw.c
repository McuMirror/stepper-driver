#include "hw.h"
#include "stm32f30x.h"
#include "usb_lib.h"
#include <string.h>

__IO int16_t adc_conversion[6];
int16_t adc_zero[6];
int16_t adc_buffer[6];
uint8_t run_control_loop = 1;

void hw_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef  DMA_InitStructure;
 
    // Clocks 
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
                          RCC_AHBPeriph_ADC12 | RCC_AHBPeriph_DMA1, ENABLE);
    RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div1);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_TIM15 | RCC_APB2Periph_TIM17, ENABLE);
 
    // LED
    hw_led_off();
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // PWM Output Timers
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);

    // ADC Timer (measure twice per cycle)
    TIM_TimeBaseStructure.TIM_Period = (PWM_PERIOD / 2) - 1;
    TIM_TimeBaseInit(TIM17, &TIM_TimeBaseStructure);

    // PWM Outputs
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_Pulse = PWM_PERIOD / 2;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
    TIM_OC1Init(TIM15, &TIM_OCInitStructure);

    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM15, ENABLE);
    TIM_Cmd(TIM17, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM2, ENABLE);
    TIM_CtrlPWMOutputs(TIM15, ENABLE);

    // Synchronize Timers
    TIM1->CNT = 0;
    TIM2->CNT = TIM1->CNT;
    TIM15->CNT = TIM1->CNT;
    TIM1->CNT = TIM1->CNT;
    TIM17->CNT = TIM1->CNT + (PWM_PERIOD / 4);

    // PWM Output GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                          GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3| GPIO_Pin_10 |
                                  GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7,  GPIO_AF_6);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8,  GPIO_AF_6);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource0,  GPIO_AF_6);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_6);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource1,  GPIO_AF_6);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_6);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3,  GPIO_AF_1);

    // ADC DMA
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_conversion;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 6;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    // ADC Calibration
    ADC_DeInit(ADC1);
    ADC_VoltageRegulatorCmd(ADC1, ENABLE);
    static volatile int i;
    for(i = 0; i < 720000; i++); // 10 us delay

    ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
    ADC_StartCalibration(ADC1);

    while(ADC_GetCalibrationStatus(ADC1) != RESET);

    // ADC
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
    ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;

    ADC_CommonInit(ADC1, &ADC_CommonInitStructure);

    ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 
    ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;         
    ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
    ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;  
    ADC_InitStructure.ADC_NbrOfRegChannel = 6;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  1, ADC_SampleTime_1Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2,  2, ADC_SampleTime_1Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3,  3, ADC_SampleTime_1Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4,  4, ADC_SampleTime_1Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5,  5, ADC_SampleTime_1Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 6, ADC_SampleTime_1Cycles5);

    ADC_Cmd(ADC1, ENABLE);
    ADC_DMAConfig(ADC1, ADC_DMAMode_Circular);
    ADC_DMACmd(ADC1, ENABLE);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));

    // Current sense GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                  GPIO_Pin_4 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Calibrate zero-current
    ADC_StartConversion(ADC1);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOS));
    for(int i = 0; i < 6; i++) {
        adc_zero[i] = adc_conversion[i];
    }

    // NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_TRG_COM_TIM17_IRQn; // TIM17 = ADC Read Start
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // DMA1 = ADC Read Done
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, ENABLE);

    // USB
    Set_System();
    Set_USBClock();
    USB_Config();
    USB_Init();
}

void hw_start_adc_conversion() {
    ADC_StartConversion(ADC1);
}

void hw_led_on() {
    GPIOC->BSRR = GPIO_Pin_14;
}

void hw_led_off() {
    GPIOC->BRR = GPIO_Pin_14;
}
