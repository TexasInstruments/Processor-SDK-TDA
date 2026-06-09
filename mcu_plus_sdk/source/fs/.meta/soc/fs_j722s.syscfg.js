let common = system.getScript("/common");

const topModules_wkup = [
    "/fs/freertos_fat/freertos_fat",
];

const topModules_r5f = [
    "/fs/freertos_fat/freertos_fat",
];
const topModules_c75 = [
    "/fs/freertos_fat/freertos_fat",
];
exports = {
    getTopModules: function() {
        let topModules = topModules_r5f;
        if(common.getSelfSysCfgCoreName().includes("wkup-r5fss0-0"))
        {
            topModules = topModules_wkup;
        }
        else if(common.getSelfSysCfgCoreName().includes("r5fss"))
        {
            topModules = topModules_r5f;
        }
        else if(common.getSelfSysCfgCoreName().includes("c75ss"))
        {
            topModules = topModules_c75;
        }

        return topModules;

    }
}