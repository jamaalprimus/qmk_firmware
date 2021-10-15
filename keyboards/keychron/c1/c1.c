/*
Copyright 2021 Dimitris Mantzouranis

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "c1.h"

// Manage Windows and Mac LEDs
// - Show status of mode switch
// - Turn LEDs off durring USB suspend
bool mode_leds_show = true;
bool mode_leds_windows;

static void mode_leds_update(void){
    writePin(LED_WIN_PIN, mode_leds_show && mode_leds_windows);
    writePin(LED_MAC_PIN, mode_leds_show && !mode_leds_windows);
}

void dip_switch_update_user(uint8_t index, bool active){
    if(index == 0) {
        if(active) { // Mac mode
            layer_move(2);
        } else { // Windows mode
            layer_move(0);
        }

        // Update mode and update leds
        mode_leds_windows = !active;
        mode_leds_update();
    }
}

void keyboard_pre_init_user(void) {
    // Setup Win & Mac LED Pins as output
    setPinOutput(LED_WIN_PIN);
    setPinOutput(LED_MAC_PIN);
}

void suspend_power_down_user(void) {
    // Turn leds off
    mode_leds_show = false;
    mode_leds_update();
}

void suspend_wakeup_init_user(void) {
    mode_leds_show = true;
    mode_leds_update();
}
