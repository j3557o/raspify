/*
 * lcd.c
 * 
 * Copyright 2012 Jesper Stockenstrand <jesper@stockenstrand.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h> 

//Define pin nr for EN and RS
#define LCD_RS 0x20
#define LCD_EN 0x80

//Define LCD settings
#define CMD_SCMD 0x04   //Set Cursor Move Direction:
#define SCMD_ID  0x02   //- Increment the Cursor After Each Byte Written to Display if Set
#define SCMD_S   0x01   //- Shift Display when Byte Written to Display

#define CMD_EDC  0x08   //Enable Display/Cursor
#define EDC_D    0x04   //- Turn Display On
#define EDC_C    0x02   //- Turn Cursor On
#define EDC_B    0x01   //- Cursor Blink On

#define CMD_MCSD 0x10	//Move Cursor/Shift Display
#define MCSD_SC  0x08   //- Display Shift On(1)/Off(0)
#define MCSD_RL  0x04   //- Direction of Shift Right(1)/Left(0)

#define CMD_SIL  0x20   //Set Interface Length
#define SIL_DL   0x10   //- Set Data Interface Length 8
#define SIL_N    0x08   //- Number of Display Lines 2(=4)
#define SIL_F    0x04   //- Character Font 5x10

#define CMD_MCD  0x80   //Move Cursor to Display Address
#define CMD_CAH	 0x01	//Clear and Home
#define CMD_HME  0x02	//Move home

int fd;
char *fileName = "/dev/i2c-0";
int address = 0x20;

void lcd_reset();
void LCD_setup();
void lcd_init();
void PutBitsOnPins(char bits);
void write_nibbles(int bits);
void write_lcd(int bits);
void lcd_string(char *s);
void write_char(char letter);
void lcd_line(char s[20]);
void lcd_clear();

void LCD_setup() {
  if ((fd = open(fileName, O_RDWR)) < 0) {
    printf("Failed to open the i2c bus\n");
    exit(1);
  }
	
  if (ioctl(fd,I2C_SLAVE,address) < 0) {
    printf("Failed to acquire bus access and/or talk to slave.\n");
    exit(1);
  }
  lcd_reset();
  lcd_init();
}

void write_lcd(int bits) {
  PutBitsOnPins(bits+LCD_EN);
  PutBitsOnPins(bits);
  usleep(500);
}

void lcd_string(char *s) {
  int i;
  for(i = 0; i<strlen(s); i++) {
    write_char(s[i]);
  }
}

void lcd_line(char *s) {
  int i;
  for(i = 0; i<20; i++) {
    if((i+1)>strlen(s)) {
      write_char(' ');
    }
    else {
      write_char(s[i]);
    }
  }
}

void PutBitsOnPins(char bits) {
  char buf[1];
  buf[0] = bits;
  if (write(fd,buf,1) != 1) {
    printf("Failed to write to the i2c bus.\n");
  }
}

void lcd_clear() {
  write_nibbles(CMD_CAH);
}

void lcd_reset() {
  PutBitsOnPins(0xFF);
  usleep(5000);
  PutBitsOnPins(0x03+LCD_EN);
  PutBitsOnPins(0x03);
  usleep(5000);
  PutBitsOnPins(0x03+LCD_EN);
  PutBitsOnPins(0x03);
  usleep(500);
  PutBitsOnPins(0x03+LCD_EN);
  PutBitsOnPins(0x03);
  usleep(500);
  PutBitsOnPins(0x02+LCD_EN);
  PutBitsOnPins(0x02);
  usleep(500);
}

void lcd_init() {
  write_nibbles(CMD_SIL|SIL_N);
  write_nibbles(CMD_EDC);
  write_nibbles(CMD_CAH);
  write_nibbles(CMD_SCMD|SCMD_ID);
  write_nibbles(CMD_EDC|EDC_D);
}

void write_nibbles(int bits) {
  write_lcd((bits >> 4) & 0x0F);
  write_lcd(bits & 0x0F);
  usleep(500);
}

void write_char(char letter) {
  write_lcd((((int)letter >> 4) & 0x0F)|LCD_RS);
  write_lcd(((int)letter & 0x0F)|LCD_RS);
}

