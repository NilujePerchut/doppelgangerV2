#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net, POWER
from src.pic16 import pic16
from lib.sch_utils import unit_map_on_he10, run_unit


def unit_pic16():
    """Unit test the pic16 part"""
    power = {"vin": Net("vin"),
             "gnd": Net("gnd")}

    stm32_sigs = ["reset", "uart_tx", "uart_rx", "data0", "data1",
                  "data2", "data3", "icsp_clock", "icsp_data", "boot_cmd"]
    stm32_nets = {name: Net(name) for name in stm32_sigs}

    rj45_nets = {name: Net(name)
                 for name in ["RJ_SPI_CLK", "RJ_SPI_MOSI", "RJ_SPI_MISO",
                              "RJ_SPI_CS"]}

    power["vin"].drive = POWER
    power["gnd"].drive = POWER

    in_sigs = {**power, **stm32_nets, **rj45_nets}
    unit_map_on_he10(in_sigs.values())

    gpios, terms_cmd = pic16(power["vin"], power["gnd"],
                             stm32_nets, rj45_nets)

    out_sigs = {**gpios, **terms_cmd}
    unit_map_on_he10(out_sigs.values())


if __name__ == "__main__":
    run_unit(unit_pic16)
