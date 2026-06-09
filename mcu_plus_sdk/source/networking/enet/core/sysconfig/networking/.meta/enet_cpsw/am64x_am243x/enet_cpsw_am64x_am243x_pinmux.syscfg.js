
let common = system.getScript("/common");
let pinmux = system.getScript("/drivers/pinmux/pinmux");
let device = common.getDeviceName();
let soc = system.getScript(`/networking/soc/networking_${common.getSocName()}`);

function getInstanceConfig(moduleInstance) {
    return {
        ...moduleInstance,
    };
};

function getPeripheralRequirements(inst, peripheralName, name)
{
    let interfaceName = getInterfaceName(inst, peripheralName);
    let resources = [];
    let pinResource = {};

    if (name == "RGMII")
    {
        if (inst.enablexmii1 == true)
        {
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_RD0", "RGMII1_RD0");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_RD1", "RGMII1_RD1");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_RD2", "RD2");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_RD3", "RD3");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_RX_CTL", "RX_CTL");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_RXC", "RX_RXC");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_TD0", "TD0");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_TD1", "TD1");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_TD2", "TD2");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_TD3", "TD3");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_TX_CTL", "TX_CTL");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII1_TXC", "TX_RXC");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
        }

        if (inst.enablexmii2 == true)
        {
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_RD0", "RGMII2_RD0");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_RD1", "RD1");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_RD2", "RD2");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_RD3", "RD3");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_RX_CTL", "RX_CTL");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_RXC", "RX_RXC");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_TD0", "TD0");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_TD1", "TD1");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_TD2", "TD2");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_TD3", "TD3");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_TX_CTL", "TX_CTL");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RGMII2_TXC", "TX_RXC");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
        }
    }
    else if (name == "RMII")
    {
        pinResource = pinmux.getPinRequirements(interfaceName, "RMII_REF_CLK", "REF_CLK");
        pinmux.setConfigurableDefault( pinResource, "rx", true );
        resources.push( pinResource);
        if (inst.enablexmii1 == true)
        {
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII1_CRS_DV", "CRS_DV");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII1_RX_ER", "RX_ER");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII1_RXD0", "RXD0");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII1_RXD1", "RXD1");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII1_TXD0", "TXD0");
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII1_TXD1", "TXD1");
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII1_TX_EN", "TX_EN");
            resources.push( pinResource);
        }

        if (inst.enablexmii2 == true)
        {
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII2_CRS_DV", "CRS_DV");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII2_RX_ER", "RX_ER");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII2_RXD0", "RXD0");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII2_RXD1", "RXD1");
            pinmux.setConfigurableDefault( pinResource, "rx", true );
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII2_TXD0", "TXD0");
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII2_TXD1", "TXD1");
            resources.push( pinResource);
            pinResource = pinmux.getPinRequirements(interfaceName, "RMII2_TX_EN", "TX_EN");
            resources.push( pinResource);
        }
    }
    else if (name == "CPSW_CPTS")
    {
       pinResource = pinmux.getPinRequirements(interfaceName, "TS_SYNC", "TS_SYNC");
       resources.push( pinResource);
    }
    else
    {
        let pinList = getInterfacePinList(inst, interfaceName);
        for(let pin of pinList)
        {
            pinResource = pinmux.getPinRequirements(interfaceName, pin);

            /* make all pins as "rx" and then override to make "rx" as false as needed  */
            pinmux.setConfigurableDefault( pinResource, "rx", true );

            resources.push( pinResource );
        }
    }

    let peripheralRequirements = {
        name: name,
        displayName: name,
        interfaceName: interfaceName,
        resources: resources,
    };

    return peripheralRequirements;
}

function getInterfaceName(inst, peripheralName)
{
    return `${peripheralName}`;
}

function getInterfacePinList(inst, peripheralName)
{
    let interfaceName = getInterfaceName(inst, peripheralName);
    let pinList = [];

    pinList = pinmux.getInterfacePinList(interfaceName);

    return pinList;
}


function pinmuxRequirements(inst) {

    let perRequirements = [];

    if (inst.enableTsOut === true)
    {
        let cptsTsSync = getPeripheralRequirements(inst, "CPSW3G", "CPSW_CPTS");
        pinmux.setPeripheralPinConfigurableDefault( cptsTsSync, "CPSW3G", "rx", false);
        perRequirements.push(cptsTsSync);
    }

    let mdio = getPeripheralRequirements(inst, "MDIO", "MDIO");

    /* set default values for "rx" for different pins, based on use case */
    pinmux.setPeripheralPinConfigurableDefault( mdio, "MDC", "rx", false);
    perRequirements.push(mdio);

    if (inst.phyToMacInterfaceMode === "RMII")
    {
        let rmii = getPeripheralRequirements(inst, "CPSW", "RMII");
        pinmux.setPeripheralPinConfigurableDefault( rmii, "RMII1_TXD0", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( rmii, "RMII1_TXD1", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( rmii, "RMII1_TXEN", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( rmii, "RMII2_TXD0", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( rmii, "RMII2_TXD1", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( rmii, "RMII2_TXEN", "rx", false);
        perRequirements.push(rmii);
    }
    else
    {
        let rgmii1 = getPeripheralRequirements(inst, "CPSW", "RGMII");
        perRequirements.push(rgmii1);
    }
    return perRequirements;
}

function getInterfaceNameList(inst) {
    let interfaceNameList = []
    if (inst.enableTsOut === true)
    {
        interfaceNameList.push("CPSW_CPTS")
    }
    interfaceNameList.push(getInterfaceName(inst, "MDIO"));
    if (inst.phyToMacInterfaceMode === "RMII")
    {
        interfaceNameList.push(getInterfaceName(inst, "RMII"));
    }
    else
    {
        interfaceNameList.push(getInterfaceName(inst, "RGMII"));
    }

    return interfaceNameList;
}

function getPeripheralPinNames(inst)
{
    let pinList = [];

    if (inst.enableTsOut === true)
    {
        pinList = pinList.concat("TS_SYNC");
    }

    let cpswPinList =  getInterfacePinList(inst, "CPSW");
    let xmiipinList = [];
    if (inst.phyToMacInterfaceMode === "RMII")
    {
        for (let pin of cpswPinList)
        {
            if (pin.startsWith("RMII"))
            {
                xmiipinList.push(pin);
            }
        }
    }
    else
    {
        for (let pin of cpswPinList)
        {;
            if (pin.startsWith("RGMII"))
            {
                xmiipinList.push(pin);
            }
        }
    }
    pinList = pinList.concat(getInterfacePinList(inst, "MDIO"), xmiipinList);

    return pinList;
}

let enet_cpsw_pinmux_module = {
    displayName: "CPSW pinmux config",
    longDescription: `This configures CPSW module pinmux`,
    config: [
        {
            name: "phyToMacInterfaceMode",
            displayName: "RMII/RGMII",
            default: "RGMII",
            options: [
                {
                    name: "RMII",
                },
                {
                    name: "RGMII",
                },
            ],
        },
        {
            name: "enableTsOut",
            displayName: "Enable CPTS TS Output",
            default: false,
            hidden: false,
        },
        {
            name: "enablexmii1",
            description: "Enable port 1 specific RGMII/RMII/MII.Change 'Disable MAC Port' status in 'MAC Port Config' to change this",
            displayName: "Enable Port1 R/G/MII For CPSW",
            default: true,
            hidden: false,
            readOnly: true,
        },
        {
            name: "enablexmii2",
            description: "Enable port 2 specific RGMII/RMII/MII.Change 'Disable MAC Port' status in 'MAC Port Config' to change this",
            displayName: "Enable Port1 R/G/MII For CPSW",
            default: true,
            hidden: false,
            readOnly: true,
        },
    ],
    collapsed: false,
};

exports = {
    config: enet_cpsw_pinmux_module,
    pinmuxRequirements,
    getInterfaceNameList,
    getPeripheralPinNames,
};
