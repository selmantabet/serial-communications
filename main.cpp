/* Selman Tabet (@selmantabet - https://selman.io/) - UIN 724009859
Assignment 5

This program consists of two routines, the first routine allows the user to 
input values of up to 3 digits long through a terminal using an SPI connection.
The values would then be displayed through the base board's SPI LEDs.
The second routine is essentially a 255-second timer that displays the time
elapsed through said SPI LEDs. The two routines are alternated upon button
presses by the user.

Developed using the Mbed IDE. Tested on an EA LPC4088 QuickStart Board. */

#include "mbed.h"
#include <stdio.h>
#include <ctype.h>

DigitalOut chip_select(p30); //Chip select
Serial pc(USBTX, USBRX);
SPI shifter(p5, p6, p7); //MOSI, MISO, SCLK
InterruptIn button(p23);
const int size = 3; //Up to three digits, can be easily changed.
char buffer[size]; //[ digit1, digit2, digit3 ]
int index = 0;
int number;
volatile bool break_flag = false; //Tripped on button press.

void isr(){ //Trip the flag (flip to true).
    break_flag = true;
}
    
void reset(){ //Reset index then clear the buffer and number variable.
    index = 0;
    for (int i = 0; i < size; i++) buffer[i] = NULL; //Clear buffer.
    number = NULL;
}

void spi_printer(){
    char x;
    pc.printf("Control the SPI LEDS from a PC. \r\n\n");
    pc.printf("Enter any number between 0-255: \r\n");
    while(true){
        while(index <= size){
            if(index == size) pc.printf("\nNow press Enter.");
            while(!pc.readable()) //Break polling while on keystroke standby.
                if(break_flag){ //Flag is tripped. Leave.
                    reset();
                    pc.printf("\n");
                    return; //Exit program.
                }
            x = pc.getc();
            if (isdigit(x)){
                if (index == size){ //At this point the buffer is full.
                    pc.printf("\nYou should have pressed Enter -.-\n");
                    pc.printf("Ignored the last integer input.\n");
                    number = atoi(buffer);
                    pc.printf("Using the value %d \n", number);
                    break;
                }
                else { //Append to the buffer
                    buffer[index] = x; index++; pc.printf("%d", atoi(&x));
                }
            }
            else if ((x == '\r') && (index == 0)){
                //Assume the value zero if  the user presses Enter first.
                number = 0; pc.printf("0\r\n"); break;
            }
            else if ((x == '\r') || (x == '\n')){ //Value complete.
                //Handle buffer and proceed to send the value via SPI.
                number = atoi(buffer); pc.printf("\r\n"); break;
            }
            else if (x == '\b'){ //In case backspace is recognized.
                if(index <=0){
                    //Should not backspace any further.
                }
                else { //Delete everything and try again.
                    //Printing "\b \b" instead did not work for some reason.
                    reset();
                    pc.printf("Entry deleted. Re-enter the value.\n");
                }   
            }
            else{ //Any other unexpected entry.
                pc.printf("\nError: Enter integer values between 0 and 255.\n");
                reset();
                pc.printf("Please try again.\n");
            }
        }
        if (number > 255){
            pc.printf("Out of bound value.\n");
            pc.printf("Please stick to values between 0 and 255.\n");
            reset();
        }
        else {
            chip_select = 0; shifter.write(number); chip_select = 1;
            reset();
        }
    }
}

void spi_255_timer(){
    int counter = 0;
    pc.printf("Initialized 255-second counter.\n\n");
    while(true){
        while(counter <= 255){
            if(break_flag){ //Flag is tripped. Leave.
                //Turn off the display.
                chip_select = 0; shifter.write(0); chip_select = 1;
                return; //Exit program.
            }
            //Display the parsed value.
            chip_select = 0; shifter.write(counter); chip_select = 1;
            wait(1.0f); counter++; //Tick!
        }
        counter = 0;
    }
}

int main(){
    button.mode(PullUp);
    chip_select = 0; //Select the device by setting chip select low
    shifter.write(0); //Clear LEDs
    chip_select = 1; //Deselect the device
    
    button.fall(&isr);
    
    while(1){
        spi_printer(); break_flag = false;
        spi_255_timer(); break_flag = false;   
    }
}