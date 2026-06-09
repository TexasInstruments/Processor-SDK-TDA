# ##############################################################################
# # SYNOPSYS CONFIDENTIAL - This is an unpublished, confidential, and          #
# # proprietary work of Synopsys, Inc., and may be subject to patent,          #
# # copyright, trade secret, and other legal or contractual protection.        #
# # This work may be used only pursuant to the terms and conditions of a       #
# # written license agreement with Synopsys, Inc. All other use, reproduction, #
# # distribution, or disclosure of this work is strictly prohibited.           #
# ##############################################################################
#

import sys
import sim

# creating an event probe to trigger efuse scanning
notifier = sim.EventProbe("reset_notifier", True)

devstat_val = sys.argv[1]  # getting devstat value from command line argument


def py_efuse_scnanner_CB(observer):
    # this functions gets called when reset is de-asserted and notifies the event to start efuse scanning
    print("--In py_efuse_scanner_CB--")
    global notifier
    notifier.notify()
    reset_pin_bool.callback_on_value_change(1)


def update_devstat_val():
    global devstat_val
    print(devstat_val)

    MCU_DEVSTAT_path = sim.get_full_model_name(
        "/TDA5_System/TDA5_SoC/MCU_Domain/Periphs/MCU_CTRL_MMR_0/cfg0/MCU_DEVSTAT"
    )
    MCU_DEVSTAT_probe = sim.MemoryProbe(MCU_DEVSTAT_path, 4, 0)
    MCU_DEVSTAT_probe.set_value(int(devstat_val, 16))


try:
    print("#---In SEC_MGR test Py script---#")
    global reset_pin_bool

    # setting up callback on reset exit to notify the event which starts the efuse scanning
    reset_pin_probe = sim.get_full_model_name(
        "/TDA5_SoC/MCU_Domain/Security/ROT/SEC_MGR_0/p_por_early_reset_in"
    )
    reset_pin_bool = sim.BoolProbe(reset_pin_probe)
    reset_pin_bool.set_callback(py_efuse_scnanner_CB)
    reset_pin_bool.callback_on_value_change(1)

    while True:
        # waiting for event to get notified to start efuse scanning
        notifier.wait()
        print("---Scanning Efuse/FLash to SEC_MGR Memory--")
        # event  has been notified to starting with efuse scanning
        update_devstat_val()

except Exception as detail:
    print("Error: ", detail)

    sim.print_message("PY:- ERROR. DEV_STAT test Py script.")

sim.suspend_script()
