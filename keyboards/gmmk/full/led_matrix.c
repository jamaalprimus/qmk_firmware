#include <string.h>
#include "rgb.h"
#include "rgb_matrix.h"
#include "rgb_matrix_types.h"
#include "color.h"

extern void _set_color(int index, uint8_t r, uint8_t g, uint8_t b);

void process_backlight(uint8_t devid, volatile LED_TYPE* states);

static void flush(void)
{
    process_backlight(0, NULL);
}

void set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < DRIVER_LED_TOTAL)
    {
        _set_color(index, r, g, b);
    }   
}

static void set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < DRIVER_LED_TOTAL; i++)
        set_color(i, r, g, b);
}

void init(void) {
}

const rgb_matrix_driver_t rgb_matrix_driver = {
    .init          = init,
    .flush         = flush,
    .set_color     = set_color,
    .set_color_all = set_color_all,
};

int rand(void)
{
   // static unsigned int z4;
   unsigned int z1, z2, z3, z4;
   int r;
   unsigned int b;
   
   z1 = timer_read32();
   z2 = 12345;
   z3 = 12345;
   z4 = z1 << 8;
   
   b  = ((z1 << 6) ^ z1) >> 13;
   z1 = ((z1 & 4294967294U) << 18) ^ b;
   b  = ((z2 << 2) ^ z2) >> 27; 
   z2 = ((z2 & 4294967288U) << 2) ^ b;
   b  = ((z3 << 13) ^ z3) >> 21;
   z3 = ((z3 & 4294967280U) << 7) ^ b;
   b  = ((z4 << 3) ^ z4) >> 12;
   z4 = ((z4 & 4294967168U) << 13) ^ b;
   
   r = z1 ^ z2 ^ z3 ^ z4;
   // z4 = r;
   
   return r;    
}