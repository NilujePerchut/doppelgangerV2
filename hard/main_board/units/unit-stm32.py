#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net, POWER
from src.stm32 import stm32
from lib.sch_utils import unit_map_on_he10, run_unit


def unit_stm32():
    """Unit test the power stage"""
    # Input signals & connector
    sw_names = ["UP", "DOWN", "LEFT", "RIGHT", "SQUARE", "TRIANGLE", "R1", "L1",
                "CROSS", "CIRCLE", "R2", "L2", "L3", "R3", "START", "SELECT",
                "HOME"]
    power = {"vin": Net("vin"), "gnd": Net("gnd"), "vio": Net("vio")}
    power["vin"].drive = POWER
    power["gnd"].drive = POWER
    power["vio"].drive = POWER
    sws = {name: Net(name) for name in sw_names}
    in_nets = {**sws, **power}
    unit_map_on_he10(in_nets.values())

    pic_iface, gpios, usb, mux_sel, bpc, bfn= stm32(power["vin"], power["gnd"],
                                                    sws, power["vio"])

    out_signals = {**pic_iface, **gpios, **usb, **mux_sel,
                   **{"bpc": bpc, "bfn": bfn}}
    unit_map_on_he10(out_signals.values())


if __name__ == "__main__":
    run_unit(unit_stm32)
