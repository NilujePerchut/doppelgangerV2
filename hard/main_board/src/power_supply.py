#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net
from lib.sch_utils import dop_part, get_res, pull_updown, bypass_cap
from lib.sch_utils import power_indicator


def power_stage(vin, gnd):
    """Set up the power stage.
    Returns vin (vio), 3v3 and supervisor"""

    reg = dop_part("TPS63001", "Texas_DRC0010J_ThermalVias",
                   fields={"Reference": "TPS63001DRDCR",
                           "Descr": "ARM Cortex M4 ST version",
                           "CC": "3008242 ",
                           "JLCC": "C28060",
                           "JLROT": "90"})

    inductor = dop_part("L", "1227AS-H-1R5N=P2",
                        fields={"JLCC": "C435389"})

    v33 = Net("V33")

    reg["VIN", "VINA", "PS/SYNC", "EN"] += vin
    reg["VOUT"] += v33
    reg["GND", "PGND"] += gnd
    v33 & get_res("0R", "0603", fields={"JLCC": "C21189"}) & reg["FB"]
    reg["L1"] & inductor[1, 2] & reg["L2"]

    bypass_cap(vin, gnd, ["10uF"]*2, fields={"JLCC": "C19702"},
               descr="POWER_SUPPLY_VIN")
    bypass_cap(v33, gnd, ["10uF"]*3, fields={"JLCC": "C19702"},
               descr="POWER_SUPPLY_VOUT")

    vin & dop_part("TestPoint", "TestPoint_Pad_D1.5mm")
    v33 & dop_part("TestPoint", "TestPoint_Pad_D1.5mm")

    return vin, v33
