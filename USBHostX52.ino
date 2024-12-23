/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/


/* This example demonstrates use of both device and host, where
 * - Device run on native usb controller (roothub port0)
 * - Host depending on MCUs run on either:
 *   - rp2040: bit-banging 2 GPIOs with the help of Pico-PIO-USB library (roothub port1)
 *   - samd21/51, nrf52840, esp32: using MAX3421e controller (host shield)
 *
 * Requirements:
 * - For rp2040:
 *   - [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB) library
 *   - 2 consecutive GPIOs: D+ is defined by PIN_USB_HOST_DP, D- = D+ +1
 *   - Provide VBus (5v) and GND for peripheral
 *   - CPU Speed must be either 120 or 240 Mhz. Selected via "Menu -> CPU Speed"
 * - For samd21/51, nrf52840, esp32:
 *   - Additional MAX2341e USB Host shield or featherwing is required
 *   - SPI instance, CS pin, INT pin are correctly configured in usbh_helper.h
 */

/*
 * Modified for gigaxac_usb but contains lots of Adafruit code covered by
 * the above statement.
 */

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

// Set this to 0 for use with XAC. Turning this only for debugging because
// it slows down input event processing.
#define USB_DEBUG 1

#define X52_HEX_DUMP 0

#if USB_DEBUG
#define DBG_print(...)    Serial.print(__VA_ARGS__)
#define DBG_println(...)  Serial.println(__VA_ARGS__)
#define DBG_printf(...)   Serial.printf(__VA_ARGS__)
#else
#define DBG_print(...)
#define DBG_println(...)
#define DBG_printf(...)
#endif

// USBHost is defined in usbh_helper.h
#include "usbh_helper.h"

// Language ID: English
#define LANGUAGE_ID 0x0409

typedef struct {
  tusb_desc_device_t desc_device;
  uint16_t manufacturer[32];
  uint16_t product[48];
  uint16_t serial[16];
  bool mounted;
} dev_info_t;

// CFG_TUH_DEVICE_MAX is defined by tusb_config header
dev_info_t dev_info[CFG_TUH_DEVICE_MAX] = { 0 };

/******************************************************************/
#include "logitech_x52.h"
volatile X52_state_t X52; // Logitech X52 HOTAS
/******************************************************************/

void setup() {
#if USB_DEBUG
  Serial.begin(115200);

  // wait until device mounted
  while( !USBDevice.mounted() ) delay(1);
  while (!Serial && (millis() < 4000)) delay(10);   // wait for native usb
  DBG_println("USBHostX52");
#else
  Serial.end();
  // wait until device mounted
  while( !USBDevice.mounted() ) delay(1);
#endif

#if defined(CFG_TUH_MAX3421) && CFG_TUH_MAX3421
  // init host stack on controller (rhport) 1
  // For rp2040: this is called in core1's setup1()
  USBHost.begin(1);
#endif
}

#if defined(CFG_TUH_MAX3421) && CFG_TUH_MAX3421
//--------------------------------------------------------------------+
// Using Host shield MAX3421E controller
//--------------------------------------------------------------------+
void loop() {
  USBHost.task();
  Serial.flush();
}

#elif defined(ARDUINO_ARCH_RP2040)
//--------------------------------------------------------------------+
// For RP2040 use both core0 for device stack, core1 for host stack
//--------------------------------------------------------------------+

typedef struct {
  uint8_t x;
  uint8_t y;
} xycoords_t;

const xycoords_t dir2xy[16] = {
  {127, 127}, // Center, no direction
  {127, 0},   // North
  {255, 0},   // North East
  {255, 127}, // East
  {255, 255}, // South East
  {127, 255}, // South
  {0, 255},   // South West
  {0, 127},   // West
  {0, 0},     // North West
  {127, 127},
  {127, 127},
  {127, 127},
  {127, 127},
  {127, 127},
  {127, 127},
};

//------------- Core0 -------------//
void loop() {
  if (X52.connected) {
    if (X52.available) {
      if (sizeof(X52.report) == X52.report_len) {
        if (X52.debug) {
          print_X52_controls(&X52);
        }
      }
      X52.available = false;
    }
  }
}

//------------- Core1 -------------//
void setup1() {
  //while ( !Serial ) delay(10);   // wait for native usb
  // configure pio-usb: defined in usbh_helper.h
  rp2040_configure_pio_usb();

  // run host stack on controller (rhport) 1
  // Note: For rp2040 pico-pio-usb, calling USBHost.begin() on core1 will have most of the
  // host bit-banging processing works done in core1 to free up core0 for other works
  USBHost.begin(1);
}

void loop1() {
  USBHost.task();
  Serial.flush();
}
#endif

//--------------------------------------------------------------------+
// TinyUSB Host callbacks
//--------------------------------------------------------------------+
void print_device_descriptor(tuh_xfer_t *xfer);

void utf16_to_utf8(uint16_t *temp_buf, size_t buf_len);

void print_lsusb(void) {
  bool no_device = true;
  for (uint8_t daddr = 1; daddr < CFG_TUH_DEVICE_MAX + 1; daddr++) {
    // TODO can use tuh_mounted(daddr), but tinyusb has an bug
    // use local connected flag instead
    dev_info_t *dev = &dev_info[daddr - 1];
    if (dev->mounted) {
      DBG_printf("Device %u: ID %04x:%04x %s %s\r\n", daddr,
                    dev->desc_device.idVendor, dev->desc_device.idProduct,
                    (char *) dev->manufacturer, (char *) dev->product);

      no_device = false;
    }
  }

  if (no_device) {
    DBG_println("No device connected (except hub)");
  }
}

// Invoked when device is mounted (configured)
void tuh_mount_cb(uint8_t daddr) {
  DBG_printf("Device attached, address = %d\r\n", daddr);

  dev_info_t *dev = &dev_info[daddr - 1];
  dev->mounted = true;

  // Get Device Descriptor
  tuh_descriptor_get_device(daddr, &dev->desc_device, 18, print_device_descriptor, 0);
}

/// Invoked when device is unmounted (bus reset/unplugged)
void tuh_umount_cb(uint8_t daddr) {
  DBG_printf("Device removed, address = %d\r\n", daddr);
  dev_info_t *dev = &dev_info[daddr - 1];
  dev->mounted = false;

  // print device summary
  print_lsusb();
}

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use.
// tuh_hid_parse_report_descriptor() can be used to parse common/simple enough
// descriptor. Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE,
// it will be skipped therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
  (void) desc_report;
  (void) desc_len;
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  DBG_printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  DBG_printf("VID = %04x, PID = %04x\r\n", vid, pid);
  if ((vid == X52.USB_VID) && (pid == X52.USB_PID)) {
    DBG_println("Logitech X52 connected");
    X52.connected = true;
    X52.available = false;
    X52.dev_addr = dev_addr;
    X52.instance = instance;
    memset((X52Report_t *)&X52.report, 0, sizeof(X52.report));
  }
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    DBG_printf("Error: cannot request to receive report\r\n");
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  DBG_printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
  if ((X52.dev_addr == dev_addr) && (X52.instance == instance)) {
    if (X52.connected) {
      X52.connected = false;
      X52.available = false;
      DBG_printf("X52 controller (%d) disconnected\r\n", instance);
    }
  }
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
  if (X52.connected) {
    memcpy((X52Report_t *)&X52.report, report, min(sizeof(X52.report), len));
    X52.report_len = len;
    if (X52_HEX_DUMP) {
      uint8_t *report = (uint8_t *)&X52.report;
      DBG_printf("X52 report(%d): ", len);
      for (uint16_t i = 0; i < min(31, X52.report_len); i++) {
        DBG_printf("0x%02X ", report[i]);
      }
      DBG_println();
    }
    X52.available = true;
  }
  // continue to request to receive report
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    DBG_printf("Error: cannot request to receive report\r\n");
  }
}

void print_device_descriptor(tuh_xfer_t *xfer) {
  if (XFER_RESULT_SUCCESS != xfer->result) {
    DBG_printf("Failed to get device descriptor\r\n");
    return;
  }

  uint8_t const daddr = xfer->daddr;
  dev_info_t *dev = &dev_info[daddr - 1];
  tusb_desc_device_t *desc = &dev->desc_device;

  DBG_printf("Device %u: ID %04x:%04x\r\n", daddr, desc->idVendor, desc->idProduct);
  DBG_printf("Device Descriptor:\r\n");
  DBG_printf("  bLength             %u\r\n"     , desc->bLength);
  DBG_printf("  bDescriptorType     %u\r\n"     , desc->bDescriptorType);
  DBG_printf("  bcdUSB              %04x\r\n"   , desc->bcdUSB);
  DBG_printf("  bDeviceClass        %u\r\n"     , desc->bDeviceClass);
  DBG_printf("  bDeviceSubClass     %u\r\n"     , desc->bDeviceSubClass);
  DBG_printf("  bDeviceProtocol     %u\r\n"     , desc->bDeviceProtocol);
  DBG_printf("  bMaxPacketSize0     %u\r\n"     , desc->bMaxPacketSize0);
  DBG_printf("  idVendor            0x%04x\r\n" , desc->idVendor);
  DBG_printf("  idProduct           0x%04x\r\n" , desc->idProduct);
  DBG_printf("  bcdDevice           %04x\r\n"   , desc->bcdDevice);

  // Get String descriptor using Sync API
  DBG_printf("  iManufacturer       %u     ", desc->iManufacturer);
  if (XFER_RESULT_SUCCESS ==
      tuh_descriptor_get_manufacturer_string_sync(daddr, LANGUAGE_ID, dev->manufacturer, sizeof(dev->manufacturer))) {
    utf16_to_utf8(dev->manufacturer, sizeof(dev->manufacturer));
    DBG_printf((char *) dev->manufacturer);
  }
  DBG_printf("\r\n");

  DBG_printf("  iProduct            %u     ", desc->iProduct);
  if (XFER_RESULT_SUCCESS ==
      tuh_descriptor_get_product_string_sync(daddr, LANGUAGE_ID, dev->product, sizeof(dev->product))) {
    utf16_to_utf8(dev->product, sizeof(dev->product));
    DBG_printf((char *) dev->product);
  }
  DBG_printf("\r\n");

  DBG_printf("  iSerialNumber       %u     ", desc->iSerialNumber);
  if (XFER_RESULT_SUCCESS ==
      tuh_descriptor_get_serial_string_sync(daddr, LANGUAGE_ID, dev->serial, sizeof(dev->serial))) {
    utf16_to_utf8(dev->serial, sizeof(dev->serial));
    DBG_printf((char *) dev->serial);
  }
  DBG_printf("\r\n");

  DBG_printf("  bNumConfigurations  %u\r\n", desc->bNumConfigurations);

  // print device summary
  print_lsusb();
}

//--------------------------------------------------------------------+
// String Descriptor Helper
//--------------------------------------------------------------------+

static void _convert_utf16le_to_utf8(const uint16_t *utf16, size_t utf16_len, uint8_t *utf8, size_t utf8_len) {
  // TODO: Check for runover.
  (void) utf8_len;
  // Get the UTF-16 length out of the data itself.

  for (size_t i = 0; i < utf16_len; i++) {
    uint16_t chr = utf16[i];
    if (chr < 0x80) {
      *utf8++ = chr & 0xff;
    } else if (chr < 0x800) {
      *utf8++ = (uint8_t) (0xC0 | (chr >> 6 & 0x1F));
      *utf8++ = (uint8_t) (0x80 | (chr >> 0 & 0x3F));
    } else {
      // TODO: Verify surrogate.
      *utf8++ = (uint8_t) (0xE0 | (chr >> 12 & 0x0F));
      *utf8++ = (uint8_t) (0x80 | (chr >> 6 & 0x3F));
      *utf8++ = (uint8_t) (0x80 | (chr >> 0 & 0x3F));
    }
    // TODO: Handle UTF-16 code points that take two entries.
  }
}

// Count how many bytes a utf-16-le encoded string will take in utf-8.
static int _count_utf8_bytes(const uint16_t *buf, size_t len) {
  size_t total_bytes = 0;
  for (size_t i = 0; i < len; i++) {
    uint16_t chr = buf[i];
    if (chr < 0x80) {
      total_bytes += 1;
    } else if (chr < 0x800) {
      total_bytes += 2;
    } else {
      total_bytes += 3;
    }
    // TODO: Handle UTF-16 code points that take two entries.
  }
  return total_bytes;
}

void utf16_to_utf8(uint16_t *temp_buf, size_t buf_len) {
  size_t utf16_len = ((temp_buf[0] & 0xff) - 2) / sizeof(uint16_t);
  size_t utf8_len = _count_utf8_bytes(temp_buf + 1, utf16_len);

  _convert_utf16le_to_utf8(temp_buf + 1, utf16_len, (uint8_t *) temp_buf, buf_len);
  ((uint8_t *) temp_buf)[utf8_len] = '\0';
}
