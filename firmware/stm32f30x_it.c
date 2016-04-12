#include "stm32f30x_it.h"
#include "hw.h"
#include "usb_istr.h"
#include "control_loop.h"
#include "motor.h"
#include "sine.h"

volatile uint16_t spare_cycles;
extern __IO int16_t adc_conversion[6];
extern int16_t adc_zero[6];

void HardFault_Handler(void) {
    for(;;);
}

void MemManage_Handler(void) {
    for(;;);
}

void BusFault_Handler(void) {
    for(;;);
}

void UsageFault_Handler(void) {
    for(;;);
}

void USB_LP_CAN1_RX0_IRQHandler(void) {
    USB_Istr();
}

void USBWakeUp_IRQHandler(void) {
    EXTI_ClearITPendingBit(EXTI_Line18);
}

static uint8_t first;

// TIM17_UP
void TIM1_TRG_COM_TIM17_IRQHandler(void) {
    first = (TIM1->CNT < PWM_PERIOD / 2);
    hw_start_adc_conversion();
    TIM_ClearFlag(TIM17, TIM_FLAG_Update);
}

// ADC DMA TC
void DMA1_Channel1_IRQHandler(void) {
    if(first) {
        if(TIM1->CCR1  >= PWM_PERIOD / 2) adc_buffer[0] = adc_conversion[0] - adc_zero[0];
        if(TIM1->CCR2  >= PWM_PERIOD / 2) adc_buffer[1] = adc_conversion[1] - adc_zero[1];
        if(TIM1->CCR3  >= PWM_PERIOD / 2) adc_buffer[2] = adc_conversion[2] - adc_zero[2];
        if(TIM2->CCR3  >= PWM_PERIOD / 2) adc_buffer[3] = adc_conversion[3] - adc_zero[3];
        if(TIM15->CCR1 >= PWM_PERIOD / 2) adc_buffer[4] = adc_conversion[4] - adc_zero[4];
        if(TIM2->CCR1  >= PWM_PERIOD / 2) adc_buffer[5] = adc_conversion[5] - adc_zero[5];
        DMA_ClearITPendingBit(DMA1_IT_TC1);
    } else {
        if(TIM1->CCR1  < PWM_PERIOD / 2) adc_buffer[0] = adc_zero[0] - adc_conversion[0];
        if(TIM1->CCR2  < PWM_PERIOD / 2) adc_buffer[1] = adc_zero[1] - adc_conversion[1];
        if(TIM1->CCR3  < PWM_PERIOD / 2) adc_buffer[2] = adc_zero[2] - adc_conversion[2];
        if(TIM2->CCR3  < PWM_PERIOD / 2) adc_buffer[3] = adc_zero[3] - adc_conversion[3];
        if(TIM15->CCR1 < PWM_PERIOD / 2) adc_buffer[4] = adc_zero[4] - adc_conversion[4];
        if(TIM2->CCR1  < PWM_PERIOD / 2) adc_buffer[5] = adc_zero[5] - adc_conversion[5];

        TIM1->CCR1 =  (motor[0].voltage_a + PWM_PERIOD) / 2;
        TIM1->CCR2 =  (motor[0].voltage_b + PWM_PERIOD) / 2;
        TIM1->CCR3 =  (motor[1].voltage_a + PWM_PERIOD) / 2;
        TIM2->CCR3 =  (motor[1].voltage_b + PWM_PERIOD) / 2;
        TIM2->CCR4 =  (motor[1].voltage_b + PWM_PERIOD) / 2;
        TIM15->CCR1 = (motor[2].voltage_a + PWM_PERIOD) / 2;
        TIM2->CCR1 =  (motor[2].voltage_b + PWM_PERIOD) / 2;
        TIM2->CCR2 =  (motor[2].voltage_b + PWM_PERIOD) / 2;

        DMA_ClearITPendingBit(DMA1_IT_TC1);

        motor[0].current_a = (float)adc_buffer[0] * CURRENT_SCALE;
        motor[0].current_b = (float)adc_buffer[1] * CURRENT_SCALE;
        motor[1].current_a = (float)adc_buffer[2] * CURRENT_SCALE;
        motor[1].current_b = (float)adc_buffer[3] * CURRENT_SCALE;
        motor[2].current_a = (float)adc_buffer[4] * CURRENT_SCALE;
        motor[2].current_b = (float)adc_buffer[5] * CURRENT_SCALE;

        control_loop();

        motor[0].voltage_a = cosine(motor[0].p + motor[0].zero, motor[0].amp);
        motor[0].voltage_b =   sine(motor[0].p + motor[0].zero, motor[0].amp);
        motor[1].voltage_a = cosine(motor[1].p + motor[1].zero, motor[1].amp);
        motor[1].voltage_b =   sine(motor[1].p + motor[1].zero, motor[1].amp);
        motor[2].voltage_a = cosine(motor[2].p + motor[2].zero, motor[2].amp);
        motor[2].voltage_b =   sine(motor[2].p + motor[2].zero, motor[2].amp);
    }
}
