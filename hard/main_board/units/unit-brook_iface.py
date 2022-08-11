#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net, POWER
from src.brook_iface import brook_iface
from lib.sch_utils import unit_map_on_he10, run_unit


def unit_brook_iface():
    """Unit test the power stage"""

    switches = {"UP", "DOWN", "LEFT", "RIGHT", "SQUARE", "TRIANGLE", "R1", "L1",
                "CROSS", "CIRCLE", "R2", "L2", "L3", "R3",
                "START", "SELECT", "HOME"}
    sw_nets = {net_name: Net(net_name) for net_name in switches}
    power_nets = {name: Net(name) for name in ["v33", "v5", "gnd"]}
    in_signals = {**sw_nets, **power_nets}
    v5_ok = Net("v5_ok")
    faultn = Net("faultn")
    for net in power_nets.values():
        net.drive = POWER
    unit_map_on_he10(list(in_signals.values()) + [v5_ok, faultn])

    # Output signals & Connectors
    usb = brook_iface(sw_nets, power_nets["v33"], power_nets["v5"],
                      v5_ok, faultn, power_nets["gnd"])

    out_signals = {**usb, **{"faultn": faultn}}
    unit_map_on_he10(list(out_signals.values()))

if __name__ == "__main__":
    run_unit(unit_brook_iface)
