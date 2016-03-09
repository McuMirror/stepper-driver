#include "hw.h"
#include "stm32f30x.h"
#include "usb_lib.h"

#define PWM_PERIOD 2250

void hw_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
 
    // Clocks 
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_TIM15, ENABLE);
 
    // LED
    hw_led_off();
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // PWM Output Timers
    hw_pwm(0, 0, 0, 0, 0, 0);

    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Set;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1Init(TIM15, &TIM_OCInitStructure);

    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM15, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM2, ENABLE);
    TIM_CtrlPWMOutputs(TIM15, ENABLE);

    // PWM Output GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                          GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
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

    // USB
    Set_System();
    Set_USBClock();
    USB_Config();
    USB_Init();
}

#define ENABLE_PWM(GPIO, PINSOURCE) {(GPIO)->MODER |= 1 << (2 * (PINSOURCE) + 1);}
#define DISABLE_PWM(GPIO, PINSOURCE) {(GPIO)->MODER &= ~(1 << (2 * (PINSOURCE) + 1));}

void hw_pwm(int16_t m1a, int16_t m1b, int16_t m2a, int16_t m2b, int16_t m3a, int16_t m3b) {
    if(m1a >= 0) {
        TIM1->CCR1 = m1a;
        ENABLE_PWM(GPIOA,  GPIO_PinSource7);
        DISABLE_PWM(GPIOA, GPIO_PinSource8);
    } else {
        TIM1->CCR1 = -m1a;
        DISABLE_PWM(GPIOA, GPIO_PinSource7);
        ENABLE_PWM(GPIOA,  GPIO_PinSource8);
    }

    if(m1b >= 0) {
        TIM1->CCR2 = m1b;
        ENABLE_PWM(GPIOB,  GPIO_PinSource0);
        DISABLE_PWM(GPIOA, GPIO_PinSource9);
    } else {
        TIM1->CCR2 = -m1b;
        DISABLE_PWM(GPIOB, GPIO_PinSource0);
        ENABLE_PWM(GPIOA,  GPIO_PinSource9);
    }

    if(m2a >= 0) {
        TIM1->CCR3 = m2a;
        ENABLE_PWM(GPIOB,  GPIO_PinSource1);
        DISABLE_PWM(GPIOA, GPIO_PinSource10);
    } else {
        TIM1->CCR3 = -m2a;
        DISABLE_PWM(GPIOB, GPIO_PinSource1);
        ENABLE_PWM(GPIOA,  GPIO_PinSource10);
    }

    if(m2b >= 0) {
        TIM2->CCR3 = m2b;
        TIM2->CCR4 = m2b;
        ENABLE_PWM(GPIOB,  GPIO_PinSource10);
        DISABLE_PWM(GPIOB, GPIO_PinSource11);
    } else {
        TIM2->CCR3 = -m2b;
        TIM2->CCR4 = -m2b;
        DISABLE_PWM(GPIOB, GPIO_PinSource10);
        ENABLE_PWM(GPIOB,  GPIO_PinSource11);
    }

    if(m3a >= 0) {
        TIM15->CCR1 = m3a;
        ENABLE_PWM(GPIOB,  GPIO_PinSource14);
        DISABLE_PWM(GPIOB, GPIO_PinSource15);
    } else {
        TIM15->CCR1 = -m3a;
        DISABLE_PWM(GPIOB, GPIO_PinSource14);
        ENABLE_PWM(GPIOB,  GPIO_PinSource15);
    }

    if(m3b >= 0) {
        TIM2->CCR1 = m3b;
        TIM2->CCR2 = m3b;
        ENABLE_PWM(GPIOA,  GPIO_PinSource15);
        DISABLE_PWM(GPIOB, GPIO_PinSource3);
    } else {
        TIM2->CCR1 = -m3b;
        TIM2->CCR2 = -m3b;
        DISABLE_PWM(GPIOA, GPIO_PinSource15);
        ENABLE_PWM(GPIOB,  GPIO_PinSource3);
    }
}

void hw_led_on() {
    GPIOC->BSRR = GPIO_Pin_14;
}

void hw_led_off() {
    GPIOC->BRR = GPIO_Pin_14;
}
