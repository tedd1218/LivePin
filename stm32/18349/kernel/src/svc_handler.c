/**
 * @file 
 *
 * @brief      
 *
 * @date       
 *
 * @author     
 */

#include <stdint.h>
#include <debug.h>
#include <syscall.h>
#include <svc_num.h>
#include <uart.h>
#include <printk.h>
#include <lcd_driver.h>
#include <arm.h>

// make sure the user program evutually gets the return value of the system call??
// not tested bc i didn't bring my circuit 

struct arm_calling_convention {
  volatile uint32_t r0;
  volatile uint32_t r1;
  volatile uint32_t r2;
  volatile uint32_t r3;
  volatile uint32_t r12;
  volatile uint32_t rLR;
  volatile uint32_t PC;
  volatile uint32_t xPSR;
};
//struct arm_calling_convention* reg
void svc_c_handler(void* regs) {
  struct arm_calling_convention* reg = (struct arm_calling_convention*) regs;
  int svc_number = (*((int*) ((reg->PC) - 2))) & 0xFF;
  // do all in svc_num break for functions not implemented 
  switch ( svc_number ) {
  case SVC_SBRK:
    reg->r0 = (uint32_t) sys_sbrk((int) reg->r0);
    break;
  case SVC_WRITE:
    reg->r0 = sys_write((STDOUT_FILENO), (char*) (uintptr_t) reg->r1, (int) reg->r2);
    break;
  case SVC_CLOSE:
    break;
  case SVC_FSTAT:
    break;
  case SVC_ISATTY:
    break;
  case SVC_LSEEK:
    break;
  case SVC_READ:
    reg->r0 = sys_read(STDIN_FILENO, (char*) (uintptr_t) reg->r1, (int) reg->r2);
    break;
  case SVC_EXIT:
    sys_exit((int) reg->r0);
    break;
  case SVC_KILL:
    break;
  case SVC_THR_INIT:
    break;
  case SVC_THR_CREATE:
    break;
  case SVC_THR_KILL:
    break;
  case SVC_GET_PID:
    break;
  case SVC_MUT_INIT:
    break;
  case SVC_MUT_LOK:
    break;
  case SVC_MUT_ULK:
    break;
  case SVC_WAIT:
    break;
  case SVC_TIME:
    break;
  case SVC_SCHD_START:
    break;
  case SVC_PRIORITY:
    break;
  case SVC_THR_TIME:
    break;
  case SVC_SLEEP_TILL_INT:
    break;
  case SVC_SERVO_ENABLE:
    reg->r0 = sys_servo_enable((uint8_t)reg->r0, (uint8_t)reg->r1);
    break;
  case SVC_SERVO_SET:
    reg->r0 = sys_servo_set((uint8_t)reg->r0, (uint8_t)reg->r1);
    break;
  default:
    break;
  }
}