#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net, POWER
from src.usb_mux import usb_mux
from lib.sch_utils import unit_map_on_he10, run_unit


def unit_usb_mux():
    """Unit test the power stage"""
    # Input signals & connector
    power = {"vin": Net("vin"), "gnd": Net("gnd")}
    power["vin"].drive = POWER
    power["gnd"].drive = POWER

    sel = {name: Net(name) for name in ["mux_sel0", "mux_sel1"]}
    rj45 = {name: Net(name) for name in ["rj45_to_mux_0_usb_p",
                                         "rj45_to_mux_1_usb_n"]}
    pic = {"gpio0": Net("pic16_gpio0"), "gpio1": Net("pic16_gpio1")}
    stm = {"gpio0": Net("stm32_gpio0"), "gpio1": Net("stm32_gpio1")}
    stm_usb = {name: Net(name) for name in ["STM32_USB_P", "STM32_USB_N"]}
    brook_usb = {name: Net(name) for name in ["BROOK_USB_P", "BROOK_USB_N"]}

    # pic and stm have same keys, so they cannot be merged in nets
    nets = {**power, **sel, **rj45, **stm_usb, **brook_usb}
    nets_v = list(nets.values()) + list(pic.values()) + list(stm.values())
    unit_map_on_he10(nets_v)

    usb_mux(power["vin"], power["gnd"], sel, rj45, pic, stm, stm_usb, brook_usb,
            default_channel=2)

if __name__ == "__main__":
    run_unit(unit_usb_mux)
