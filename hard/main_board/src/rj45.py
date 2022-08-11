#! /usr/bin/env python3
# -*- coding: utf-8 -*-


from skidl import Net
from lib.sch_utils import dop_part, bypass_cap, power_indicator


def rj45():
    """Create the rj45 interface"""

    power = {"vcc": Net("VUSB"), "gnd": Net("GND")}
    usb = {"rj45_to_mux_0_usb_p": Net("RJ45_USB_P"),
           "rj45_to_mux_1_usb_n": Net("RJ45_USB_N")}
    sigs = {name: Net(name)
            for name in ["RJ_SPI_CLK", "RJ_SPI_MOSI", "RJ_SPI_MISO",
                         "RJ_SPI_CS"]}

    rj45_conn = dop_part("RJ45", "RJ45", fields={})
    esd_prot = dop_part("USBLC6-2SC6", "TSOT-23-6",
                        fields = {"Reference": "USBLC6-2SC6",
                                  "Descr": "ESD protection for USB2",
                                  "CC": "1269406",
                                  "JLCC": "C558442",
                                  "JLROT": "180"})

    dbg_conn = dop_part("CONN_01X06", "SIL6")

    rj45_conn[8] & power["vcc"] & dbg_conn[1]
    rj45_conn[1] & power["gnd"] & dbg_conn[6]
    bypass_cap(power["vcc"], power["gnd"], "100nF", fields={"JLCC": "C14663"},
               descr="RJ45_VCC")
    power_indicator([power["vcc"], power["gnd"]])

    # USB + its ESD protection
    rj45_conn[5] & esd_prot[3,4] & usb["rj45_to_mux_1_usb_n"]
    rj45_conn[6] & esd_prot[1,6] & usb["rj45_to_mux_0_usb_p"]
    esd_prot[5] += power["vcc"]
    esd_prot[2] += power["gnd"]

    rj45_conn[2] & sigs["RJ_SPI_CLK"] & dbg_conn[5]
    rj45_conn[3] & sigs["RJ_SPI_MOSI"] & dbg_conn[4]
    rj45_conn[4] & sigs["RJ_SPI_MISO"] & dbg_conn[3]
    rj45_conn[7] & sigs["RJ_SPI_CS"] & dbg_conn[2]

    return power, usb, sigs
