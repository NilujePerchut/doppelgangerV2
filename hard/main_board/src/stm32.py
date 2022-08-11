#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from skidl import Net
from lib.sch_utils import dop_part, bypass_cap, get_capa, get_res, pull_updown


def stm32(vin, gnd, switches, vio):
    """Set up the power stage."""

    sw_map = {"UP": "PA10",
              "DOWN": "PC1",
              "LEFT": "PA4",
              "RIGHT": "PA1",
              "SQUARE": "PC5",
              "TRIANGLE": "PA5",
              "R1": "PA6",
              "L1": "PA3",
              "CROSS": "PB0",
              "CIRCLE": "PC4",
              "R2": "PB1",
              "L2": "PA2",
              "L3": "PA0",
              "R3": "PA7",
              "START": "PC15",
              "SELECT": "PC13",
              "HOME": "PC14",
              }

    pic_iface = {"reset": Net("STM_TO_PIC_RESET"),
                 "icsp_clock": Net("STM_TO_PIC_ICSP_CLOCK"),
                 "icsp_data": Net("STM_TO_PIC_ICSP_DATA"),
                 "boot_cmd": Net("STM_TO_PIC_BOOT_CMD"),
                 # UART
                 "uart_tx": Net("STM_TO_PIC_UART_TX"),
                 "uart_rx": Net("STM_TO_PIC_UART_RX"),
                 # DATA[3:0]
                 "data0": Net("STM_TO_PIC_DATA0"),
                 "data1": Net("STM_TO_PIC_DATA1"),
                 "data2": Net("STM_TO_PIC_DATA2"),
                 "data3": Net("STM_TO_PIC_DATA3")}

    gpios = {"gpio0": Net("stm32_gpio0"),
             "gpio1": Net("stm32_gpio1")}

    usb = {"STM32_USB_P": Net("STM32_USB_P"),
           "STM32_USB_N": Net("STM32_USB_N")}

    mux_sel = {"mux_sel0": Net("mux_sel0"),
               "mux_sel1": Net("mux_sel1")}

    mcu = dop_part("STM32F405RGTX", "LQFP-64_10x10mm_P0.5mm",
                   fields={"Reference": "STM32F405RGT6",
                           "Descr": "ARM Cortex M4 ST version",
                           "CC": "2064363",
                           "JLCC": "C15742",
                           "JLROT": "90"})
    xtal = dop_part("Crystal", "Crystal_SMD_5032-2Pin_5.0x3.2mm_HandSoldering",
                    fields={"Reference": "QCL8.00000F18B23B",
                            "Descr": "Quartz 8MHz",
                            "CC": "2508448",
                            "JLCC": "C115962"})

    sw_conn = dop_part("CONN_01X04", "SIL4")
    pic_uart_conn = dop_part("CONN_01X03", "SIL3")
    serial_dbg_conn = dop_part("CONN_01X03", "SIL3")
    gpios_dbg_conn = dop_part("CONN_01X03", "SIL3")
    leds_conn = dop_part("CONN_01X04", "Bornier4_3.81mm")

    # Power
    mcu["VDD", "VDDA", "VBAT"] += vin
    mcu["VSS", "VSSA"] += gnd
    bypass_cap(vin, gnd, ["100nF"]*4, fields={"JLCC": "C14663"},
               descr="STM32F405")
    bypass_cap(vin, gnd, "4.7uF", package="0805", fields={"JLCC": "C1779"},
               descr="STM32F405")

    v_capas = [get_capa("2.2uF", "0805", fields={"JLCC": "C49217"})
               for i in range(2)]
    mcu["VCAP_1"] & v_capas[0][1, 2] & gnd
    mcu["VCAP_2"] & v_capas[1][1, 2] & gnd

    # Reset RC: 10K-100nF
    vin & get_res("10K", "0603", fields={"JLCC": "C25804"}) &\
        Net("STM32_RESETn") & mcu["NRST"] &\
        (get_capa("100nF", "0603", fields={"JLCC": "C14663"}) |\
         dop_part("SW_PUSH", "BP")) & gnd

    # XTAL
    # External capacitor value is given in ST AN2867: 2 (CL - CS)
    #     where CL is the crystal load capacitance
    #     where CS os the PCB capacitance (around 5-6pF)
    #     => 2 (20pF - 5pf) = 30pF
    mcu["PH0"] & Net("XTAL_IN") & xtal[1] &\
        get_capa("30pF", "0603", fields={"JLCC": "C22397"}) & gnd
    ext_res = get_res("0R", package="0603", fields={"JLCC": "C21189"})
    mcu["PH1"] & Net("XTAL_OUT") & ext_res[1, 2] & xtal[2] & \
        get_capa("30pF", "0603", fields={"JLCC": "C22397"}) & gnd

    # SWD interface
    sw_conn[1] += vin
    sw_conn[2] & Net("SWDIO") & mcu["PA13"]
    sw_conn[3] & Net("SWDCLK") & mcu["PA14"]
    sw_conn[4] += gnd

    # STM32 serial debug connector
    serial_dbg_conn[1] += gnd
    mcu["PB10"] & Net("DBG_TX") & serial_dbg_conn[3]
    dbg_rx = Net("DBG_RX")
    pull_updown(vin, dbg_rx, "10K", fields={"JLCC": "C25804"})
    mcu["PB11"] & dbg_rx & serial_dbg_conn[2]

    # PIC Interface
    mcu["PB6"] += pic_iface["uart_tx"]
    mcu["PB7"] += pic_iface["uart_rx"]
    mcu["PB5"] += pic_iface["icsp_clock"]
    mcu["PB4"] += pic_iface["icsp_data"]
    mcu["PB9"] += pic_iface["reset"]
    mcu["PB8"] += pic_iface["boot_cmd"]
    mcu["PA15"] += pic_iface["data0"]
    mcu["PC10"] += pic_iface["data1"]
    mcu["PC11"] += pic_iface["data2"]
    mcu["PC12"] += pic_iface["data3"]

    # Put PIC UART interface on a connector for debug purposes
    pic_uart_conn[1] += gnd
    pic_uart_conn[2] += pic_iface["uart_tx"]
    pic_uart_conn[3] += pic_iface["uart_rx"]

    # Buttons
    for bt_name in sw_map:
        mcu[sw_map[bt_name]] += switches[bt_name]

    # Gpios
    mcu["PA8"] & gpios_dbg_conn[3] & gpios["gpio0"]
    mcu["PA9"] & gpios_dbg_conn[2] & gpios["gpio1"]
    gpios_dbg_conn[1] += gnd

    # USB
    mcu["PA12"] += usb["STM32_USB_P"]
    mcu["PA11"] += usb["STM32_USB_N"]

    # BOOT signals
    boot0 = Net("BOOT0")
    mcu["BOOT0"] & boot0 & get_res("100K", package="0603",
                                   fields={"JLCC": "C25803"}) & gnd
    boot0 & dop_part("SW_PUSH", "BP") & vin

    mcu["PB2"] += gnd  # BOOT1

    # Leds
    leds_conn[1] += vin
    mcu["PC6"] & Net("LED_L") & get_res("330R", "0603",
                                        fields={"JLCC": "C23138"}) &\
                                        leds_conn[2]
    mcu["PC7"] & Net("LED_R") & get_res("330R", "0603",
                                        fields={"JLCC": "C23138"}) &\
                                        leds_conn[3]
    leds_conn[4] += gnd

    # Mux Selection
    mcu["PC9"] += mux_sel["mux_sel0"]
    mcu["PC8"] += mux_sel["mux_sel1"]

    # Brook power command
    brook_power_cmd = Net("BROOK_POWER_CMD")
    mcu["PC2"] += brook_power_cmd
    pull_updown(vin, brook_power_cmd, "10K", fields={"JLCC": "C25804"})
    # Also provide a signal that acts as a placeholder
    brook_power_switch_faultn = Net("BROOK_POWER_FAULTN")
    mcu["PC3"] += brook_power_switch_faultn

    # Vio analog (simple divider by 2)
    r_ana = [get_capa("10K", "0603", fields={"JLCC": "C25804"})
             for i in range(2)]
    vio & r_ana[0] & mcu["PC0"] & r_ana[1] & gnd

    # Not connected
    not_connected = ["PB3", "PB12", "PB13", "PB14", "PB15", "PD2"]
    for nc in not_connected:
        mcu[nc] & dop_part("TestPoint", "TestPoint_Pad_D1.5mm")

    return pic_iface, gpios, usb, mux_sel, brook_power_cmd,\
        brook_power_switch_faultn
