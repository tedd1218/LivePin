#include <unistd.h>
#include <printk.h>
#include <gpio.h>
#include <keypad_driver.h>
#include <systick.h>

// D8 col1
#define COL1_PORT GPIO_A
#define COL1_PIN 9

// D6 col2 
#define COL2_PORT GPIO_B
#define COL2_PIN 10

// D10 col3
#define COL3_PORT GPIO_B
#define COL3_PIN 6

// D7 row1
#define ROW1_PORT GPIO_A
#define ROW1_PIN 8

// D12 row2
#define ROW2_PORT GPIO_A
#define ROW2_PIN 6

// D11 row3
#define ROW3_PORT GPIO_A
#define ROW3_PIN 7

// D9 row4
#define ROW4_PORT GPIO_C
#define ROW4_PIN 7

void keypad_init() {
    gpio_init(ROW1_PORT, ROW1_PIN, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, 
                PUPD_PULL_UP, ALT0); //row1 = D7
                
    gpio_init(COL2_PORT, COL2_PIN, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, 
                PUPD_PULL_DOWN, ALT0); //col2 = D6
                
    gpio_init(ROW2_PORT, ROW2_PIN, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, 
                PUPD_PULL_UP, ALT0); //row2 = d12
                
    gpio_init(ROW3_PORT, ROW3_PIN, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, 
                PUPD_PULL_UP, ALT0); //row3 = d11
                
    gpio_init(COL3_PORT, COL3_PIN, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, 
                PUPD_PULL_DOWN, ALT0); //col3 = d10
                
    gpio_init(ROW4_PORT, ROW4_PIN, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, 
                PUPD_PULL_UP, ALT0); //row4 = d9

    gpio_init(COL1_PORT, COL1_PIN, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, 
                PUPD_PULL_DOWN, ALT0); //col1 = d8
    return; 
}

char keypad_read() { //rows input, cols are gpio outputs , read row set col
    // clear everything
    gpio_clr(ROW1_PORT, ROW1_PIN);
    gpio_clr(ROW2_PORT, ROW2_PIN);
    gpio_clr(ROW3_PORT, ROW3_PIN);
    gpio_clr(ROW4_PORT, ROW3_PIN);
    
    uint32_t curTime = systick_get_ticks();
    // idk the unit of time for this
    while(systick_get_ticks() < curTime + 100) {

    }

    // row 1
    gpio_set(ROW1_PORT, ROW1_PIN); 
    // col 1
    if (gpio_read(COL1_PORT, COL1_PIN) == 1) {
        return '1';
    }
    // col 2
    else if ((gpio_read(COL2_PORT, COL2_PIN) == 1)) {
        return '2';
    }
    // col 3
    else if ((gpio_read(COL3_PORT, COL3_PIN) == 1)) {
        return '3';
    }
    else {
        gpio_clr(ROW1_PORT, ROW1_PIN);
    }

    // row 2
    gpio_set(ROW2_PORT, ROW2_PIN);
    // col 1
    if (gpio_read(COL1_PORT, COL1_PIN) == 1) {
        return '4';
    }
    // col 2
    else if ((gpio_read(COL2_PORT, COL2_PIN) == 1)) {
        return '5';
    }
    // col 3
    else if ((gpio_read(COL3_PORT, COL3_PIN) == 1)) {
        return '6';
    }
    else {
        gpio_clr(ROW2_PORT, ROW2_PIN);
    }

    // row 3
    gpio_set(ROW3_PORT, ROW3_PIN);
    // col 1
    if (gpio_read(COL1_PORT, COL1_PIN) == 1) {
        return '7';
    }
    // col 2
    else if ((gpio_read(COL2_PORT, COL2_PIN) == 1)) {
        return '8';
    }
    // col 3
    else if ((gpio_read(COL3_PORT, COL3_PIN) == 1)) {
        return '9';
    }
    else {
        gpio_clr(ROW3_PORT, ROW3_PIN);
    }

    // row 4
    gpio_set(ROW4_PORT, ROW4_PIN);
    if (gpio_read(COL1_PORT, COL1_PIN) == 1) {
        return '*';
    }
    // col 2
    else if ((gpio_read(COL2_PORT, COL2_PIN) == 1)) {
        return '0';
    }
    // col 3
    else if ((gpio_read(COL3_PORT, COL3_PIN) == 1)) {
        return '#';
    }
    else {
        gpio_clr(ROW4_PORT, ROW4_PIN);
    }

    //need to do row 4
    return '\0';
}
