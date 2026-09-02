cd /ti_fs/vision_apps 

PROCESSOR=`uname -a`
if [[ ${PROCESSOR} == *_"J721S2"_* ]]; then
    # Disable unused devices for ECU_BUILD
    k3conf disable device 179 # CODEC0
    k3conf disable device 360 # USB0
    k3conf disable device 28  # CPSW1

    k3conf disable device 170 # PBIST3
    k3conf disable device 171 # PBIST0
    k3conf disable device 172 # PBIST1
    k3conf disable device 173 # PBIST4
    k3conf disable device 178 # MCU_PBIST2
    k3conf disable device 177 # MCU_PBIST1
    k3conf disable device 176 # MCU_PBIST0

    k3conf disable device 2   # ATL0
    k3conf disable device 117 # GPMC0
    k3conf disable device 95  # ELM0
    k3conf disable device 98  # MMCSD0
    k3conf disable device 276 # PCIE1
    k3conf disable device 154 # DSS_DSI0
    k3conf disable device 214 # I2C0
    k3conf disable device 224 # NAVSS0

    k3conf disable device 0   # MCU_ADC0
    k3conf disable device 1   # MCU_ADC1
    k3conf disable device 154 # DSS_DSI0
    k3conf disable device 207 # MCU_MCAN0
    k3conf disable device 208 # MCU_MCAN1
    k3conf disable device 108 # MCU_FSS0_HYPERBUS
    k3conf disable device 109 # MCU_OSPI0
    k3conf disable device 110 # MCU_OSPI1

elif [[ ${PROCESSOR} == *_"J784S4"_* ]] || [[ ${PROCESSOR} == *_"J742S2"_* ]]; then
    # Disable unused devices for ECU_BUILD
    k3conf disable device 241 # CODEC0
    k3conf disable device 242 # CODEC2
    k3conf disable device 398 # USB0
    k3conf disable device 62  # CPSW1
    k3conf disable device 64  # CPSW_9XUSS_J7AM0

    k3conf disable device 234 # PBIST4
    k3conf disable device 233 # PBIST1
    k3conf disable device 232 # PBIST0
    k3conf disable device 231 # PBIST3
    k3conf disable device 239 # MCU_PBIST1
    k3conf disable device 238 # MCU_PBIST0
    k3conf disable device 240 # MCU_PBIST2

    k3conf disable device 332 # PCIE0
    k3conf disable device 333 # PCIE1
    k3conf disable device 334 # PCIE2
    k3conf disable device 335 # PCIE3

    k3conf disable device 2   # ATL0
    k3conf disable device 169 # GPMC0
    k3conf disable device 130 # ELM0
    k3conf disable device 140 # MMCSD0
    k3conf disable device 215 # DSS_DSI0

    k3conf disable device 273 # I2C3
    k3conf disable device 272 # I2C2
    k3conf disable device 271 # I2C1
    k3conf disable device 270 # I2C0

    k3conf disable device 387 # UFS0
    k3conf disable device 263 # MCU_MCAN0
    k3conf disable device 264 # MCU_MCAN1
    k3conf disable device 0   # MCU_ADC0
    k3conf disable device 1   # MCU_ADC1
    k3conf disable device 170 # MCU_I3C0
    k3conf disable device 161 # MCU_OSPI0
    k3conf disable device 162 # MCU_OSPI1
    k3conf disable device 160 # MCU_FSS0_HYPERBUS
    k3conf disable device 405 # SERDES_10G1
    k3conf disable device 406 # SERDES_10G2
fi

. ./vision_apps_init.sh 
sleep 8 
( sleep 3; echo "a14" ) | ./run_app_tidl_avp4.sh

