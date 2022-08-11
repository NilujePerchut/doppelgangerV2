#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net
from lib.sch_utils import dop_part, bypass_cap, get_capa, get_res, pull_updown


def usb_mux(vin, gnd, sel, from_rj45,
            from_pic, from_stm, from_stm_usb, from_brook_usb,
            default_channel=None):
    """Set up the usb_mux. default channel puts pull resistors on the sel sig"""

    mux = dop_part("SN74CBT3253", "TSSOP16",
                   fields = {"Reference": "SN74CB3Q3253P",
                             "Descr": "Low RDS ON 1->4 dual mux",
                             "CC": "3006857",
                             "JLCC": "C424399",
                             "JLROT": "-90"})

    mux["VCC"] += vin
    mux["GND", "1~OE", "2~OE"] += gnd
    bypass_cap(vin, gnd, "100nF", fields={"JLCC": "C14663"}, descr="USB_MUX")

    # Setup selection
    mux["S0"] += sel["mux_sel0"]
    mux["S1"] += sel["mux_sel1"]

    if default_channel is not None:
        sel_bin = [int(d) for d in format(default_channel, "#02b")[2:][::-1]]
        for i, val in enumerate(sel_bin):
            pull_updown([gnd, vin][val], sel[F"mux_sel{i}"], "10K",
                        fields={"JLCC": "C25804"})

    # RJ45 input
    mux["1A"] += from_rj45["rj45_to_mux_0_usb_p"]
    mux["2A"] += from_rj45["rj45_to_mux_1_usb_n"]

    # PIC16 interface
    mux["1B1"] += from_pic["gpio0"]
    mux["2B1"] += from_pic["gpio1"]

    # STM32 interface
    mux["1B2"] += from_stm["gpio0"]
    mux["2B2"] += from_stm["gpio1"]

    # Brook USB interface
    mux["1B3"] += from_brook_usb["BROOK_USB_P"]
    mux["2B3"] += from_brook_usb["BROOK_USB_N"]

    # STM32 USB interface
    mux["1B4"] += from_stm_usb["STM32_USB_P"]
    mux["2B4"] += from_stm_usb["STM32_USB_N"]
