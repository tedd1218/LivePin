/**
 * @file kernel.c
 *
 * @brief      Kernel entry point
 *
 * @date
 * @author
 */

#include <arm.h>
#include <kernel.h>
#include <uart.h>
#include <systick.h>
#include <printk.h>
#include <uart_polling.h>
#include <gpio.h>
#include <timer.h>
#include <servok.h>


int kernel_main( void ) {

  init_349(); // DO NOT REMOVE THIS LINE
  systick_init();
  uart_init(1);
  timer_init(2, 160, 1);
  // printk("my name is safiya i am very sad\n");

  //sys_read(STDIN_FILENO, input, 19);

  // enter_user_mode();

  
  //gpio init a led in whatever pin. 
  // gpio_init(GPIO_B, 10, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL,OUTPUT_SPEED_LOW,
  //   PUPD_NONE, ALT0);
  
  //timer_init(2, 160, 1);

  //sys_servo_enable(0,1);
  // sys_servo_enable(1,1);

  // for(int i =0; i<1000000; i++);
  // //sys_servo_set(0,90);

  // sys_servo_set(1,0);
  // for(int i =0; i<1000000; i++);

  // sys_servo_set(1,90);


  // testing svc (use a print statement after this)
  // enter_user_mode();


  while(1){
    //systick_delay(100);
    // printk("Interrupt \n");
    gpio_init(GPIO_B, 4, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW , PUPD_NONE, ALT0);
    gpio_set(GPIO_B, 4);
  }
  return 0;
}
