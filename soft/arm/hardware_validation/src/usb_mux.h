#pragma once

/* USB mux iface*/
#define USBMUX_RCC RCC_GPIOC
#define USBMUX_PORT GPIOC
#define USBMUX_SEL0 GPIO9
#define USBMUX_SEL1 GPIO8

enum {
	MUX_PIC_IOS = 0,
	MUX_ARM_IOS,
	MUX_BROOK_USB,
	MUX_ARM_USB,
};

#define USB_MUX_DEFAULT MUX_ARM_USB

void set_usb_mux(int mux_index);
void init_usb_mux(void);
