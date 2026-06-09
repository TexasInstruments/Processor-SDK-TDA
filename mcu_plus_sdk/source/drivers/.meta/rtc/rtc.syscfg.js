let common = system.getScript("/common");
let soc = system.getScript(`/drivers/soc/drivers_${common.getSocName()}`);

function getModule() {

    let driverVer = soc.getDriverVer("rtc");

    return system.getScript(`/drivers/rtc/${driverVer}/rtc_${driverVer}`);
}

exports = getModule();
