#! /usr/bin/env python3
# -*- coding: utf-8 -*-


from skidl import Net, POWER
from lib.sch_utils import LVC07, dop_part, bypass_cap, pull_updown


def _oc_buffers(oc_map, switches, v33, gnd):
    """Insert open collector buffers"""
    lvc07_chips = [LVC07(v33, gnd, ["100nF", "0603"],
                         c_fields={"JLCC": "C14663"})
                  for i in range(3)]
    oc_nets = {v: Net(F"{v}_BROOK") for v in list(switches.keys())}

    for net in list(switches.values()):
        if "DISABLE" in net.name:
            continue
        # Need to produce a name without _FILTERED
        name = net.name.replace("_FILTERED", "")
        chip, gate = oc_map[name]
        lvc07_chips[chip].add(net, out=oc_nets[name], index=gate)

    for chip in lvc07_chips:
        chip.fill_unused(v33)

    return oc_nets


def _to_brook_conn_name(name):
    """Try to convert poorly signal names to other poorly signal names"""
    brook_name = name.title()
    # If digital pad direction, adds "D_"
    if brook_name in ["Up", "Down", "Right", "Left"]:
        brook_name = "D_" + brook_name
    # Remap controls on PS4 names + Fixes UP full capital case
    try:
        brook_name = {"Home": "PS_Key", "Start": "Options",
                      "Select": "Share"}[brook_name]
    except KeyError:
        # Nothing to do
        pass

    return brook_name


def brook_iface(switches, v33, v5, v5_ok, faultn, gnd):
    """Create the brook interface"""

    # (oc_chip, oc_gate)
    oc_map = {"UP":              (0, 4),
              "DOWN":            (0, 5),
              "LEFT":            (1, 0),
              "RIGHT":           (1, 1),
              "SQUARE":          (2, 2),
              "TRIANGLE":        (2, 3),
              "R1":              (2, 5),
              "L1":              (1, 4),
              "CROSS":           (1, 3),
              "CIRCLE":          (2, 1),
              "R2":              (1, 5),
              "L2":              (2, 0),
              "L3":              (1, 2),
              "R3":              (2, 4),
              "START":           (0, 1),
              "SELECT":          (0, 3),
              "HOME":            (0, 2),
              }

    oc_nets = _oc_buffers(oc_map, switches, v33, gnd)

    brook_conn = dop_part("BROOK_FB", "BROOK_FB")
    brook_conn["GND"] += gnd
    brook_conn["Vdd_V330"] += NC
    v5_sw = Net("V5_SW")
    v5_sw.drive = POWER
    brook_conn["USB_V500"] += v5_sw

    usb = {name: Net(name) for name in ["BROOK_USB_P", "BROOK_USB_N"]}
    brook_conn["D+"] += usb["BROOK_USB_P"]
    brook_conn["D-"] += usb["BROOK_USB_N"]

    # Connects every oc_nets
    for oc_net in oc_nets:
        # Skip the DISABLE signal
        if "DISABLE" in oc_net:
            continue

        brook_conn_name = _to_brook_conn_name(oc_net)
        brook_conn[brook_conn_name] += oc_nets[oc_net]

    # Add the SOCD selector
    # New SOCD style: <- + -> == -> and -> + <- == <-
    brook_conn["ADC_Ry"] & dop_part("GS2", "GS2") & gnd

    # Add 5V input
    # Use a dual use
    usbpowerswitch= dop_part("USBPowerSwitch", "SOT23-5",
                             fields={"Reference": "STMPS2151STR",
                                     "Descr": "Power switch",
                                     "CC": "1842616",
                                     "JLCC": "C351147",
                                     "JLROT": "180"})
    usbpowerswitch["IN"] += v5
    usbpowerswitch["GND"] += gnd
    usbpowerswitch["EN"] += v5_ok
    usbpowerswitch["OUT"] += v5_sw
    bypass_cap(v5, gnd, "1uF", package="0805", fields={"JLCC": "C28323"},
               descr="BROOK_USBPOWERSWITCH_VIN")
    bypass_cap(v5_sw, gnd, "100nF", fields={"JLCC": "C14663"},
               descr="BROOK_USBPOWERSWITCH_VOUT")
    pull_updown(gnd, v5_ok, "10K", fields={"JLCC": "C25804"})
    usbpowerswitch[3] += faultn
    pull_updown(v5, faultn, "10K", fields={"JLCC": "C25804"})

    # Returns the USB signals
    return usb
