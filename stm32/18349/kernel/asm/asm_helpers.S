.cpu cortex-m4
.syntax unified
.thumb

.section .text

.thumb_func
.global enter_user_mode
enter_user_mode:
  ldr r0, = __psp_stack_top //load the address of process stack into r0
  msr psp, r0             //Set PSP (move to system register)
  //can i just use store?  str/msr? 

  b _crt0

  //do we need to set the control register? 

