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
#include <uart.h>
#include <printk.h>
#include <lcd_driver.h>
#include <arm.h>

#define UNUSED __attribute__((unused))

extern uint8_t __heap_top;
extern uint8_t  __heap_low;

void* CURRENT_PROGRAM = &__heap_low;

void *sys_sbrk(int incr){
  void* cur = CURRENT_PROGRAM;
  CURRENT_PROGRAM += incr;
  if ((int) CURRENT_PROGRAM > (int) &__heap_top) {
    return (void *) -1;
  }
  return cur; 
}

int sys_write(int file,char *ptr, int len){
  if(file != STDOUT_FILENO){
    return -1;
  }
  int i = 0;
  while( i < len){
    int ret = uart_put_byte(ptr[i]);//send byte to UART
    if(ret != -1){// if its not full (Transmission successful)
      i++; 
    }
  }
  return i; //Number of bytes written 
}

int sys_read(int file, char *ptr, int len) {
  //read should read bytes one by one and echo each byte back as they are read  
  int count = 0;  
  //if not STDIN
  if (file != STDIN_FILENO){
    return -1;
  }
  //if STDIN

  else if(file == STDIN_FILENO) {
    while(count < len){
      char a;
      while(uart_get_byte(&a) == -1){}

      if(a == ('\4')){ //closure of the stream of characters 
        //ptr[count] = '\0';
        return (count); //number of charracters read 
      }
      else if(a == '\b'){//backspace (erases previously written character)
        if(count > 0){ //Only delete if buffer not empty
          count--;
        }
        printk("\b \b"); 
      }
      else if((a == '\n')){ //newline gets placed in the buffer and echoed back 
        ptr[count] = '\n';
        count++;
        uart_put_byte('\n');
        return count;
      }
      else if(a =='\r'){
        //printk("\r\n");
        ptr[count] = '\r';
        count++;
        printk("\r\n");
        return count;
      }
      else{ //normal character
        uart_put_byte(a);
        ptr[count] = a; 
        count++;
      }
    }
  }
  return count;
}

void sys_exit(int status){
  printk("%d \n", status); //print the exit code 
  uart_flush(); //flush the UART

  lcd_print((char*) status); //should we initialize?
  
  disable_interrupts();
  while(1);
}