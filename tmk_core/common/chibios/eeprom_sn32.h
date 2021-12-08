/*
 * This software is experimental and a work in progress.
 * Under no circumstances should these files be used in relation to any critical system(s).
 * Use of these files is at your own risk.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This files are free to use from http://engsta.com/stm32-flash-memory-eeprom-emulator/ by
 * Artur F.
 *
 * Modifications for QMK and SONIX chips by smplman and dexter93
 *
 * This library assumes 8-bit data locations. To add a new MCU, please provide the flash
 * page size and the total flash size in Kb. The number of available pages must be a multiple
 * of 2. Only half of the pages account for the total EEPROM size.
 * This library also assumes that the pages are not used by the firmware.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef __EEPROM_H
#define __EEPROM_H

#include "ch.h"
#include "hal.h"
#include "Flash.h"

// HACK ALERT. This definition may not match your processor
// To Do. Work out correct value for EEPROM_PAGE_SIZE on the STM32F103CT6 etc
#if defined(EEPROM_EMU_SN32F240B)
#    define MCU_SN32F240B
#elif defined(EEPROM_EMU_SN32F260)
#    define MCU_SN32F260
#else
#    error "not implemented."
#endif

#ifndef EEPROM_PAGE_SIZE
#    if defined(MCU_SN32F240B)
#        define FEE_PAGE_SIZE       (uint16_t)0x0040     // Page size = 64bytes
#        define FEE_TOTAL_PAGES     1024     // How many pages are available
#        define FEE_DENSITY_PAGES   23     // How many pages are used as EEPROM
#    elif defined(MCU_SN32F260)
#        define FEE_PAGE_SIZE       (uint16_t)0x0040     // Page size = 64bytes
#        define FEE_TOTAL_PAGES     480     // How many pages are available
#        define FEE_DENSITY_PAGES   23      // How many pages are used as EEPROM
#    else
#        error "No MCU type specified. Add something like -DMCU_SN32F240B to your compiler arguments (probably in a Makefile)."
#    endif
#endif

#ifndef EEPROM_START_ADDRESS
#    if defined(MCU_SN32F240B)
#        define FEE_MCU_FLASH_SIZE 64  // User ROM Size in Kb
#    elif defined(MCU_SN32F260)
#        define FEE_MCU_FLASH_SIZE 30  // User ROM Size in Kb
#    else
#        error "No MCU type specified. Add something like -DMCU_SN32F240B to your compiler arguments (probably in a Makefile)."
#    endif
#endif

// DONT CHANGE
// Choose location for the first EEPROM Page address on the top of flash
#define FEE_PAGE_BASE_ADDRESS ((uint32_t)(FEE_PAGE_SIZE * FEE_TOTAL_PAGES - ((FEE_DENSITY_PAGES + 1) * FEE_PAGE_SIZE))) // Last page used is 1022, don't mess with the last
#define FEE_DENSITY_BYTES (FEE_PAGE_SIZE * FEE_DENSITY_PAGES - 1)
#define FEE_LAST_PAGE_ADDRESS (FEE_PAGE_BASE_ADDRESS + (FEE_PAGE_SIZE * FEE_DENSITY_PAGES))
#define FEE_EMPTY_WORD ((uint16_t)0xFFFF)
#define FEE_ADDR_OFFSET(Address) (Address)

// Use this function to initialize the functionality
uint16_t EEPROM_Init(void);
void     EEPROM_Erase(void);
uint16_t EEPROM_WriteDataByte(uint16_t Address, uint8_t DataByte);
uint8_t  EEPROM_ReadDataByte(uint16_t Address);

#endif /* __EEPROM_H */