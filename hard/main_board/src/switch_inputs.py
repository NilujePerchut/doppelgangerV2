#! /usr/bin/env python3
# -*- coding: utf-8 -*-


from skidl import Net
from lib.sch_utils import dop_part, pull_updown, LVC32


def _menu_button_filter(master, nets, v33, gnd):
    """Adds a LVC32 filter for active high signals"""
    out_nets = []
    lvc32_chip = LVC32(v33, gnd, ["100nF", "0603"], c_fields={"JLCC": "C14663"})
    for i, net in enumerate(nets):
        out_net = Net(net.name + "_FILTERED")
        lvc32_chip.add(master, net, out_net, index=i)
        out_nets.append(out_net)
    lvc32_chip.fill_unused()
    return out_nets


def joystick_input_layer(v33, gnd):
    # Input connectors pinout (not affected means GND).
    # (connector_number, connector_pin)
    BtnConns = {"UP":      (1, 4),
                "DOWN":    (1, 5),
                "LEFT":    (1, 2),
                "RIGHT":   (1, 3),
                "SQUARE":  (0, 6),
                "TRIANGLE":(0, 7),
                "R1":      (0, 9),
                "L1":      (0, 10),
                "CROSS":   (0, 4),
                "CIRCLE":  (0, 5),
                "R2":      (0, 3),
                "L2":      (0, 2),
                "L3":      (1, 1),
                "R3":      (0, 8),
                "DISABLE": (1, 9),
                "START":   (1, 8),
                "SELECT":  (1, 7),
                "HOME":    (1, 6)
               }

    js = [dop_part("BORNIER_10", "BORNIER_10",
                   fields={"Reference": "PT1,5/10-5.0-H",
                           "Descr": "10-pin 5mm pitch",
                           "CC": "1793681"})
          for i in range(2)]

    # Generate input signal list and add pullup resistor
    in_nets = {signal: Net(signal) for signal in list(BtnConns.keys())}
    pull_updown(v33, list(in_nets.values()), "10K", fields={"JLCC": "C25804"})

    # Assign each connector pin a net
    for signal, pinout in BtnConns.items():
        js[pinout[0]][pinout[1]] += in_nets[signal]

    # Put GND on last port of each input port
    js[0][1] += gnd
    js[1][10] += gnd

    # Filter HOME, SELECT, HOME according to the DISABLE signal
    # ATTENTION: order in the following list matters to gate attribution
    to_be_filtered = [in_nets[n] for n in ["START", "SELECT", "HOME"]]
    filtered = _menu_button_filter(in_nets["DISABLE"], to_be_filtered, v33, gnd)
    in_nets["START"], in_nets["SELECT"], in_nets["HOME"] = filtered

    # Insert an UP push button to force bootloader entry
    in_nets["UP"] & dop_part("SW_PUSH", "BP") & gnd

    # Remove DISABLE from the in_nets dict (only needed for filtering)
    in_nets.pop("DISABLE")

    return in_nets
