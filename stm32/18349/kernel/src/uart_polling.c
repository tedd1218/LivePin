#include <unistd.h>

#include <gpio.h>
#include <rcc.h>
#include <uart_polling.h>
#include <printk.h>

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

/** @brief Enable  Bit for UART Config register */
#define UART_EN (1 << 13)

/** 0x08B0 =  */
#define USARTDIV 0x8B

#define RCC_EN (1 << 17)

#define uart ((struct uart_reg_map *)UART2_BASE)

/**
 * @brief initializes UART to given baud rate with 8-bit word length, 1 stop bit, 0 parity bits
 *
 * @param baud Baud rate
 */
void uart_polling_init (int baud){
    (void) baud; /* This line is simply here to suppress the Unused Variable Error. */
                 /* You should remove this line in your final implementation */

    // reset and clock signal (RCC)
    // always do first
    struct rcc_reg_map *rcc = RCC_BASE;
    rcc->apb1_enr |= RCC_EN;

    uart->CR1 |= UART_EN;

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

    return;
}

/**
 * @brief transmits a byte over UART
 *
 * @param c character to be sent
 */
void uart_polling_put_byte (char c){
    
    (void) c;
    uint32_t statusBitMask = 1<<7; //7th bit
    uint32_t statusBit = (uart->SR & statusBitMask);
    
    while(statusBit == 0){ //data register is not empty
        statusBit = (uart->SR & statusBitMask);
        continue;
    }

    uart->DR = c; 
    return;
}

/**
 * @brief receives a byte over UART
 */
char uart_polling_get_byte () {
    uint32_t statusBitMask = 1 << 5; //fifth bit
    uint32_t statusBit = (uart->SR & statusBitMask);

    while(statusBit == 0){ //data register is empty
        statusBit = (uart->SR & statusBitMask);
        continue;
    }
    return (char)(uart->DR); 
    //should we use gpio_read? 
}
