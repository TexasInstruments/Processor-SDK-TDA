#!/usr/bin/env python2.7
import sys
import os
import shutil

devices = ["j721e","j721s2", "j784s4","am62a","j722s", "j742s2", "tda54"]

USER_GUIDE_DOC_PATH = "../../docs/doxygen/user_guide_pages/tidl_user_guide_doxygen.cfg"
USER_GUIDE_NDA_DOC_PATH = "../../docs/doxygen/user_guide_nda_pages/tidl_user_guide_nda_doxygen.cfg"
C7X_CONFIG_REL_PATH = "/../../../makerules/config.mk"
ARM_CONFIG_REL_PATH = "/../../../arm-tidl/config.mk"
TIDL_CONFIG_PATH_REL = "../../../ti_dl/test/testvecs/config"
TIDL_CUSTOMER_PATH_REL = "../../../ti_dl/test/testvecs/config/customers"
TIDL_IMPORT_PATH_REL = "../../../ti_dl/utils/tidlModelImport"
###########################################################################################
#J721E:
j721eData = {
    "config_list.txt":"config_list_j721e.txt",
    "config_list_full_test.txt":"config_list_full_test_j721e.txt", 
    "customers_infer_list.txt":"customers_infer_list_j721e.txt",
    "customers_infer_list_full_test.txt":"customers_infer_list_full_test_j721e.txt",
    "customers_import_list.sh":"customers_import_list_j721e.sh",
    "customers_import_list_full_test.sh":"customers_import_list_full_test_j721e.sh",
    "import_all_models.sh":"import_all_models_j721e.sh",
    "import_models_full_test.sh":"import_models_full_test_j721e.sh"
    }
#J721S2
j721s2Data = {
    "config_list.txt":"config_list_j721s2.txt",
    "config_list_full_test.txt":"config_list_full_test_j721s2.txt", 
    "customers_infer_list.txt":"customers_infer_list_j721s2.txt",
    "customers_infer_list_full_test.txt":"customers_infer_list_full_test_j721s2.txt",
    "customers_import_list.sh":"customers_import_list_j721s2.sh",
    "customers_import_list_full_test.sh":"customers_import_list_full_test_j721s2.sh",
    "import_all_models.sh":"import_all_models_j721s2.sh",
    "import_models_full_test.sh":"import_models_full_test_j721s2.sh"
    }
#J784S4
j784s4Data = {
    "config_list.txt":"config_list_j784s4.txt",
    "config_list_full_test.txt":"config_list_full_test_j784s4.txt", 
    "customers_infer_list.txt":"customers_infer_list_j784s4.txt",
    "customers_infer_list_full_test.txt":"customers_infer_list_full_test_j784s4.txt",
    "customers_import_list.sh":"customers_import_list_j784s4.sh",
    "customers_import_list_full_test.sh":"customers_import_list_full_test_j784s4.sh",
    "import_all_models.sh":"import_all_models_j784s4.sh",
    "import_models_full_test.sh":"import_models_full_test_j784s4.sh"
    }
#J742S2
j742s2Data = {
    "config_list.txt":"config_list_j742s2.txt",
    "config_list_full_test.txt":"config_list_full_test_j742s2.txt", 
    "customers_infer_list.txt":"customers_infer_list_j742s2.txt",
    "customers_infer_list_full_test.txt":"customers_infer_list_full_test_j742s2.txt",
    "customers_import_list.sh":"customers_import_list_j742s2.sh",
    "customers_import_list_full_test.sh":"customers_import_list_full_test_j742s2.sh",
    "import_all_models.sh":"import_all_models_j742s2.sh",
    "import_models_full_test.sh":"import_models_full_test_j742s2.sh"
    }
#AM62A:
am62aData = {
    "config_list.txt":"config_list_am62a.txt",
    "config_list_full_test.txt":"config_list_full_test_am62a.txt", 
    "customers_infer_list.txt":"customers_infer_list_am62a.txt",
    "customers_infer_list_full_test.txt":"customers_infer_list_full_test_am62a.txt",
    "customers_import_list.sh":"customers_import_list_am62a.sh",
    "customers_import_list_full_test.sh":"customers_import_list_full_test_am62a.sh",
    "import_all_models.sh":"import_all_models_am62a.sh",
    "import_models_full_test.sh":"import_models_full_test_am62a.sh"
    }
#J722S:
j722sData = {
    "config_list.txt":"config_list_j722s.txt",
    "config_list_full_test.txt":"config_list_full_test_j722s.txt", 
    "customers_infer_list.txt":"customers_infer_list_j722s.txt",
    "customers_infer_list_full_test.txt":"customers_infer_list_full_test_j722s.txt",
    "customers_import_list.sh":"customers_import_list_j722s.sh",
    "customers_import_list_full_test.sh":"customers_import_list_full_test_j722s.sh",
    "import_all_models.sh":"import_all_models_j722s.sh",
    "import_models_full_test.sh":"import_models_full_test_j722s.sh"
    }
#TDA54:
tda54Data = {
    "config_list.txt":"config_list_tda54.txt",
    "config_list_full_test.txt":"config_list_full_test_am62a.txt", 
    "customers_infer_list.txt":"customers_infer_list_am62a.txt",
    "customers_infer_list_full_test.txt":"customers_infer_list_full_test_am62a.txt",
    "customers_import_list.sh":"customers_import_list_am62a.sh",
    "customers_import_list_full_test.sh":"customers_import_list_full_test_am62a.sh",
    "import_all_models.sh":"import_all_models_tda54.sh",
    "import_models_full_test.sh":"import_models_full_test_am62a.sh"
    }


deviceDict = { 'j721e': j721eData, 'j721s2': j721s2Data, 'j784s4': j784s4Data, 'j742s2': j742s2Data,'am62a': am62aData, 'j722s': j722sData, 'tda54': tda54Data}
##########################################################################################
def createCopy(device, symLinkRelPath, symLinkName, fileFolder):
    #Update the symbolic link to the correct device:
    cur_path = os.getcwd()
    symlink_path = cur_path + os.path.sep + symLinkRelPath
    try:
        os.chdir(symlink_path)
    except:
        print("Skipping (directory not found): " + symlink_path)
        return
    sourceFile = fileFolder + os.path.sep + deviceDict[device][symLinkName]
    if not os.path.exists(sourceFile):
        print("Skipping (source not found): " + sourceFile)
        try:
            os.chdir(cur_path)
        except:
            print("Error returning to script path " + symLinkName)
        return
    #Delete old file
    try:
        os.remove(symLinkName)
    except:
        print("Error deleting old file: " + symLinkName)
    print("Moving : " + sourceFile)

    try:
        shutil.copy(sourceFile, symLinkName)
        #os.symlink(sourceFile, symLinkName)
    except:
        print("Failure while creating a symbolic link to: " + sourceFile)

    #Return to the older path:
    try:
        os.chdir(cur_path)
    except:
        print("Error returning to script path " + symLinkName)
        return

def createSymLink(device, symLinkRelPath, symLinkName, fileFolder):
    #Update the symbolic link to the correct device:
    cur_path = os.getcwd()
    symlink_path = cur_path + os.path.sep + symLinkRelPath
    try:
        os.chdir(symlink_path)
    except:
        print("Error switching to directories to " + symlink_path)
    #Delete symbolic link
    print("Updating symbolic links")
    try:
        os.remove(symLinkName)
    except:
        print("Error deleting old symbolic link to " + symLinkName)
    sourceFile = fileFolder + os.path.sep + deviceDict[device][symLinkName]
    print("Creating a symbolic link to : " + sourceFile)

    try:
        os.symlink(sourceFile, symLinkName)
    except:
        print("Failure while creating a symbolic link to: " + sourceFile)
    
    #Return to the older path:
    try:
        os.chdir(cur_path)
    except:
        print("Error returning to script path " + symLinkName)
        return

def updateDeviceConfig(device):
    #Update the symbolic link to the correct device:
    cur_path = os.getcwd()
    symlink_path = cur_path + "/../../../ti_dl/test/testvecs/config/import/"
    try:
        os.chdir(symlink_path)
    except:
        print("Error switching to import directory")

    #Delete symbolic link
    print("Updating symbolic links")
    try:
        os.remove("./device_config.cfg")
    except:
        print("Error deleting old symbolic link to device_config")
    if device == "j721s2":
        config_j721s2 = "./device_configs/j721s2_config.cfg"
        os.symlink(config_j721s2, "device_config.cfg")
    elif device == "j721e":
        config_j721e = "./device_configs/j721e_config.cfg"
        os.symlink(config_j721e, "device_config.cfg")
    elif device == "j784s4":
        config_j784s4 = "./device_configs/j784s4_config.cfg"
        os.symlink(config_j784s4, "device_config.cfg")
    elif device == "j742s2":
        config_j742s2 = "./device_configs/j742s2_config.cfg"
        os.symlink(config_j742s2, "device_config.cfg")
    elif device == "am62a":
        config_am62a = "./device_configs/am62a_config.cfg"
        os.symlink(config_am62a, "device_config.cfg")
    elif device == "j722s":
        config_j722s = "./device_configs/j722s_config.cfg"
        os.symlink(config_j722s, "device_config.cfg")
    elif device == "tda54":
        config_tda54 = "./device_configs/tda54_config.cfg"
        os.symlink(config_tda54, "device_config.cfg")
    #Return to the older path:
    try:
        os.chdir(cur_path)
    except:
        print("Error returning to script path <device_configuration>")
        return

def findAndUpdateTargetSOC(lineList, device, relPath):
    stringPos = -1
    idx = 0
    targetSOC = ["TARGET_SOC","?="]
    print("Attempting to update TARGET_SOC in ", relPath)
    for line in lineList:
        configList = line.split(" ")
        configList = [word for word in configList if word]
        if targetSOC[0] in configList:
            indexSOC = configList.index(targetSOC[0])
            if targetSOC[1] == configList[indexSOC + 1]:
                #print("TARGET_SOC ?= Found")
                stringPos = idx
                break
            if stringPos != -1:
                break
        if stringPos != -1:
            break
        idx = idx + 1
    if stringPos != -1:
        print("Updating device in " + relPath +" for: " + str(device))
        lineList[idx] = "TARGET_SOC ?= " + str(device)
    else:
        print("Unable to find TARGET_SOC in ", str(relPath))
    return lineList

def updateConfigMake(device, relPath):
    #CurrentPath:
    cur_path = os.getcwd()
    config_mk_path = cur_path + relPath
    data = []
    with open(config_mk_path, "r") as conf:
        confdata = conf.read()
        data = confdata.split("\n")
        data = findAndUpdateTargetSOC(data, device, relPath)
    #Form file and write out updated config
    confdata_updated = "\n".join(data)
    with open(config_mk_path, "w") as conf:
        conf.write(confdata_updated)

def updateConfigLists(device):
    #Update the symbolic link to the correct device:
    cur_path = os.getcwd()
    symlink_path = cur_path + "/../../../ti_dl/test/testvecs/config/"
    try:
        os.chdir(symlink_path)
    except:
        print("Error switching to the infer directory")




def updateDocCfg(device, docCfg, pathBase):
    #pathBase = user_guide_html_ or user_guide_nda_html_
    if not os.path.exists(docCfg):
        print("Skipping doc config (not found): " + docCfg)
        return
    with open(docCfg,"r") as conf:
        confData = conf.read()
        data = confData.split("\n")
        idx = 0
        for line in data:
            outString = line.find("HTML_OUTPUT")
            if outString != -1:
                if line[0] == '#':
                    idx = idx + 1
                    continue
                print("Updating doc path")
                data[idx] = "HTML_OUTPUT            = ../../" + pathBase + device
            outString = line.find("ENABLED_SECTIONS")
            if outString != -1:
                if line[0] == '#':
                    idx = idx + 1
                    continue
                print("Updating doc define")
                data[idx] = "ENABLED_SECTIONS       = " + str(device).upper()
            idx = idx + 1
        confdata_updated = "\n".join(data)
        with open(docCfg, "w") as conf:
            conf.write(confdata_updated)


def configureTIDL(device):
    updateConfigMake(device, C7X_CONFIG_REL_PATH)
    updateConfigMake(device, ARM_CONFIG_REL_PATH)
    #############################################
    #Symbolic Links:
    updateDeviceConfig(device)
    #Update Config Lists:
    createCopy(device, TIDL_CONFIG_PATH_REL, "config_list.txt", "config_lists")
    createCopy(device, TIDL_CONFIG_PATH_REL, "config_list_full_test.txt", "config_lists")
    #Update Import Lists:
    createCopy(device, TIDL_IMPORT_PATH_REL, "import_all_models.sh", "imports")
    createCopy(device, TIDL_IMPORT_PATH_REL, "import_models_full_test.sh", "imports")
    #Update Customer Lists:
    #Config:
    createCopy(device, TIDL_CUSTOMER_PATH_REL, "customers_infer_list.txt", "customer_infers")
    createCopy(device, TIDL_CUSTOMER_PATH_REL, "customers_infer_list_full_test.txt", "customer_infers")
    #Import:
    createCopy(device, TIDL_CUSTOMER_PATH_REL, "customers_import_list.sh", "customer_imports")
    createCopy(device, TIDL_CUSTOMER_PATH_REL, "customers_import_list_full_test.sh", "customer_imports")
    ############################################
    updateDocCfg(device,USER_GUIDE_DOC_PATH,"user_guide_html_")
    updateDocCfg(device,USER_GUIDE_NDA_DOC_PATH,"user_guide_nda_html_")

#Linux only - Validated on Ubuntu 18.03, can only be run from the python/ folder in TIDL
nArgs = len(sys.argv)
if nArgs <= 1:
    print("Insufficient Arguments")
else:
    #Consider only the first device
    device2configure = sys.argv[1].lower()
    if device2configure in devices:
        configureTIDL(device2configure)
    else:
        print("Unsupported Device")