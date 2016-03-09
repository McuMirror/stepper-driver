#include "hw.h"
#include "stm32f30x.h"
#include "usb_lib.h"

void hw_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  
    // LED
    hw_led_off();
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // USB
    Set_System();
    Set_USBClock();
    USB_Config();
    USB_Init();
}

void hw_led_on() {
    GPIOC->BSRR = GPIO_Pin_14;
}

void hw_led_off() {
    GPIOC->BRR = GPIO_Pin_14;
}
