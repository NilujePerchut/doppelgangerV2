#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net, POWER
from src.term_switch import term_switch
from lib.sch_utils import unit_map_on_he10, run_unit


def unit_term_switch():
    """Unit test the power stage"""
    # Input signals & connector
    power = {name: Net(name) for name in ["vin", "gnd", "v33"]}
    others = {name: Net(name) for name in ["command", "line", "cmd"]}
    power["vin"].drive = POWER
    power["gnd"].drive = POWER
    power["v33"].drive = POWER
    sigs = {**power, **others}
    unit_map_on_he10(sigs.values())

    sigs = {"toto": [others["line"], "3K5", others["cmd"], power["v33"]]}

    term_switch(power["vin"], power["v33"], power["gnd"], sigs)


if __name__ == "__main__":
    run_unit(unit_term_switch)
