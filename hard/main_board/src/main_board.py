#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from skidl import generate_netlist, lib_search_paths, KICAD, ERC, POWER
from skidl import generate_xml
from src.rj45 import rj45
from src.power_supply import power_stage
from src.switch_inputs import joystick_input_layer
from src.brook_iface import brook_iface
from src.stm32 import stm32
from src.pic16 import pic16
from src.usb_mux import usb_mux
from term_switch import term_switch
from mechs import mechs


def main_board():
    """The main board (lego style)"""
    rj_power, usb_rj, rj_sigs = rj45()
    gnd = rj_power["gnd"]
    vio, v33 = power_stage(rj_power["vcc"], gnd)

    # Set POWER tag to alim signalsto make erc happy
    gnd.drive = POWER
    vio.drive = POWER
    v33.drive = POWER

    sws_in_nets = joystick_input_layer(v33, gnd)
    stm_sigs = stm32(v33, gnd, sws_in_nets, vio)
    stm_to_pic, gpios_stm, usb_stm, mux_sel, bpower_cmd, bpower_ftn = stm_sigs
    usb_brook = brook_iface(sws_in_nets, v33, vio, bpower_cmd, bpower_ftn, gnd)
    gpios_pic, terms_cmds = pic16(vio, gnd, stm_to_pic, rj_sigs)
    usb_mux(v33, gnd, mux_sel, usb_rj, gpios_pic, gpios_stm, usb_stm,
            usb_brook, default_channel=3)

    terms = {
             "RJ_SPI_CLK":  [rj_sigs["RJ_SPI_CLK"], "3K5",
                             terms_cmds["snes_term_cmd"], vio],
             "RJ_SPI_CS":   [rj_sigs["RJ_SPI_CS"], "3K5",
                             terms_cmds["snes_term_cmd"], vio],
             "RJ_SPI_MISO": [rj_sigs["RJ_SPI_MISO"], "4K7",
                             terms_cmds["psx_term_cmd"], vio],
             "GPIO0":       [gpios_stm["gpio0"], "4K7",
                             terms_cmds["psx_term_cmd"], v33]}
    term_switch(vio, v33, gnd, terms)

    mechs()


if __name__ == "__main__":
    lib_search_paths[KICAD].append(os.environ["KIPRJLIB"])
    main_board()
    ERC()
    generate_netlist()
    generate_xml()
