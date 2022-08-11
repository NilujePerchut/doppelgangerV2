#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net, POWER
from src.switch_inputs import joystick_input_layer
from lib.sch_utils import unit_map_on_he10, run_unit


def unit_switch_inputs():
    """Unit test the power stage"""
    power = {"v33": Net("v33"), "gnd": Net("gnd")}
    power["v33"].drive = POWER
    power["gnd"].drive = POWER
    unit_map_on_he10(power.values())

    in_nets = joystick_input_layer(power["v33"], power["gnd"])

    unit_map_on_he10(in_nets.values())


if __name__ == "__main__":
    run_unit(unit_switch_inputs)
