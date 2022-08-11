#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net
from src.rj45 import rj45
from lib.sch_utils import unit_map_on_he10, run_unit


def unit_rj45():
    """Unit test the power stage"""
    # Input signals & connector

    power, usb, other_sigs = rj45()
    out_signals = {**power, **usb, **other_sigs}
    unit_map_on_he10(out_signals.values())


if __name__ == "__main__":
    run_unit(unit_rj45)
