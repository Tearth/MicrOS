#include "mode_06H.h"
#include "../registerFunctions.h"
#include "../../../../Memory/Manager/Heap/heap.h"
#include "../../../DAL/VideoCard/videocard.h"

//REGISTER VALUES
unsigned char g_640x200x2[] =
	{
	/* MISC */
		0x63, 
	/* SEQ */
		0x03, 0x01, 0x01, 0x00, 0x06, 
	/* CRTC */
		0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
		0x00, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x9C, 0x8E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xC2,
		0xFF, 
	/* GC */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x00,
		0xFF, 
	/* AC */
		0x00, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17,
	    0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17,
	    0x01, 0x00, 0x01, 0x00, 0x00, 
	};

//PALETTE
unsigned char palette06H[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x2A, 0x00, 0x00, 0x2A, 0x2A, 0x2A, 0x00, 0x00, 0x2A, 
    0x00, 0x2A, 0x2A, 0x15, 0x00, 0x2A, 0x2A, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x2A, 
    0x00, 0x00, 0x2A, 0x2A, 0x2A, 0x00, 0x00, 0x2A, 0x00, 0x2A, 0x2A, 0x15, 0x00, 0x2A, 0x2A, 0x2A, 
    0x15, 0x15, 0x15, 0x15, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x15, 0x3F, 
    0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x15, 0x15, 0x15, 0x15, 0x3F, 0x15, 0x3F, 
    0x15, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x15, 0x3F, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x3F, 0x3F, 0x3F, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x2A, 0x00, 0x00, 0x2A, 0x2A, 0x2A, 0x00, 0x00, 0x2A, 
    0x00, 0x2A, 0x2A, 0x15, 0x00, 0x2A, 0x2A, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x2A, 
    0x00, 0x00, 0x2A, 0x2A, 0x2A, 0x00, 0x00, 0x2A, 0x00, 0x2A, 0x2A, 0x15, 0x00, 0x2A, 0x2A, 0x2A, 
    0x15, 0x15, 0x15, 0x15, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x15, 0x3F, 
    0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x15, 0x15, 0x15, 0x15, 0x3F, 0x15, 0x3F, 
    0x15, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x15, 0x3F, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x3F, 0x3F, 0x3F, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//DOUBLE BUFFER POINTER;

unsigned char *MODE06H_BUFFER[] = {NULL, NULL};
unsigned char bufferTurnedOn06H = 0;

int8_t setMode06H()
{
    writeRegisters(g_640x200x2);
    set_vga_palette(palette06H);
    setTurnOnBufferFunc(&turnOffBuffer06H);
    setTurnOffBufferFunc(&turnOffBuffer06H);
    setIsBufferOnFunc(&isBufferOn06H);
    setSwapBuffersFunc(&swapBuffers06H);
    setDrawPixelFunc(&drawPixel06H);
    setDrawLineFunc(&drawLine06H);
    setDrawCircleFunc(&drawCircle06H);
    setDrawRectangleFunc(&drawRectangle06H);
    setClearScreenFunc(&clearScreen06H);
    return 0x06;
}

int8_t turnOnBuffer06H()
{
    if(bufferTurnedOn06H) return -1;
    for(int i = 1; i >= 0; i--)
    {
        MODE06H_BUFFER[i] = heap_kernel_alloc(MODE06H_HEIGHT * MODE06H_WIDTH / 16, 0);
        if(MODE06H_BUFFER[i] == NULL)
        {
            for(int j = 1; j >= i; j--)
            {
                heap_kernel_dealloc(MODE06H_BUFFER[j]);
                MODE06H_BUFFER[j] = NULL;
            }
            return -1;
        }
    }
    setDrawPixelFunc(&drawPixel06HBuffered);
    setDrawLineFunc(&drawLine06HBuffered);
    setDrawCircleFunc(&drawCircle06HBuffered);
    setDrawRectangleFunc(&drawRectangle06HBuffered);
    setClearScreenFunc(&clearScreen06HBuffered);
    bufferTurnedOn06H = 1;
    return 0;
}

int8_t turnOffBuffer06H()
{
    if(!bufferTurnedOn06H) return -1;
    for(int i = 1; i >= 0; i--)
    {
        heap_kernel_dealloc(MODE06H_BUFFER[i]);
        MODE06H_BUFFER[i] = NULL;
    }
    setDrawPixelFunc(&drawPixel06H);
    setDrawLineFunc(&drawLine06H);
    setDrawCircleFunc(&drawCircle06H);
    setDrawRectangleFunc(&drawRectangle06H);
    setClearScreenFunc(&clearScreen06H);
    bufferTurnedOn06H = 0;
    return 0;
}

uint8_t isBufferOn06H()
{
    return bufferTurnedOn06H;
}

int8_t swapBuffers06H()
{
    if(!bufferTurnedOn06H) return -1;
    memcpy(VGA_VRAM_2, MODE06H_BUFFER[0], MODE06H_WIDTH * MODE06H_HEIGHT / 16);
    memcpy(VGA_VRAM_2 + 0x2000, MODE06H_BUFFER[1], MODE06H_WIDTH * MODE06H_HEIGHT / 16);
    //memcpy(VGA_VRAM, MODE13H_BUFFER, MODE13H_HEIGHT * MODE13H_WIDTH);
    return 0;
}

int8_t drawPixel06H(uint8_t color, uint16_t x, uint16_t y)
{
    if((x>=MODE06H_WIDTH) || (y >=MODE06H_HEIGHT))
        return -1;
    unsigned char *fb = (unsigned char *) VGA_VRAM_2;
    unsigned int offset = (y/2 * MODE06H_WIDTH + x)/8;
	unsigned bit_no = x % 8;
	bit_write(fb[offset + (y%2 ? 0x2000 : 0)], (1<<(7 - bit_no)), (color & 0x1));
    return 0;
}

int8_t drawLine06H(uint8_t color, uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by)
{
    return 0;
}

int8_t drawCircle06H(uint8_t color, uint16_t x, uint16_t y, uint16_t radius) 
{
    return 0;
}

int8_t drawRectangle06H(uint8_t color, uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by)
{
    return 0;
}
int8_t clearScreen06H()
{
    return 0;
}

int8_t drawPixel06HBuffered(uint8_t color, uint16_t x, uint16_t y)
{
    if((!bufferTurnedOn06H) || (x>=MODE06H_WIDTH) || (y >=MODE06H_HEIGHT))
        return -1;
    unsigned int offset = (y * MODE06H_WIDTH + x)/8;
	unsigned bit_no = x % 8;
    bit_write(MODE06H_BUFFER[y%2][offset], (1<<(7 - bit_no)), (color & 0x1));
    return 0;
}

int8_t drawLine06HBuffered(uint8_t color, uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by)
{
    return 0;
}
int8_t drawCircle06HBuffered(uint8_t color, uint16_t x, uint16_t y, uint16_t radius)
{
    return 0;
}
int8_t drawRectangle06HBuffered(uint8_t color, uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by)
{
    return 0;
}
int8_t clearScreen06HBuffered()
{
    return 0;
}