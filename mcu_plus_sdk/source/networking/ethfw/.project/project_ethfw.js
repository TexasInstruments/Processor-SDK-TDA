function getComponentProperty(device)
{
    return require(`./project_${device}_ethfw`).getComponentProperty();
};

function getComponentBuildProperty(buildOption)
{
    return require(`./project_${buildOption.device}_ethfw`).getComponentBuildProperty(buildOption);
};

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
