#ifndef SED1520_H_
#define SED1520_H_

// define to disable busy-delays (useful for simulation)
//// #define LCD_SIM (1)
// define to enable additional debugging functions
//// #define LCD_DEBUG (1)

#include <stdint.h>

typedef struct IMG_DEF 
 { unsigned char width_in_pixels;      /* Image width */
   unsigned char height_in_pixels;     /* Image height*/
   const unsigned char *char_table;    /* Image table start address in memory  */
 } IMG_DEF;


#define DECL_BMP(width,height,imgdata)	IMG_DEF bmp_##imgdata={width,height,imgdata}


/* draw modes */
#define LCD_MODE_CLEAR     0
#define LCD_MODE_SET       1
#define LCD_MODE_XOR       2
#define LCD_MODE_INVERT		3

/* command function equates for SED1520 LCD Display Controller */
#define LCD_DISP_OFF       0xAE	/* turn LCD panel OFF */
#define LCD_DISP_ON        0xAF	/* turn LCD panel ON */
#define LCD_SET_LINE       0xC0	/* set line for COM0 (4 lsbs = ST3:ST2:ST1:ST0) */
#define LCD_SET_PAGE       0xB8	/* set page address (2 lsbs = P1:P0) */
#define LCD_SET_COL        0x00	/* set column address (6 lsbs = Y4:Y4:Y3:Y2:Y1:Y0) */
#define LCD_SET_ADC_NOR    0xA0	/* ADC set for normal direction */
#define LCD_SET_ADC_REV    0xA1	/* ADC set for reverse direction */
#define LCD_STATIC_OFF     0xA4	/* normal drive */
#define LCD_STATIC_ON      0xA5	/* static drive (power save) */
#define LCD_DUTY_16        0xA8	/* driving duty 1/16 */
#define LCD_DUTY_32        0xA9	/* driving duty 1/32 */
#define LCD_SET_MODIFY     0xE0	/* start read-modify-write mode */
#define LCD_CLR_MODIFY     0xEE	/* end read-modify-write mode */
#define LCD_RESET          0xE2	/* soft reset command */

/* LCD screen and bitmap image array consants */
#define LCD_X_BYTES		122
#define LCD_Y_BYTES		4
#define SCRN_LEFT		0
#define SCRN_TOP		0
#define SCRN_RIGHT		121
#define SCRN_BOTTOM		31



#define FONT_BASE_ADDR		0x08040000

#define FONT_ASC0612_SIZE	0x480
#define FONT_HZ1212_SIZE	0x294F0
#define FONT_ASC1224_SIZE	0x1200
#define FONT_HZ2424_SIZE	0x7bed0
#define FONT_ASC0612_ADDR	(FONT_BASE_ADDR)
#define FONT_HZ1212_ADDR	(FONT_BASE_ADDR+FONT_ASC0612_SIZE)
#define FONT_ASC1224_ADDR	(FONT_BASE_ADDR+FONT_ASC0612_SIZE+FONT_HZ1212_SIZE)
#define FONT_HZ2424_ADDR	(FONT_BASE_ADDR+FONT_ASC0612_SIZE+FONT_HZ1212_SIZE+FONT_ASC1224_SIZE)



/* SED1520 is used with reverse direction (ADC_REV). 
   This value is the address of the leftmost column: */
#define LCD_STARTCOL_REVERSE	19

/*蜂鸣器和打印3.3v移到此处*/
void ControlBitShift( unsigned char data );


void lcd_init(void);
void lcd_fill( const unsigned char pattern );
void lcd_update(const unsigned char top, const unsigned char bottom);
void lcd_asc0608( char left, char top, char *p,const char mode );
void lcd_text12( char left, char top, char *pinfo, char len, const char mode );

void lcd_bitmap(const uint8_t left, const uint8_t top, const struct IMG_DEF *img_ptr, const uint8_t mode);
void lcd_clear( const unsigned char top, const unsigned char bottom );

void lcd_hline(uint8_t from,uint8_t to ,uint8_t line);
void lcd_vline(uint8_t col,uint8_t from,uint8_t to);
void lcd_fill_1(uint8_t left,uint8_t top,uint8_t right,uint8_t bottom,uint8_t pattern);

/*蜂鸣器使用*/
void ControlBitShift( unsigned char data );



#endif
