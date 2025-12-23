/**
 * @file
 *
 * @brief
 *
 * @date
 *
 * @author
 */

#include <unistd.h>
#include <timer.h>
#include <rcc.h>
#include <nvic.h>
#include <gpio.h>
#include <printk.h>

#define UNUSED __attribute__((unused))

int timer_fired = 0; 

/** @brief tim2_5 */
struct tim2_5 {
  volatile uint32_t cr1; /**< 00 Control Register 1 */
  volatile uint32_t cr2; /**< 04 Control Register 2 */
  volatile uint32_t smcr; /**< 08 Slave Mode Control */
  volatile uint32_t dier; /**< 0C DMA/Interrupt Enable */
  volatile uint32_t sr; /**< 10 Status Register */
  volatile uint32_t egr; /**< 14 Event Generation */
  volatile uint32_t ccmr[2]; /**< 18-1C Capture/Compare Mode */
  volatile uint32_t ccer; /**< 20 Capture/Compare Enable */
  volatile uint32_t cnt; /**< 24 Counter Register */
  volatile uint32_t psc; /**< 28 Prescaler Register */
  volatile uint32_t arr; /**< 2C Auto-Reload Register */
  volatile uint32_t reserved_1; /**< 30 */
  volatile uint32_t ccr[4]; /**< 34-40 Capture/Compare */
  volatile uint32_t reserved_2; /**< 44 */
  volatile uint32_t dcr; /**< 48 DMA Control Register */
  volatile uint32_t dmar; /**< 4C DMA address for full transfer Register */
  volatile uint32_t or; /**< 50 Option Register */
};

struct tim2_5* const timer_base[] = {(void *)0x0,   // N/A - Don't fill out
                                     (void *)0x0,   // N/A - Don't fill out
                                     (void *)0x40000000,    // TODO: fill out address for TIMER 2
                                     (void *)0x40000400,    // TODO: fill out address for TIMER 3
                                     (void *)0x40000800,    // TODO: fill out address for TIMER 4
                                     (void *)0x40000C00};   // TODO: fill out address for TIMER 5


void timer_init(int timer, uint32_t prescalar, uint32_t period) {
  struct tim2_5 *timerStruct = timer_base[timer];

  // rcc enable 
  struct rcc_reg_map *rcc = RCC_BASE;
  uint32_t rccTimerEn = 1 << (timer - 2);
  // are OR operators still legal?
  rcc->apb1_enr = rcc->apb1_enr | rccTimerEn;

  // enable counter in ctrl 1
  uint16_t counterEn = 1;
  timerStruct->cr1 |= counterEn;

  // prescalar and period  
  //The counter clock frequency (CK_CNT) is equal to fCK_PSC / (PSC[15:0] + 1).
  //uint32_t sys_clk = 16000000; //16Mhz  (CHECK THIS) systick_get_ticks();
  
  //prescalar acts as a divided to enable timers to measure longer periods of time 
  timerStruct->psc = prescalar;
  

  // put period in prelaod register
  // do i mean reload instead of preload
  timerStruct->arr = period;
  
  // EGR 
  // put before the prescalar bc it says this clears the prescalar counter
  uint16_t ugEn = 1;
  timerStruct->egr |= ugEn;


  // DIER UIE
  timerStruct->dier |= (1 <<0); //UIE: Update Intterupt enable

  // NVIC interrupt enable (ensures that when a timer overflows an intterupt is triggered)
  if(timer ==2 ){
    nvic_irq(28, IRQ_ENABLE);
  }
  else if(timer ==3){
    nvic_irq(29, IRQ_ENABLE);
  }
  else if(timer ==4){
    nvic_irq(30, IRQ_ENABLE);
  }
  else if (timer == 5){
    nvic_irq(50, IRQ_ENABLE);
  }
}

void timer_disable(int timer) {
  struct rcc_reg_map *rcc = RCC_BASE;
  // struct tim2_5 *timerStruct = timer_base[timer + 2];

  //disable rcc  
  if (timer == 2){
    rcc->apb1_enr &= (0<<0); //TIM2 Clock Disable
  }
  else if(timer == 3){
    rcc->apb1_enr &= (0<<1); //TIM3 Clock Disable
  }
  else if(timer == 4){
    rcc->apb1_enr &= (0<<2); //TIM4 Clock Disable
  }
  else if(timer == 5){
    rcc->apb1_enr &= (0<<3);  //TIM4 Clock Disable
  }

}

void timer_clear_interrupt_bit(int timer) {
  struct tim2_5 *timerStruct = timer_base[timer];

  //clear update interrupt flag in SR
  timerStruct->sr &= ~(1<<0);


}

//add timer handler 
//turn on led every 

// void TIM2_IRQHANDLER(){

//   //turn led on
//   if(timer_fired){
//     gpio_set(GPIO_B,10);
//     timer_fired = 0; 
//   }
//   else{
//     gpio_clr(GPIO_B,10);
//     timer_fired = 1; 
//   }
//   printk("hello \n");
  
//   //clear intterupt flag
//   timer_clear_interrupt_bit(2);
// } 