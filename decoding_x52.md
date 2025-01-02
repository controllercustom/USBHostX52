This article explains how the USB X52 decoder was developed. The following
command line utilities work on Ubuntu 22.04 but probably work on any Debian or
Debian derived distribution. I suspect a Raspberry Pi running RPi OS can do all
this as well.

## List all USB devices one per line

Plug the Logitech X52 into a USB host port.

```
$ lsusb
...
Bus 001 Device 012: ID 06a3:075c Saitek PLC X52 Flight Controller
...
```

The only relevant line is the one for the Saitek/Logitech X52. The important
information for remaining steps is 06a3:075c. The first group of hex digits is
the USB Vendor ID. The second group is the USB Product ID.

## List detailed USB descriptors for one USB device

These lines from the listing below indicate the X52 is a USB HID device.

```
      bInterfaceClass         3 Human Interface Device
      bInterfaceSubClass      0
      bInterfaceProtocol      0
```

Note some joysticks are USB vendor specific devices so are much harder to work
with. For example, Xbox One controllers are vendor (Microsoft) specific so are
not USB HID devices.

```
$ sudo lsusb -vd 06a3:075c

Bus 001 Device 012: ID 06a3:075c Saitek PLC X52 Flight Controller
Device Descriptor:
  bLength                18
  bDescriptorType         1
  bcdUSB               2.00
  bDeviceClass            0
  bDeviceSubClass         0
  bDeviceProtocol         0
  bMaxPacketSize0         8
  idVendor           0x06a3 Saitek PLC
  idProduct          0x075c X52 Flight Controller
  bcdDevice           21.00
  iManufacturer           1 Logitech
  iProduct                2 X52 H.O.T.A.S.
  iSerial                 0
  bNumConfigurations      1
  Configuration Descriptor:
    bLength                 9
    bDescriptorType         2
    wTotalLength       0x0022
    bNumInterfaces          1
    bConfigurationValue     1
    iConfiguration          0
    bmAttributes         0x80
      (Bus Powered)
    MaxPower              100mA
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        0
      bAlternateSetting       0
      bNumEndpoints           1
      bInterfaceClass         3 Human Interface Device
      bInterfaceSubClass      0
      bInterfaceProtocol      0
      iInterface              0
        HID Device Descriptor:
          bLength                 9
          bDescriptorType        33
          bcdHID               1.11
          bCountryCode            0 Not supported
          bNumDescriptors         1
          bDescriptorType        34 Report
          wDescriptorLength     119
         Report Descriptors:
           ** UNAVAILABLE **
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x81  EP 1 IN
        bmAttributes            3
          Transfer Type            Interrupt
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0010  1x 16 bytes
        bInterval              10
Device Status:     0x0000
  (Bus Powered)
```

## Show the USB HID Report Descriptor

HID devices return control values in structures known as HID reports.
The format of the HID report is described by a HID report descriptor.
HID devices return their HID report descriptor on demand. The
usbhid-dump command returns a hex dump of the descriptor. The lsusb
command can do this but usbhid-dump is much easier to use.

```
$ sudo usbhid-dump -d 06a3:075c
001:012:000:DESCRIPTOR         1735253017.435155
 05 01 09 04 A1 01 09 01 A1 00 09 30 09 31 15 00
 26 FF 07 75 0B 95 02 81 02 09 35 15 00 26 FF 03
 75 0A 95 01 81 02 09 32 09 33 09 34 09 36 15 00
 26 FF 00 75 08 95 04 81 02 05 09 19 01 29 22 15
 00 25 01 95 22 75 01 81 02 75 02 95 01 81 01 05
 01 09 39 15 01 25 08 35 00 46 3B 01 66 14 00 75
 04 95 01 81 42 05 05 09 24 09 26 15 00 25 0F 75
 04 95 02 81 02 C0 C0

```

## Decode the descriptor

This [website](https://eleccelerator.com/usbdescreqparser/) has a USB
descriptor decoder/parser. It converts USB descriptor hex dumps into human
readable format.

Copy and paste the above hex dump into the webpage Input window.
Press the USB HID Report Descriptor button. The contents of the Output
window is shown below.

```
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x04,        // Usage (Joystick)
0xA1, 0x01,        // Collection (Application)
0x09, 0x01,        //   Usage (Pointer)
0xA1, 0x00,        //   Collection (Physical)
0x09, 0x30,        //     Usage (X)
0x09, 0x31,        //     Usage (Y)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x07,  //     Logical Maximum (2047)
0x75, 0x0B,        //     Report Size (11)
0x95, 0x02,        //     Report Count (2)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x35,        //     Usage (Rz)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x03,  //     Logical Maximum (1023)
0x75, 0x0A,        //     Report Size (10)
0x95, 0x01,        //     Report Count (1)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x32,        //     Usage (Z)
0x09, 0x33,        //     Usage (Rx)
0x09, 0x34,        //     Usage (Ry)
0x09, 0x36,        //     Usage (Slider)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x04,        //     Report Count (4)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x09,        //     Usage Page (Button)
0x19, 0x01,        //     Usage Minimum (0x01)
0x29, 0x22,        //     Usage Maximum (0x22)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x95, 0x22,        //     Report Count (34)
0x75, 0x01,        //     Report Size (1)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x75, 0x02,        //     Report Size (2)
0x95, 0x01,        //     Report Count (1)
0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x09, 0x39,        //     Usage (Hat switch)
0x15, 0x01,        //     Logical Minimum (1)
0x25, 0x08,        //     Logical Maximum (8)
0x35, 0x00,        //     Physical Minimum (0)
0x46, 0x3B, 0x01,  //     Physical Maximum (315)
0x66, 0x14, 0x00,  //     Unit (System: English Rotation, Length: Centimeter)
0x75, 0x04,        //     Report Size (4)
0x95, 0x01,        //     Report Count (1)
0x81, 0x42,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
0x05, 0x05,        //     Usage Page (Game Ctrls)
0x09, 0x24,        //     Usage (Move Right/Left)
0x09, 0x26,        //     Usage (Move Up/Down)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x0F,        //     Logical Maximum (15)
0x75, 0x04,        //     Report Size (4)
0x95, 0x02,        //     Report Count (2)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0xC0,              // End Collection

// 119 bytes
```

This information can now the used to create a C struct that overlays the
control values (HID report) from the joystick. Only the lines that correspond
to elements of the C struct are explained.

```
0x09, 0x30,        //     Usage (X)
0x09, 0x31,        //     Usage (Y)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x07,  //     Logical Maximum (2047)
0x75, 0x0B,        //     Report Size (11)
0x95, 0x02,        //     Report Count (2)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
```

The first two elements are X and Y axes that range from 0 to 2047.
This takes 11 bits each.

```C
    uint32_t x:11;      // 0..1024..2047
    uint32_t y:11;      // 0..1024..2047
```

```
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x03,  //     Logical Maximum (1023)
0x75, 0x0A,        //     Report Size (10)
0x95, 0x01,        //     Report Count (1)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
```

The next element ranges from 0 to 1023. This takes 10 bits.

```C
    uint32_t twist:10;  // 0..512..1023
```

```
0x09, 0x32,        //     Usage (Z)
0x09, 0x33,        //     Usage (Rx)
0x09, 0x34,        //     Usage (Ry)
0x09, 0x36,        //     Usage (Slider)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x04,        //     Report Count (4)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
```

The next four elements range from 0 to 255. Each one takes 8 bits. The
Z axis corresponds to the big throttle. The Rx axis corresponds to the
small rotary dial on the throttle. And so on. The correlation of
descriptor axis name to C struct element name is done by moving the
controls one at time then observing which element values changes. Then
the element name is changed to describe the X52 control.

```C
    uint8_t throttle;
    uint8_t throttle_small_rotary;
    uint8_t throttle_large_rotary;
    uint8_t throttle_slider;
```

```
0x05, 0x09,        //     Usage Page (Button)
0x19, 0x01,        //     Usage Minimum (0x01)
0x29, 0x22,        //     Usage Maximum (0x22)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x95, 0x22,        //     Report Count (34)
0x75, 0x01,        //     Report Size (1)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No
```

There are 34 buttons. The names are mostly based on labels printed on
the joystick. Note the big trigger is has two buttons. trigger1 is
the first position. trigger2 is the second fully depressed position.

```C
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
```

```
0x75, 0x02,        //     Report Size (2)
0x95, 0x01,        //     Report Count (1)
0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
```

This element is filler bits for byte alignment purposes, most likely.

```C
    uint16_t filler:2;
```

```
0x09, 0x39,        //     Usage (Hat switch)
0x15, 0x01,        //     Logical Minimum (1)
0x25, 0x08,        //     Logical Maximum (8)
0x35, 0x00,        //     Physical Minimum (0)
0x46, 0x3B, 0x01,  //     Physical Maximum (315)
0x66, 0x14, 0x00,  //     Unit (System: English Rotation, Length: Centimeter)
0x75, 0x04,        //     Report Size (4)
0x95, 0x01,        //     Report Count (1)
0x81, 0x42,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
```

The next element is a hat switch/direction pad that ranges from 1 to 8. This
takes 4 bits. The 8 values correspond to 8 compass directions.

```C
    uint16_t top_dpad:4;  // 0..8, 0=center, 1=N, ...
```

```
0x05, 0x05,        //     Usage Page (Game Ctrls)
0x09, 0x24,        //     Usage (Move Right/Left)
0x09, 0x26,        //     Usage (Move Up/Down)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x0F,        //     Logical Maximum (15)
0x75, 0x04,        //     Report Size (4)
0x95, 0x02,        //     Report Count (2)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
```

The final two elements are for the mini joystick on the throttle. The axes
range from 0 to 15. Two axes takes 4 bits each.

```C
    uint16_t mouse_y:4;
    uint16_t mouse_x:4;
```
