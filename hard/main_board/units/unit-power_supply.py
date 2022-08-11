#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net, POWER
from src.power_supply import power_stage
from lib.sch_utils import dop_part, run_unit


def unit_power_stage():
    """Unit test the power stage"""
    # Input signals & connector
    vin = Net("vin")
    gnd = Net("gnd")
    vin.drive = POWER
    gnd.drive = POWER
    in_connector = dop_part("CONN_01X02", "SIL2")
    in_connector[1] += vin
    in_connector[2] += gnd

    # Output signals & Connectors
    vin_o, v3V3, = power_stage(vin, gnd)
    out_connector = dop_part("CONN_01x03", "SIL2")
    out_connector[1] += vin_o
    out_connector[2] += v3V3

    assert vin_o is vin

if __name__ == "__main__":
    run_unit(unit_power_stage)
