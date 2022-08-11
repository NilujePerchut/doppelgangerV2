#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net, NC
from lib.sch_utils import dop_part, bypass_cap, get_res, pull_updown


def pic16(vin, gnd, pic_iface, rj45_sigs):
    """Set up the power stage."""

    stm32_map = {"reset": None,  # MCLR
                 "icsp_clock": None,
                 "icsp_data": None,
                 "boot_cmd": "RA6",
                 # UART
                 "uart_tx": "RA4",  # STM32_TX --> PIC16_RX
                 "uart_rx": "RA5",  # STM32_RX <-- PIC16_TX
                 # DATA[3:0]
                 "data0": "RC0",
                 "data1": "RC1",
                 "data2": "RC2",
                 "data3": "RC3"}

    gpios = {"gpio0": Net("pic16_gpio0"),
             "gpio1": Net("pic16_gpio1")}

    term_cmds = {"snes_term_cmd": Net("snes_term_cmd"),
                 "psx_term_cmd": Net("psx_term_cmd")}

    mcu = dop_part("16F18855", "SSOP-28_5.3x10.2mm_P0.65mm",
                   fields={"Reference": "PIC16F18855-I/SS",
                           "Descr": "8-bit PIC with CLC",
                           "CC": "2517655",
                           "JLCC": "C510915"})

    icsp_conn = dop_part("CONN_01X05", "SIL5")
    gpios_dbg_conn = dop_part("CONN_01X03", "SIL3")

    mclr = Net("MCLR")

    mcu["Vdd"] += vin
    mcu["Vss"] += gnd
    bypass_cap(vin, gnd, "100nF", fields={"JLCC": "C14663"}, descr="PIC16")

    mcu[1] += mclr

    for name, net in pic_iface.items():
        if stm32_map[name] is None:
            continue
        mcu[stm32_map[name]] += net

    # Gpios
    mcu["RB2"] & gpios_dbg_conn[3] & gpios["gpio1"]
    mcu["RB3"] & gpios_dbg_conn[2] & gpios["gpio0"]
    gpios_dbg_conn[1] += gnd

    # Termination commands
    mcu["RB4"] += term_cmds["snes_term_cmd"]
    mcu["RB5"] += term_cmds["psx_term_cmd"]
    pull_updown(gnd, term_cmds["snes_term_cmd"], "330R",
                fields={"JLCC": "C23138"})
    pull_updown(gnd, term_cmds["psx_term_cmd"], "330R",
                fields={"JLCC": "C23138"})

    # ICSP connector
    icsp_conn[1] += mclr
    icsp_conn[2] += vin
    icsp_conn[3] += gnd
    mcu["RB6"] & icsp_conn[5] & pic_iface["icsp_clock"]
    mcu["RB7"] & icsp_conn[4] & pic_iface["icsp_data"]

    # RJ inputs
    mcu["RC4"] += rj45_sigs["RJ_SPI_CLK"]
    mcu["RC5"] += rj45_sigs["RJ_SPI_MOSI"]
    mcu["RC6"] += rj45_sigs["RJ_SPI_CS"]
    mcu["RC7"] += rj45_sigs["RJ_SPI_MISO"]

    # MCLR is connected through a serial resistor with an additional pullup
    # In order to sustain the potential 14V on MCLR from the pickit, we will
    # use a MOSFET to act as an open drain.
    # See Microchip TB087 for more details
    mos = dop_part("Q_NMOS_DGS", "SOT23",
                   fields={"Reference": "3LN01C-TB-H",
                           "Descr": "30V 150mA NMOS",
                           "CC": "2630394",
                           "JLCC": "C520561",
                           "JLROT": "180"})
    mos[1] += pic_iface["reset"]  # Gate
    mos[3] & get_res("100R", "0603", fields={"JLCC": "C22775"}) & mclr  # Drain
    mos[2] += gnd  # Source

    pull_updown(vin, mclr, "12K5", fields={"JLCC": "C22797"})
    pull_updown(gnd, pic_iface["reset"], "10K", fields={"JLCC": "C23138"})

    # debug test points
    for dbg in range(4):
        mcu[F"RA{dbg}"] & dop_part("TestPoint", "TestPoint_Pad_D1.5mm")

    # Unused pins
    mcu["RA7", "RB0", "RB1"] += NC
    return gpios, term_cmds
