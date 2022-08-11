#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net
from lib.sch_utils import dop_part


def mechs():
    """Gathers all mechanical parts (fixes, fiducials, sakura)"""

    [dop_part("FIDUCIAL", "FIDUCIAL") for i in range(3)]
    [dop_part("Fix", "FIX") for i in range(2)]
    [dop_part("Fix", "JLCPCB_TOOLING_HOLE") for i in range(3)]
    [dop_part("Sakura", "Sakura_solder_mask") for i in range(2)]
