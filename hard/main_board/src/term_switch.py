#! /usr/bin/env python3
# -*- coding: utf-8 -*-


from skidl import Net
from lib.sch_utils import dop_part, bypass_cap, get_res, HC4066


def get_pu_res(value):
    # Uses 3.6K because no 3.5K available at JLCPCB
    if value == "3K5":
        return get_res("3K6", "0603", fields={"JLCC": "C22980"})
    else:
        return get_res("4K7", "0603", fields={"JLCC": "C23162"})


def term_switch(vio, v33, gnd, sigs):
    """Create a termination switcher
        sig format: sigs = {"name": [net, value, cmd_net, pu_net]}
    """

    tsw = HC4066(vio, gnd, ["100nF", "0603"], c_fields={"JLCC": "C14663"})

    for net_name in sorted(sigs):
        net, value, cmd_net, pu_net = sigs[net_name]
        res = get_pu_res(value)
        upper_net = Net("{0}_UPPER_TERM_SW".format(net_name))
        pu_net & res[1, 2] & upper_net
        tsw.add(cmd_net, net, upper_net)

    tsw.fill_unused()
