#include <unistd.h>

#include <gpio.h>
#include <i2c.h>
#include <rcc.h>
/** @brief I2C register map from reference manual */
struct I2C_reg_map{
	volatile unsigned long I2C_CR1;  // Control register 1
	volatile unsigned long I2C_CR2;  // Control register 2
	volatile unsigned long I2C_OAR1; // Own Address register1 
	volatile unsigned long I2C_OAR2; // Own Address register2 
	volatile unsigned long I2C_DR;   // Data Register
	volatile unsigned long I2C_SR1;  // Status Register 1
	volatile unsigned long I2C_SR2;  // Status Regiser 2 
	volatile unsigned long I2C_CCR;  // Clock Control Register
	volatile unsigned long I2C_TRISE;// TRISE Register
	volatile unsigned long I2C_FLTR; // FLTR Register
};

/** @brief Base address for I2C */
#define I2C_BASE  ((struct I2C_reg_map *) 0x40005400)

/** @brief initializing an i2c */
//#define i2c ((struct I2C_reg_map *)I2C_BASE)

/** @brief mask to enable proper RCC */
#define RCC_EN (1 << 21)


/** mask for frequency of clock */
#define I2CDIV 0x8B 
#define FREQ 0x10

//we will be using D14 and D15 pins for I2c 
// clk should equal (16*10^6) / (100*10^3) when called
void i2c_master_init(uint16_t clk){

    struct I2C_reg_map *i2c = I2C_BASE;

    //gpio pins D14 + D15 
	gpio_init(GPIO_B, 8, MODE_ALT, OUTPUT_OPEN_DRAIN,
        OUTPUT_SPEED_LOW, PUPD_NONE, ALT4); //D15 pull down or

    gpio_init(GPIO_B, 9, MODE_ALT, OUTPUT_OPEN_DRAIN, 
              OUTPUT_SPEED_LOW, PUPD_NONE, ALT4); //D14 (alt4?)

    
    //RCC init
    struct rcc_reg_map *rcc = RCC_BASE;
    rcc-> apb1_enr |= RCC_EN; 

    //peripheral clock frequency
    //i2c -> I2C_CR2 |= I2CDIV; //16Mhz 
    i2c -> I2C_CR2 = FREQ;

    //set the clock 
    i2c -> I2C_CCR = clk; //100khz 

    //enable i2c
    i2c -> I2C_CR1 |= 0x1; 
    //enable ACK 
    // acknowledge returned after a byte is received
    i2c -> I2C_CR1 |= (1 << 10);

    //set maximum rise time? 
    return;
}

void i2c_master_start() {
    struct I2C_reg_map *i2c = I2C_BASE;
    uint16_t startBitMask = 0x1;

    //make sure that bus not busy before setting control bit 
    // always evaluates to false
    while(((i2c->I2C_SR2) & 1 << 1) != 0){
        continue; 
    }

    //start Condition
    i2c -> I2C_CR1 |= (1<<8); //set start bit

    while((i2c->I2C_SR1&startBitMask) == 0){ //1 == Start condition 0 == No Start Condition
        continue;
    }
    return;
}

void i2c_master_stop() {
    //set control bit
    struct I2C_reg_map *i2c = I2C_BASE ;
    
    //set stop bit

    //Byte Transfer finished  (aka BTF bit)
    while(((i2c->I2C_SR1) & (1<<2)) == 0){
        continue;
    } 

    //wait until slave mode
    // while( (i2c->I2C_SR2|0x1) == 1){ //wait till slave mode
    //     continue;
    // }
    // this is wrong
    uint16_t stopBitMask = 1 << 7; //4th bit (binary = 1000)

    while((((i2c->I2C_SR1) & stopBitMask))== 0){ //1 == Stop condition 0 == No stop Condition
        continue;
    }

    i2c -> I2C_CR1 |= (1 <<9); 
    
    return;
}

int i2c_master_write(uint8_t *buf, uint16_t len, uint8_t slave_addr){

    struct I2C_reg_map *i2c = I2C_BASE;

    i2c_master_start(); //wait for start bit
 
    //send slave address
    i2c -> I2C_DR = (slave_addr << 1); //data register, only need 7 bits of slave addr

    //wait for master address to be sent (bit 1 of status reg)
    while(((i2c -> I2C_SR1) & (1 << 1) ) == 0){
        continue; 
    }

    //clear ADDR flag (This bit is cleared by software reading SR1 register followed reading SR2
    // clraddr = i2c -> I2C_SR1;
    volatile unsigned long clraddr = i2c -> I2C_SR2; 
    (void) clraddr;

    //send data
    for(uint16_t i =0 ; i< len; i++){
        //TXE bit
        //wait till data register is empty, ensures that the previous data was transfered
        while(((i2c->I2C_SR1) & (1<<7)) == 0){ 
            continue;
        }
        i2c -> I2C_DR = buf[i]; //load data register with buf contents
    }

    i2c_master_stop();
    return 0;
}

int i2c_master_read(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    (void) buf;
    (void) len;
    (void) slave_addr;

    return 0;
}
