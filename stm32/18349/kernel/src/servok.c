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
#include <syscall.h>
#include <gpio.h>
#include <timer.h>

#define UNUSED __attribute__((unused))


volatile int TOTAL_COUNT = 0;
volatile int pulse_width[3] = {0,0,0}; //pulse width of each servo
volatile int servo_enabled[3] = {0,0,0}; // enabled state of each servo 

// channel means servo 1 or servo 2
int sys_servo_enable(uint8_t channel, uint8_t enabled){
  if(channel >2) {return -1; }
  servo_enabled[channel] = enabled; 

  if(enabled){
    //enable the periodic pulse signal on the given channel
    // using timer 2
    
    if (channel == 1) {
      gpio_init(GPIO_B, 5, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW , PUPD_NONE, ALT0);
    }
    else if (channel == 0) {
      gpio_init(GPIO_A, 10, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW , PUPD_NONE, ALT0);
    }
    else if (channel == 2) {
      gpio_init(GPIO_B, 3, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW , PUPD_NONE, ALT0);
    }
  }
  else{
    //set the channel output to low 
    if (channel == 1) {
      gpio_clr(GPIO_B, 5);
    }
    else if (channel == 0) {
      gpio_clr(GPIO_A, 10);
    }
    else if (channel == 2) {
      gpio_clr(GPIO_B, 3);
    }
  }
  
  return 0;
}

int sys_servo_set(uint8_t channel, uint8_t angle){
  pulse_width[channel] = angle + 60;
  // convert angle to width of pulse
  return 0;
}

// have global timer
// when global timer val equls width pulse 
// as long as width pulse 
// width tells you "where" to go
// tick for width pulse time then clear
void TIM2_IRQHANDLER() {
  TOTAL_COUNT++; 
  if (TOTAL_COUNT >= 1000) { //reset every 20ms 
    TOTAL_COUNT = 0;
  }

  if(servo_enabled[0]){
    if(TOTAL_COUNT == 0){
      gpio_set(GPIO_A,10);
    }
    if(TOTAL_COUNT == pulse_width[0]/2){
      gpio_clr(GPIO_A,10); //stop PWM
    }
  }

  if(servo_enabled[1]){
    if(TOTAL_COUNT == 0){
      gpio_set(GPIO_B, 5); 
    }
    if(TOTAL_COUNT == pulse_width[1]/2){
      gpio_clr(GPIO_B,5);
    }

  }

  if(servo_enabled[2]){
    if(TOTAL_COUNT == 0){
      gpio_set(GPIO_B, 3); 
    }
    if(TOTAL_COUNT == pulse_width[2]/2){
      gpio_clr(GPIO_B,3);
    }

  }

  timer_clear_interrupt_bit(2);
}