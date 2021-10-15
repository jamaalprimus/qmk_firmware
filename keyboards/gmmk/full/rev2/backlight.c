#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <SN32F260.h>
#include "ch.h"
#include "hal.h"
#include "color.h"
#include "wait.h"
#include "util.h"
#include "matrix.h"
#include "debounce.h"
#include "quantum.h"

#define I2C_SCL A4
#define I2C_SDA A5

#define I2C_SCL_IN readPin(I2C_SCL)
#define I2C_SDA_IN readPin(I2C_SDA)


//#define I2C_SCL_HI setPinInputHigh(I2C_SCL)
#define I2C_SCL_LO do { setPinOutput(I2C_SCL); writePinLow(I2C_SCL); } while (0)
#define I2C_SCL_HIZ do { setPinOutput(I2C_SCL); writePinHigh(I2C_SCL); } while (0)

//#define I2C_SDA_HI setPinInputHigh(I2C_SDA)
#define I2C_SDA_LO do { setPinOutput(I2C_SDA); writePinLow(I2C_SDA); } while (0)
#define I2C_SDA_HIZ setPinInputHigh(I2C_SDA)

#define I2C_DELAY

static uint8_t sel_frame[2] = {0xFF, 0xFF};
static uint8_t sel_frame_idx = 0;

void i2c_init(void)
{    
    // SN_GPIO0->MODE = SN_GPIO0->MODE | (1 << 20) | (1 << 21);
    I2C_SCL_HIZ;
    I2C_SDA_HIZ;
}

static void i2c_process_bit(uint8_t *i2c_tx_byte)
{
    if (*i2c_tx_byte & 0x80)
    {
        I2C_SDA_HIZ;
    }
    else
    {
        I2C_SDA_LO;
    }

    *i2c_tx_byte = *i2c_tx_byte << 1;
    I2C_DELAY;

    I2C_SCL_HIZ;

    I2C_DELAY;

    I2C_SCL_LO;

    I2C_DELAY;
}

static uint8_t i2c_transaction(uint8_t i2c_addr_rw, uint8_t* i2c_data_ptr, uint8_t i2c_byte_ct)
{
    uint8_t txb;
    uint8_t fail = 0;
    
    I2C_SDA_LO;

    I2C_DELAY;

    I2C_SCL_LO;

    I2C_DELAY;

    txb = i2c_addr_rw;
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);

    goto I2C_STATE_READ_ACK;

I2C_STATE_WRITE_BYTE:
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);
    i2c_process_bit(&txb);

    i2c_byte_ct--;
    i2c_data_ptr++;

I2C_STATE_READ_ACK:
    I2C_SDA_HIZ;
    I2C_DELAY;
    I2C_SCL_HIZ;
    I2C_DELAY;

    // check ACK    
    if (!I2C_SDA_IN)
    {
        // Slave did ACK, move on to data
        if (i2c_byte_ct > 0)
        {
            txb = *i2c_data_ptr;

            I2C_SDA_LO;
            I2C_DELAY;
            I2C_SCL_LO;
            I2C_DELAY;

            goto I2C_STATE_WRITE_BYTE;
        }
    }
    else {
        fail = 1;
    }

    I2C_DELAY;

    I2C_SDA_LO;

    I2C_DELAY;

    I2C_SCL_LO;

    I2C_DELAY;

    I2C_SCL_HIZ;

    I2C_DELAY;
    I2C_SDA_HIZ;

    I2C_DELAY;

    return fail;
}

static uint8_t i2c_write_buf(uint8_t devid, uint8_t* data, uint8_t len)
{
    int32_t tries = 10;
    
    while ((tries-- > 0) && i2c_transaction(devid, data, len));
    
    return 0;
}

static void i2c_write_reg(uint8_t devid, uint8_t reg, uint8_t data)
{
    uint8_t i2c_data[2];
    
    i2c_data[0] = reg;
    i2c_data[1] = data;

    i2c_write_buf(devid, i2c_data, 2);
}

static void reset_rgb(int devid)
{
    i2c_write_reg(devid, 0xFD, 0x0B);

    i2c_write_reg(devid, 0x0A, 0x00);
    i2c_write_reg(devid, 0x00, 0x00);
    i2c_write_reg(devid, 0x01, 0x10);
    i2c_write_reg(devid, 0x05, 0x00);
    i2c_write_reg(devid, 0x06, 0x00);
    i2c_write_reg(devid, 0x08, 0x00);
    i2c_write_reg(devid, 0x09, 0x00);
    i2c_write_reg(devid, 0x0B, 0x00);
    i2c_write_reg(devid, 0x0D, 0x0F);
    i2c_write_reg(devid, 0x0E, 0x01);
    i2c_write_reg(devid, 0x14, 68);
    i2c_write_reg(devid, 0x15, 128);
    i2c_write_reg(devid, 0x0F, 153);

    i2c_write_reg(devid, 0xFD, 0);
    for (int32_t i = 0; i < 0x10; i++)
        i2c_write_reg(devid, i, 0xFF);
    /* skip blink control 0x10~0x1F as reset 0 */
    for (int32_t i = 0x20; i < 0xA0; i++)
        i2c_write_reg(devid, i, 0x0);

    i2c_write_reg(devid, 0xFD, 1);
    for (int32_t i = 0; i < 0x10; i++)
        i2c_write_reg(devid, i, 0xFF);
    /* skip blink control 0x10~0x1F as reset 0 */
    for (int32_t i = 0x20; i < 0xA0; i++)
        i2c_write_reg(devid, i, 0x0);

    i2c_write_reg(devid, 0xFD, 0xB);
    i2c_write_reg(devid, 0x0A, 1);
}

#ifdef KEYMAP_ISO
 
/* ISO */

/*
 * led index to RGB address
 */
static const uint8_t g_led_pos[DRIVER_LED_TOTAL] = {
/* 0*/ 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xC0,0xC1,0xC2,0xC3,0xC4,
/*16*/ 0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x9C,0x9D,0x9E,0x9F,0xCA,0x90,0x91,0x92,0x93,0x94,0x95,0x96,
/*37*/ 0x60,0x61,0x62,0x63,0x64,0x65,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x97,0x98,0x60,0x61,0x62,0x63,0x64,0x65,
/*58*/ 0x30,0x31,0x32,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x30,0x31,0x32,
/*74*/ 0x03,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x36,0x37,0x38,0x39,0x3A,
/*91*/ 0x03,0x04,0x05,0x07,0x09,0x0A,0x0B,0x0D,0x0E,0x0F,0x3B,0x3C,0x3D,0x04/* KC_NUBS */
};

/*
 * led index to chip selection table (0: E8, 1: EE)
 */
static const uint8_t g_led_chip[DRIVER_LED_TOTAL] = {
/* 0*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,
/*16*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,
/*37*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,
/*58*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,
/*74*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,
/*91*/    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1/* KC_NUBS */
};

#else

/* ANSI */

/*
 * led index to RGB address
 */
static const uint8_t g_led_pos[DRIVER_LED_TOTAL] = {
/* 0*/ 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xC0,0xC1,0xC2,0xC3,0xC4,
/*16*/ 0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x9C,0x9D,0x9E,0x9F,0xCA,0x90,0x91,0x92,0x93,0x94,0x95,0x96,
/*37*/ 0x60,0x61,0x62,0x63,0x64,0x65,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x97,0x98,0x60,0x61,0x62,0x63,0x64,0x65,
/*58*/ 0x30,0x31,0x32,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x30,0x31,0x32,
/*74*/ 0x03,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x36,0x37,0x38,0x39,0x3A,
/*91*/ 0x03,0x04,0x05,0x07,0x09,0x0A,0x0B,0x0D,0x0E,0x0F,0x3B,0x3C,0x3D
};

/*
 * led index to chip selection table (0: E8, 1: EE)
 */
static const uint8_t g_led_chip[DRIVER_LED_TOTAL] = {
/* 0*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,  0,    0,   0,
/*16*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  0,    0,   0,   0,   0,   0,   0,   0,
/*37*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  0,    0,   0,   0,   0,   0,   0,   0,
/*58*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  0,    0,   0,
/*74*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,  0,    0,   0,   0,
/*91*/    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

#endif

static void set_pwm(uint8_t dev, uint8_t addr, uint8_t value)
{
    /* >=0x80 for frame 2 otherwise frame 1 */
    if (addr >= 0x80) {
        if (sel_frame[sel_frame_idx] != 1) {
            i2c_write_reg(dev, 0xFD, 1);
            sel_frame[sel_frame_idx] = 1;
        }
        addr -= 0x80;
    }
    else if (sel_frame[sel_frame_idx] != 0) {
        i2c_write_reg(dev, 0xFD, 0);
        sel_frame[sel_frame_idx] = 0;
    }

    i2c_write_reg(dev, addr + 0x20, value);
}

void _set_color(int index, uint8_t r, uint8_t g, uint8_t b)
{   
    uint8_t dev;
    int l = g_led_pos[index];

    if (g_led_chip[index]) {
        dev = 0xEE;
        sel_frame_idx = 1;
    }
    else {
        dev = 0xE8;
        sel_frame_idx = 0;
    }

    set_pwm(dev, l, r);
    set_pwm(dev, l + 0x10, g);
    set_pwm(dev, l + 0x20, b);
}

void _read_color(int index, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = 0;
    *g = 0;
    *b = 0;
}

void process_backlight(uint8_t devid, volatile LED_TYPE *states)
{
    static unsigned char state = 0;

    switch (state)
    {
        case 0: /* init RGB chips */
            i2c_init();
            reset_rgb(0xE8);
            reset_rgb(0xEE);

            state = 1;
            break;
    }
}