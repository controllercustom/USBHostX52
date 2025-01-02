/*
MIT License

Copyright (c) 2024 controllercustom@myyahoo.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
typedef struct __attribute__((packed)) {
    uint32_t x:11;      // 0..1024..2047
    uint32_t y:11;      // 0..1024..2047
    uint32_t twist:10;  // 0..512..1023
    uint8_t throttle;
    uint8_t throttle_small_rotary;
    uint8_t throttle_large_rotary;
    uint8_t throttle_slider;

    uint32_t trigger1:1;
    uint32_t fire:1;
    uint32_t button_a:1;
    uint32_t button_b:1;

    uint32_t button_c:1;
    uint32_t pinky:1;
    uint32_t button_d:1;
    uint32_t button_e:1;

    uint32_t button_t1_up:1;
    uint32_t button_t1_down:1;
    uint32_t button_t2_up:1;
    uint32_t button_t2_down:1;

    uint32_t button_t3_up:1;
    uint32_t button_t3_down:1;
    uint32_t trigger2:1;
    uint32_t top_rotary_north:1;

    uint32_t top_rotary_east:1;
    uint32_t top_rotary_south:1;
    uint32_t top_rotary_west:1;
    uint32_t throttle_rotary_north:1;

    uint32_t throttle_rotary_east:1;
    uint32_t throttle_rotary_south:1;
    uint32_t throttle_rotary_west:1;
    uint32_t top_rotary_green:1;

    uint32_t top_rotary_yellow:1;
    uint32_t top_rotary_red:1;
    uint32_t button_function:1;
    uint32_t button_start:1;

    uint32_t button_reset:1;
    uint32_t button_i:1;
    uint32_t button_mouse:1;
    uint32_t button_wheel:1;

    uint16_t throttle_wheel_down:1;
    uint16_t throttle_wheel_up:1;
    uint16_t filler:2;
    uint16_t top_dpad:4;  // 0..8, 0=center, 1=N, ...
    uint16_t mouse_y:4;
    uint16_t mouse_x:4;
} X52Report_t;

typedef struct {
  X52Report_t report;
  const uint16_t USB_VID = 0x06a3;
  const uint16_t USB_PID = 0x075c;
  uint8_t dev_addr;
  uint8_t instance;
  uint8_t report_len;
  bool connected = false;
  bool available = false;
  bool debug = true;
} X52_state_t;

void print_X52_controls(volatile X52_state_t *X52) {
  DBG_printf("X:%d,Y:%d,twist:%d,throttle:%d,thr_small:%d,thr_large:%d,thr_slider:%d,",
      X52->report.x, X52->report.y, X52->report.twist,
      X52->report.throttle,
      X52->report.throttle_small_rotary, X52->report.throttle_large_rotary,
      X52->report.throttle_slider);
    if (X52->report.trigger1) DBG_print("trigger 1,");
    if (X52->report.fire) DBG_print("fire,");
    if (X52->report.button_a) DBG_print("a,");
    if (X52->report.button_b) DBG_print("b,");

    if (X52->report.button_c) DBG_print("c,");
    if (X52->report.pinky) DBG_print("pinky,");
    if (X52->report.button_d) DBG_print("d,");
    if (X52->report.button_e) DBG_print("e,");

    if (X52->report.button_t1_up) DBG_print("T1 up,");
    if (X52->report.button_t1_down) DBG_print("T1 down,");
    if (X52->report.button_t2_up) DBG_print("T2 up,");
    if (X52->report.button_t2_down) DBG_print("T2 down,");

    if (X52->report.button_t3_up) DBG_print("T3 up,");
    if (X52->report.button_t3_down) DBG_print("T3 down,");
    if (X52->report.trigger2) DBG_print("trigger 2,");
    if (X52->report.top_rotary_north) DBG_print("top rotary north,");

    if (X52->report.top_rotary_east) DBG_print("top rotary east,");
    if (X52->report.top_rotary_south) DBG_print("top rotary south,");
    if (X52->report.top_rotary_west) DBG_print("top rotary west,");
    if (X52->report.throttle_rotary_north) DBG_print("throttle_rotary_north,");

    if (X52->report.throttle_rotary_east) DBG_print("throttle_rotary_east,");
    if (X52->report.throttle_rotary_south) DBG_print("throttle_rotary_south,");
    if (X52->report.throttle_rotary_west) DBG_print("throttle_rotary_west,");
    if (X52->report.top_rotary_green) DBG_print("top rotary green,");

    if (X52->report.top_rotary_yellow) DBG_print("top rotary yellow,");
    if (X52->report.top_rotary_red) DBG_print("top rotary red,");
    if (X52->report.button_function) DBG_print("Function,");
    if (X52->report.button_start) DBG_print("Start/Stop,");

    if (X52->report.button_reset) DBG_print("Reset,");
    if (X52->report.button_i) DBG_print("i,");
    if (X52->report.button_mouse) DBG_print("mouse click,");
    if (X52->report.button_wheel) DBG_print("wheel click");

    if (X52->report.throttle_wheel_down) DBG_print("wheel down,");
    if (X52->report.throttle_wheel_up) DBG_print("wheel up,");
  DBG_printf("dpad:%d,mouse_x:%d,mouse_y:%d",
      X52->report.top_dpad, X52->report.mouse_x, X52->report.mouse_y);
  DBG_println();
}
