#############################################################################
# Portions contained herein copyright 1996-2025 Synopsys, Inc.              #
#                                                                           #
# This Synopsys software and all associated documentation are proprietary   #
# to Synopsys, Inc. and may only be used pursuant to the terms and          #
# conditions of a written license agreement with Synopsys, Inc.             #
# Synopsys licensees may modify this code as necessary for use with their   #
# products and/or Synopsys licensed products, and may distribute such       #
# modifications to other Synopsys licensees for the same purpose.           #
# All other use, reproduction, modification, or distribution of the         #
# Synopsys software or the associated documentation is strictly prohibited. #
#############################################################################

import os
import platform
import subprocess

from armsp.sim_shared.plugins import info

SCREEN = (
    "TDA5_System.TDA5_SoC.MCU_Domain.Designware_IP.VIRTIO_GPU.virtio_gpu_tlm_0.SCREEN0"
)


def _get_extapps_dir():
    vpconfig_dir = info.get_vp_config_dir()
    for rel in ("../../extapps", "../extapps"):
        extapps = os.path.abspath(os.path.join(vpconfig_dir, rel))
        if os.path.isdir(extapps):
            return extapps
    snps_vp_home = os.environ.get("SNPS_VP_HOME", "")
    if snps_vp_home:
        extapps = os.path.abspath(os.path.join(snps_vp_home, "extapps"))
        if os.path.isdir(extapps):
            return extapps
    return None


starter_key = os.environ.get("COWARE_SIM_STARTER_KEY", "")
extapps = _get_extapps_dir()
if starter_key and extapps:
    launch_script = "launch.sh" if platform.system() == "Linux" else "launch.bat"
    atps2lcd = (
        "ATPS2LCD_NOKMI_SDL2"
        if platform.system() == "Linux"
        else "ATPS2LCD_NOKMI_SDL2.exe"
    )
    launch = os.path.join(extapps, launch_script)
    applet = os.path.join(extapps, atps2lcd)
    if os.path.isfile(applet):
        cmd = [
            launch,
            applet,
            "--starter-key",
            starter_key,
            "-d",
            SCREEN,
            "-t",
            SCREEN,
            "--client-app-class",
            SCREEN,
        ]
        print("Launching virtio GPU display at begin of simulation: %s" % cmd)
        subprocess.Popen(cmd)
    else:
        print("ATPS2LCD binary not found: %s" % applet)
else:
    print(
        "Skipping virtio GPU display launch (starter_key=%s extapps=%s)"
        % (bool(starter_key), extapps)
    )
