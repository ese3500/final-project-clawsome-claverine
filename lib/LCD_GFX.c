/*
 * LCD_GFX.c
 *
 * Created: 9/20/2021 6:54:25 PM
 *  Author: You
 */ 

#include "LCD_GFX.h"
#include "ST7735.h"

/******************************************************************************
* Local Functions
******************************************************************************/
void plotLineLow(short x0, short y0, short x1, short y1, uint16_t color)
{
	short dx = x1-x0;
	short dy = y1-y0;
	short yi = 1;
	if(dy<0)
	{
		yi = -1;
		dy = -dy;
	}
	short D = (2*dy)-dx;
	short y = y0;

	for(short x = x0;x<x1;++x)
	{
		LCD_drawPixel(x,y,color);
		if(D>0)
		{
			y += yi;
			D += (2*(dy-dx));
		}else
		{
			D += 2*dy;
		}
	}
}

void plotLineHigh(short x0, short y0, short x1, short y1, uint16_t color)
{
	short dx = x1-x0;
	short dy = y1-y0;
	short xi = 1;
	if(dx<0)
	{
		xi = -1;
		dx = -dx;
	}
	short D = (2*dx)-dy;
	short x = x0;

	for(short y = y0;y<y1;++y)
	{
		LCD_drawPixel(x,y,color);
		if(D>0)
		{
			x += xi;
			D += (2*(dx-dy));
		}else
		{
			D += 2*dx;
		}
	}
}

/******************************************************************************
* Global Functions
******************************************************************************/

/**************************************************************************//**
* @fn			uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
* @brief		Convert RGB888 value to RGB565 16-bit color data
* @note
*****************************************************************************/
uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
	return ((((31*(red+4))/255)<<11) | (((63*(green+2))/255)<<5) | ((31*(blue+4))/255));
}

/**************************************************************************//**
* @fn			void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color)
* @brief		Draw a single pixel of 16-bit rgb565 color to the x & y coordinate
* @note
*****************************************************************************/
void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color) {
	LCD_setAddr(x,y,x,y);
	SPI_ControllerTx_16bit(color);
}

/**************************************************************************//**
* @fn			void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor)
* @brief		Draw a character starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor){
	uint16_t row = character - 0x20;		//Determine row of ASCII table starting at space
	int i, j;
	if ((LCD_WIDTH-x>7)&&(LCD_HEIGHT-y>7)){
		for(i=0;i<5;i++){
			uint8_t pixels = ASCII[row][i]; //Go through the list of pixels
			for(j=0;j<8;j++){
				if ((pixels>>j)&1==1){
					LCD_drawPixel(x+i,y+j,fColor);
				}
				else {
					LCD_drawPixel(x+i,y+j,bColor);
				}
			}
		}
	}
}


/******************************************************************************
* LAB 4 TO DO. COMPLETE THE FUNCTIONS BELOW.
* You are free to create and add any additional files, libraries, and/or
*  helper function. All code must be authentically yours.
******************************************************************************/

/**************************************************************************//**
* @fn			void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
* @brief		Draw a colored circle of set radius at coordinates
* @note
*****************************************************************************/
void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
{
	unsigned short radius_squared = radius*radius;
	for(short x = x0-radius;x<x0+radius;++x)
	{
		short with_back_forth = sqrt(radius_squared-(x-x0)*(x-x0));
		LCD_setAddr(x,y0-with_back_forth,x,y0+with_back_forth);
		clear(LCD_PORT, LCD_TFT_CS);
		for(uint8_t y=0;y<2*with_back_forth;++y)
		{
			SPI_ControllerTx_16bit_stream(color);
		}
		set(LCD_PORT, LCD_TFT_CS);
	}
	return;
}

/**************************************************************************//**
* @fn			void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c)
* @brief		Draw a line from and to a point with a color
* @note
*****************************************************************************/
void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t color)
{
	if (abs(y1-y0) < abs(x1-x0))
	{
		if(x0>x1)
		{
			plotLineLow(x1,y1,x0,y0,color);
		}else
		{
			plotLineLow(x0,y0,x1,y1,color);
		}
	}else
	{
		if(y0>y1)
		{
			plotLineHigh(x1,y1,x0,y0,color);
		}else
		{
			plotLineHigh(x0,y0,x1,y1,color);
		}
	}
	// As given in the description of the lab this was copied from the pseudocode from wikipedia
}



/**************************************************************************//**
* @fn			void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
* @brief		Draw a colored block at coordinates
* @note
*****************************************************************************/
void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
{
	if(x1<x0)
	{
		uint8_t temp = x1;
		x1 = x0;
		x0 = temp;
	}
	if(y1<y0)
	{
		uint8_t temp = y1;
		y1 = y0;
		y0 = temp;
	}
	LCD_setAddr(x0,y0,x1,y1);
	clear(LCD_PORT, LCD_TFT_CS);
	for(uint8_t y=y0;y<y1;++y)
	{
		for(uint8_t x=x0;x<=x1;++x)
		{
			SPI_ControllerTx_16bit_stream(color);
		}
	}
	set(LCD_PORT, LCD_TFT_CS);
}

/**************************************************************************//**
* @fn			void LCD_setScreen(uint16_t color)
* @brief		Draw the entire screen to a color
* @note
*****************************************************************************/
void LCD_setScreen(uint16_t color) 
{
// 	LCD_setAddr(0,0,162,132);
// 	SPI_ControllerTx_16bit(BLUE);
// 	_delay_ms(1000);
	LCD_setAddr(0,0,161,131);
	clear(LCD_PORT, LCD_TFT_CS);
	for(uint8_t y=0;y<132;++y)
	{
		for(uint8_t x=0;x<162;++x)
		{
			SPI_ControllerTx_16bit_stream(color);
		}
	}
	set(LCD_PORT, LCD_TFT_CS);
}

/**************************************************************************//**
* @fn			void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
* @brief		Draw a string starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
{	
	for(uint8_t i = 0;*(str+i);++i)
	{
		LCD_drawChar(x+i*5,y, *(str+i), fg, bg);
	}
}