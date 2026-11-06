/* Copyright 2019 coseyfannitutti
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "v15d.h"
#include "bitmaps.h"

oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    return OLED_ROTATION_180;
}

bool encoder_update_kb(uint8_t index, bool clockwise) {
    if (clockwise) {
        tap_code(KC_VOLU);
    } else {
        tap_code(KC_VOLD);
    }
    return false;
}

#ifdef OLED_ENABLE

/**
 * draws a bitmap to the display
 * specify position and dimensions of bitmap
 *
 * create the bitmaps with https://javl.github.io/image2cpp/
 * (with Draw Mode set to vertical - 1 bit per pixel)
 */
void draw_bitmap_P(const char *bmp, const int w, const int h, const int x, const int y) {
    for (int i = 0; i < h / 8; i++) {
        for (int j = 0; j < w; j++) {
            oled_write_raw_byte(pgm_read_byte(&(bmp[(w * i) + j])), (128 * (i + y)) + j + x);
        }
    }
}

/**
 * clear the area where you once drew a bitmap.
 * specify position and dimensions af area
 */
void clear_area(const int w, const int h, const int x, const int y) {
    for (int i = 0; i < h / 8; i++) {
        for (int j = 0; j < w; j++) {
            oled_write_raw_byte(0x00, (128 * (i + y)) + j + x);
        }
    }
}

uint32_t last_input = 0;
int      key_event  = 0; // 0: released, 1: pressed

/**
 * gets last keyboard input
 */
void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    last_input = timer_read32();
    if (record->event.pressed) {
        key_event = 1;
    } else {
        key_event = 0;
    }
}

#    define DISPLAY_TIMEOUT 10000
#    define IDLE_FRAME_TIME 150

/**
 * do oled things
 */
bool oled_task_user(void) {
    uint32_t now = timer_read32();
    //* test if last keyboard input was not too long ago
    // otherwise display will be turned off
    if (last_input + DISPLAY_TIMEOUT > now) {
        //* Host Keyboard LED Status
        // show if locked key are activated
        led_t led_state = host_keyboard_led_state();

        if (led_state.caps_lock) {
            draw_bitmap_P(IMG_CAP, IMG_CAP_SIZE[0], IMG_CAP_SIZE[1], 118, 0);
        } else {
            clear_area(IMG_CAP_SIZE[0], IMG_CAP_SIZE[1], 118, 0);
        }

        //* BONGO CAT

        if (key_event == 0) {
            // if last key event was releasing, then show idle animation

            static uint32_t last_update = 0;

            // limit framerate
            if (last_update + IDLE_FRAME_TIME < now) {
                last_update = now;

                static int frame = 0;
                draw_bitmap_P(IMG_BONGO_IDLE[frame], IMG_BONGO_IDLE_SIZE[0], IMG_BONGO_IDLE_SIZE[1], 0, 0);

                frame++;
                if (frame >= IMG_BONGO_IDLE_FRAMES) {
                    frame = 0;
                }
            }
        } else if (key_event == 1) {
            // if last key event was pressing down, then show random tap image
            key_event        = 2; // get rid of pressed status, to avoid triggering this section over and over
            int chosen_frame = rand() % IMG_BONGO_TAP_FRAMES;
            draw_bitmap_P(IMG_BONGO_TAP[chosen_frame], IMG_BONGO_TAP_SIZE[0], IMG_BONGO_TAP_SIZE[1], 0, 0);
        }

        // char s[65];
        // sprintf(s, "%ld", last_input);
        // oled_set_cursor(0, 0);
        // oled_write(s, false);

    } else {
        // no input for some time, clear display to avoid burn in
        oled_clear();
    }

    // render the changes
    oled_render_dirty(true);
    return false;
}

#endif
