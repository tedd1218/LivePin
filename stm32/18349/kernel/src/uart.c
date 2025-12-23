/**
 * @file uart.c
 *
 * @brief Interrupt-driven UART implementation. Similar to a stream buffer.
 *
 * @date Feb 9, 2025
 *
 * @author 
 */

 #include <rcc.h>
 #include <uart.h>
 #include <nvic.h>
 #include <gpio.h>
 
 /** @brief The UART register map. */
 struct uart_reg_map {
     volatile uint32_t SR;   /**< Status Register */
     volatile uint32_t DR;   /**<  Data Register */
     volatile uint32_t BRR;  /**<  Baud Rate Register */
     volatile uint32_t CR1;  /**<  Control Register 1 */
     volatile uint32_t CR2;  /**<  Control Register 2 */
     volatile uint32_t CR3;  /**<  Control Register 3 */
     volatile uint32_t GTPR; /**<  Guard Time and Prescaler Register */
 };
 
 /** @brief Base address for UART2 */
 #define UART2_BASE  (struct uart_reg_map *) 0x40004400
 
 #define TX_BUFFER_SIZE 256
 volatile char TX_BUFFER[TX_BUFFER_SIZE];
 volatile int TX_FIRST = 0;
 volatile int TX_LAST = 0; 
 volatile int NUM_ELEMENTSTX = 0;
 
 #define RX_BUFFER_SIZE 256
 volatile char RX_BUFFER[RX_BUFFER_SIZE];
 volatile int RX_FIRST = 0;
 volatile int RX_LAST = 0;
 volatile int NUM_ELEMENTSRX = 0;
 
 #define USARTDIV 0x8B
 #define RCC_EN (1 << 17)
 
 void uart_init(int baud){
   (void) baud; /* This line is simply here to suppress the Unused Variable Error. */
                  /* You should remove this line in your final implementation */
 
     // reset and clock signal (RCC)
     // always do first
     struct rcc_reg_map *rcc = RCC_BASE;
     rcc->apb1_enr |= RCC_EN;
 
     struct uart_reg_map *uart = UART2_BASE;
     uint32_t UART_EN = (1 << 13);
     uart->CR1 |= UART_EN;
 
     // it says TX and RX interrupts are the same wire??
     // TX
     gpio_init(GPIO_A, 2, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, 0x07);
 
     // RX
     gpio_init(GPIO_A, 3, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, 0x07);
 
 
     //brr
     uart->BRR |= USARTDIV;
 
     // enable transmitter and receiver
     uint32_t CR1TransmitMask = 1 << 3;
     uint32_t CR1RecieverMask = 1 << 2;
     uart->CR1 = uart->CR1 | CR1TransmitMask | CR1RecieverMask;
 
     // turn on recieving 
     uint32_t RXNEIE = 1 << 5;
     uart->CR1 = uart->CR1 | RXNEIE;

     //turn on tranmitting 
     uint32_t TXNEIE = 1 << 7;
     uart->CR1 = uart->CR1 | TXNEIE;

 
     TX_LAST = 0;
 
     nvic_irq(38, IRQ_ENABLE);
     // do nvic_clear_pending in the handlers
     return;
 }
 int uart_put_byte(char c) {
  struct uart_reg_map *uart = UART2_BASE;
  uint32_t txeieBitMask = 1 << 7;
  uart->CR1 &= ~txeieBitMask;

  if (NUM_ELEMENTSTX == TX_BUFFER_SIZE) {
      return -1; // Buffer full
  }

  TX_BUFFER[TX_LAST] = c;
  TX_LAST = (TX_LAST + 1) % TX_BUFFER_SIZE;
  NUM_ELEMENTSTX++;

  // Ensure TX interrupt is enabled so data gets transmitted
  uart->CR1 |= txeieBitMask;

  return 0;
}
 
 // RX interrupt enable
 int uart_get_byte(char *c) {
  if (NUM_ELEMENTSRX == 0) {
      return -1; // Buffer empty
  }

  *c = RX_BUFFER[RX_FIRST];
  RX_FIRST = (RX_FIRST + 1) % RX_BUFFER_SIZE;
  NUM_ELEMENTSRX--;

  return 0;
}
 
 void USART2_TX_IRQHandler() {
  struct uart_reg_map *uart = UART2_BASE;

  if (NUM_ELEMENTSTX > 0 && (uart->SR & (1 << 7))) {  // Check if TXE is set
      uart->DR = TX_BUFFER[TX_FIRST];
      TX_FIRST = (TX_FIRST + 1) % TX_BUFFER_SIZE;
      NUM_ELEMENTSTX--;
  }
  //Not empty->empty
  if (NUM_ELEMENTSTX == 0) {
      uart->CR1 &= ~(1 << 7);  //Disable TXEIE when buffer is empty
  }
}
 void USART2_RX_IRQHandler() {
  struct uart_reg_map *uart = UART2_BASE;
  
  if (uart->SR & (1 << 5)) {  
      RX_BUFFER[RX_LAST] = uart->DR;
      RX_LAST = (RX_LAST + 1) % RX_BUFFER_SIZE;
      NUM_ELEMENTSRX++;

      if (NUM_ELEMENTSRX == RX_BUFFER_SIZE) {
          uart->CR1 &= ~(1 << 5);  //Disable RX interrupt if buffer full
      }
  }
}
 
 void USART2_IRQHandler() {
  struct uart_reg_map *uart = UART2_BASE;

  if (uart->SR & (1 << 7)) { 
      USART2_TX_IRQHandler();
  }

  if (uart->SR & (1 << 5)) { 
      USART2_RX_IRQHandler();
  }

  nvic_clear_pending(38);
}

 void uart_flush(){
   struct uart_reg_map *uart = UART2_BASE;
   // TRANSMIT SET TO 0s
   while(TX_FIRST != TX_LAST) {
     TX_FIRST = (TX_FIRST + 1) % TX_BUFFER_SIZE;
   }
   // RX put everything in DR 
   // while dr is not empty wait until next RX put
   while (RX_FIRST != RX_LAST) {
     uint32_t statusBit = 1 << 7;
     while ((uart->SR & statusBit) == 0) {
       continue;
     }
     uart->DR = RX_BUFFER[RX_FIRST];
     RX_FIRST = (RX_FIRST + 1) % RX_BUFFER_SIZE;
   }
   RX_FIRST = 0;
   RX_LAST = 0;
   
   TX_FIRST = 0;
   TX_LAST = 0;
   uart->DR;
 }
