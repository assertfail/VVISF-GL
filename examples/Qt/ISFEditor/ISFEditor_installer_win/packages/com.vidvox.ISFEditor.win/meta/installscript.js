
function Component()
{
}

Component.prototype.createOperations = function()
{
    //  call default implementation to actually install the files
    component.createOperations();

    if (systemInfo.productType === "windows")    {
        component.addOperation("CreateShortcut",
                               "@TargetDir@/ISFEditor.exe",
                               "@StartMenuDir@/ISFEditor.lnk",
                               "description=The ISF Editor");
    }
}
