#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net
from src.mechs import mechs
from lib.sch_utils import run_unit


def unit_mechs():
    """Unit test the mechanical parts"""
    mechs()


if __name__ == "__main__":
    run_unit(unit_mechs)
