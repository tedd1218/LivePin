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
#include <systick.h>

struct systick_reg_map {
    volatile unsigned long STK_CTRL;
    volatile unsigned long STK_LOAD;
    volatile unsigned long STK_VAL;
    volatile unsigned long STK_CALIB;
};

#define SYSTICK_BASE (struct systick_reg_map*) 0xE000E010
volatile uint32_t GLOBAL_TICK_COUNT = 0;

void systick_init() {
    struct systick_reg_map *systick = SYSTICK_BASE;
    uint32_t ENABLE = 1;
    uint32_t TICKINT = 1 << 1;
    uint32_t CLKSRC = 1 << 2; 

    // 16 mHz
    systick->STK_LOAD = 15999;

    systick->STK_CTRL = (systick->STK_CTRL) | CLKSRC;
    systick->STK_CTRL = (systick->STK_CTRL) | ENABLE;
    systick->STK_CTRL = (systick->STK_CTRL) | TICKINT;
}

void systick_delay(uint32_t ticks) {
    uint32_t cur = GLOBAL_TICK_COUNT + ticks;
    while (GLOBAL_TICK_COUNT < cur) 
    {

    }
}

uint32_t systick_get_ticks() {
    return GLOBAL_TICK_COUNT;
}

void systick_c_handler() {
    GLOBAL_TICK_COUNT++;
}