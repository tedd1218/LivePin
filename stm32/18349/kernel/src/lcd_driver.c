#include <unistd.h>

#include <i2c.h>
#include <lcd_driver.h>
#include <systick.h>

//P0 (connected to RS on the LCD) toggles between instruction mode and data mode. RS=0 indicates that you
// are sending an instruction (clearing the display, moving cursor, etc.). RS=1 indicates you are writing data
// (sending characters to display).
// • P1 (connected to RW on the LCD) should always be set to 0.
// • P2 (connected to E on the LCD) needs to be pulsed (set then unset after a 4 bit packet is sent).
// • P3 (not shown in figure) is connected to an internal transistor controlling the LCD backlight and should always
// be set as 1

// just input this when it is called
// #define instr_data_mode 0x1 //p0 instruction vs data mode 
#define read_write 0 //read or write, we are always writing
#define LCD_enable (1 << 2) //p2 should be set then unset
//#define backLight (1 << 3) //p3 = backlight, should always be 1

#define PCF874_ADDR 0x27 //0 1 0 0 A1 A2 A0 where A1,A2,A0 == 111
//u can only send 4 bits at a time

//helper functions, send data through i2c
void send(uint8_t data, uint8_t instr_data_mode){
    
    uint8_t i2c_write_buf[4];
    i2c_write_buf[0] = (data & 0xF0) | 0b1100 | instr_data_mode;  //data 00000000
    i2c_write_buf[1] = (data & 0xF0) | 0b1000 | instr_data_mode;
    i2c_write_buf[2] = ((data << 4) & 0xF0) | 0b1100 | instr_data_mode; 
    i2c_write_buf[3] = ((data << 4) & 0xF0) | 0b1000 | instr_data_mode;
    //first 4 bits
    //do you mean PCF874_ADDR? instead of PCF8754_ADDR
    i2c_master_write(i2c_write_buf, 0x4, PCF874_ADDR); //set enable (ask id addr should be shifted over one r)
    //i2c_master_write(i2c_write_buf, 0x4, PCF874_ADDR >> 1); //unset enable

    // //last 4 bits
    // i2c_master_write(i2c_write_buf | LCD_enable, 0x4,PCF874_ADDR >> 1); //set enable
    // i2c_master_write(i2c_write_buf, 0x4, (PCF874_ADDR >> 1)); //unset enable
}

void send_instruction(uint8_t instructions){
    send(instructions, 0); //instruction mode
}
void send_data(uint8_t data){
    send(data, 1);
}

void lcd_driver_init() {
    // initialize the HD44780U (the actual display controller that gives functionality)
    //lcd function set
    systick_delay(16);
    send_instruction(0x30);
    systick_delay(5);
    send_instruction(0x30);
    systick_delay(1);
    send_instruction(0x30);
    send_instruction(0x20);

    lcd_clear();
    for(int i = 0; i < 500; i ++ ){}

	return;
}

void lcd_print(char *input){
    while (*input){
        send_data((uint8_t) *input);
        input++;
    }
}

void lcd_set_cursor(uint8_t row, uint8_t col){
    if(row > 1 || col > 15) return; //not in bounds
    //base address for cursor = 0x80 
    //second row = 0x40
    uint8_t offset = 0x0; 

    if (row == 1){ 
        offset = 0x40 + col;
    }
    else offset = col;

    send_instruction(0x80 | offset); 

}

void lcd_clear() {
    send_instruction(0x01); //lcd clear
    // for(int i = 0; i < 2000; i ++ ){} //clearing might take a while
    // delay for 2 seconds 
    systick_delay(2000);
}
