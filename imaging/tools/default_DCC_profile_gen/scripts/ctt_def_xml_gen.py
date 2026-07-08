#!/usr/bin/python

import os
import utils
import sys
import dccxml
import dccxml_awb
import dccxml_rgb2rgb
import dccxml_cfai
import dccxml_h3a_aewb
import dccxml_ldc
import dccxml_lsc
import dccxml_viss_blc
import dccxml_glbce
import dccxml_viss_pcid
import dccxml_nsf4
import dccxml_decompand
import dccxml_h3amux
import dccxml_gamma
from utils import g_tuning_output

version= '0.7'

def init_generic_params(sparams):
    #
    #    GENERIC SENSOR PARAMS
    #
    property_num = (
        'SENSOR_ID',
        'SENSOR_WIDTH',
        'SENSOR_HEIGHT',
        'WDR_MODE',
        'COLOR_PATTERN',
        'BIT_DEPTH',
        'WDR_BIT_DEPTH',
        'GAMMA_PRE',
        'H3A_INPUT_LSB',
    )
    property_num_opt = (
        'YUV_GAMMA',

        'LDC_SENSOR_PIXEL_PITCH_IN_MM',
        'LDC_OUTPUT_VIEW_SCALER',
        'LDC_MESH_DOWNSAMPLE_M',
        'LDC_INPUT_WIDTH',
        'LDC_INPUT_HEIGHT',
        'LDC_INPUT_H_CENTER',
        'LDC_INPUT_V_CENTER',


        'LSC_SENSOR_PIXEL_PITCH',
        'LSC_INPUT_H_CENTER',
        'LSC_INPUT_V_CENTER',
        'LSC_INPUT_WIDTH',
        'LSC_INPUT_HEIGHT',
    )

    property_str = (
        'SENSOR_NAME',
        'SENSOR_DCC_NAME',
        'WDR_KNEE_X',
        'WDR_KNEE_Y',
    )

    property_str_opt = (
        'LDC_LENS_SPEC_FILE',
        'LSC_LENS_SPEC_FILE',
        'RAW_RGB2RGB',
    )

    property_pattern_opt = (
        #TODO_Gang: add photo-space support
        r'(RAW_RGB2RGB)_(\d+)_(\d+)K',
        # Gang_TODO: auto tune
    )
    property_int_or_str = (
        'BLACK_PRE',
        'BLACK_POST',
    )

    params = {k : int(sparams[k]) for k in property_num}
    for k in property_num_opt:
        if k in sparams:
            try:
                params[k] = int(sparams[k])
            except ValueError:
                params[k] = float(sparams[k])

    for k in property_str:
        params[k] = sparams[k]

    for k in property_str_opt:
        if k in sparams:
            params[k] = sparams[k]

    for k in property_int_or_str:
        if sparams[k].isdigit():
            params[k] = int(sparams[k])
        else:
            params[k] = sparams[k]

    for pat in property_pattern_opt:
        for k in sparams:
            import re
            m = re.match(pat, k)
            if m:
                key = m.group(1)
                cls = int(m.group(2))
                val = int(m.group(3))
                if key not in params:
                    params[key] = {}
                params[key][cls] = (sparams[k], val)

    #print(params['RAW_RGB2RGB'])

    params['VERSION'] = 0
    params['VENDOR_ID'] = 1

    wdr_names = ['linear', 'wdr']
    params['WDR_MODE'] = wdr_names[params['WDR_MODE']]
    
    return params

def init_dcc_regions(sys_params,Params):
    #
    region = {}
    region['PREVIEW'] = {}
    region['PREVIEW']['NUM_REGIONS'] = 1
    region['PREVIEW']['NUM_VARS']    = 3
    region['PREVIEW']['VAR_ID']      = [ 2,  1,   0]
    region['PREVIEW']['RGN_LIMITS']  = [((0,250000), (0, 100000), (0, 20000))]
    #
    #    DCC REGION DEFINITION
    #
    Params['NUM_USE_CASE'] = 1
    Params['USE_CASE'] = {}
    Params['SYS_PARAMS'] = {}
    Params['USE_CASE'][0] = {}
    #
    #    DCC REGION DEFINITION
    #
    #   ID = 65535
    Params['USE_CASE'][0]['ID'] = 65535
    Params['USE_CASE'][0]['RGN_DEFN'] = {}
    Params['USE_CASE'][0]['RGN_DEFN'] = region['PREVIEW']

    return

def generate_pcid_xml(sys_params,pcid_params,params):
    pcid_File = 'viss_pcid'
    pcid_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_viss_pcid.GenVISS_PCIDXML(pcid_Dir, pcid_File, params, pcid_params)
    return

def generate_nsf4_xml(sys_params,nsf4_params,params):
    nsf4_File = 'viss_nsf4'
    nsf4_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_nsf4.GenNSF4XML(nsf4_Dir, nsf4_File, params, nsf4_params)
    return

def generate_glbce_xml(sys_params,glbce_params,params):
    glbce_File = 'wdr_glbce_dcc'
    glbce_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_glbce.GenGLBCEXML(glbce_Dir, glbce_File, params, glbce_params)
    return

def generate_viss_blc_xml(sys_params,viss_blc_params,params):
    viss_blc_File = 'viss_blc'
    viss_blc_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_viss_blc.GenVISS_BLCXML(viss_blc_Dir, viss_blc_File, params, viss_blc_params)
    return

def generate_ldc_xml(sys_params,ldc_params,params):
    ldc_File = 'mesh_ldc_dcc'
    ldc_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_ldc.GenLDCXML(ldc_Dir, ldc_File, params, ldc_params)
    return

def generate_lsc_xml(sys_params,lsc_params,params):
    lsc_File = 'rawfe_lsc_dcc'
    lsc_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_lsc.GenLSCXML(lsc_Dir, lsc_File, params, lsc_params)
    return

def generate_h3a_aewb_xml(sys_params,h3a_aewb_params,params):
    H3A_AEWB_File = 'h3a_aewb_dcc'
    H3A_AEWB_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_h3a_aewb.GenH3A_AEWBXML(H3A_AEWB_Dir, H3A_AEWB_File, params, h3a_aewb_params)
    return

def generate_h3amux_xml(sys_params,h3amux_params,params):
    h3amux_File = 'h3a_mux_luts_dcc'
    h3amux_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_h3amux.GenH3AMUXXML(h3amux_Dir, h3amux_File, params, h3amux_params)
    return

def generate_cfai_xml(sys_params, cfai_params, params):
    CFAI_File = 'cfa_dcc'
    CFAI_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_cfai.GenCFAIXML(CFAI_Dir, CFAI_File, params, cfai_params)
    return

def generate_rgb2rgb1_xml(sys_params,rgb2rgb1_params,params):
    # setup multiple regions if needed
    if 'RAW_RGB2RGB' in params:
        n = 1
        if isinstance(params['RAW_RGB2RGB'], dict):
            n = len(params['RAW_RGB2RGB'])
        else:
            params['RAW_RGB2RGB'] = {0: (params['RAW_RGB2RGB'], 5000)}

        if n > 1:
            regions = rgb2rgb1_params['USE_CASE'][0]['RGN_DEFN']
            regions['NUM_REGIONS'] = n
            regions['RGN_LIMITS'] = []
            for i in range(n):
                regions['RGN_LIMITS'].append([ [0, 250000], [0, 100000], [0, 20000] ])
            for i in range(n-1):
                regions['RGN_LIMITS'][i+0][2][1] = (params['RAW_RGB2RGB'][i][1] + params['RAW_RGB2RGB'][i+1][1]) // 2
                regions['RGN_LIMITS'][i+1][2][0]= regions['RGN_LIMITS'][i+0][2][1] + 1

    RGB2RGB1_File = 'rgb2rgb_dcc'
    RGB2RGB1_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_rgb2rgb.GenRGB2RGBXML(RGB2RGB1_Dir, RGB2RGB1_File, params, rgb2rgb1_params)
    return

def generate_awb_xml(sys_params,awb_params,params):
    awb_File = 'awb_alg_ti3_tuning'
    awb_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_awb.GenAWBXML(awb_Dir, awb_File, params, awb_params)
    return

def generate_decmp_xml(sys_params,decmp_params,params):
    decmp_File = params['WDR_MODE'] + '_decompand_dcc'
    decmp_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']
    dccxml_decompand.GenDECMPXML(decmp_Dir, decmp_File, params, decmp_params)
    return

def generate_gamma_xml(sys_params, gamma_params, params):
    gamma_File = 'viss_gamma_dcc'
    gamma_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/' + params['WDR_MODE']
    dccxml_gamma.GenGAMMAXML(gamma_Dir, gamma_File, params, gamma_params)
    return

def generate_dcc_gen_script(sys_params, params):
    dcc_gen_File = 'generate_dcc.sh'
    dcc_gen_Dir  = sys_params['PRJ_DIR'] + '/dcc_xmls/'+params['WDR_MODE']

    if (os.path.exists(dcc_gen_Dir) == False):
        print ('Creating directory:  %s\n' %dcc_gen_Dir)
        try:
            os.makedirs(dcc_gen_Dir)
        except OSError as err:
            utils.error('%s: %s' %(err.strerror, err.filename), skip=True)

    filename = os.path.join(dcc_gen_Dir, '%s' %(dcc_gen_File));
    print ('Creating XML File:  %s\n' %filename)

    handle = dccxml.OpenFile(filename)
    if(params['WDR_MODE'] == 'linear'):
        wdr_suffix = ""
    else:
        wdr_suffix = "_wdr"

    handle.write('DCC_TOOL_PATH=../../../../../tools/dcc_tools/\n')
    handle.write('OUT_PATH=../../../../include\n')
    handle.write('\n')
    handle.write('rm -f *.bin\n')

    handle.write('bin_folder=../../dcc_bins/\n')
    handle.write('if [ ! -d "$bin_folder" ]\n')
    handle.write('then\n')
    handle.write('    mkdir "$bin_folder"\n')
    handle.write('fi\n')
    handle.write('\n')
    handle.write('bin_folder=../../dcc_bins/%s/\n' %params['SENSOR_DCC_NAME'])
    handle.write('if [ ! -d "$bin_folder" ]\n')
    handle.write('then\n')
    handle.write('    mkdir "$bin_folder"\n')
    handle.write('fi\n')
    handle.write('\n')
    handle.write('bin_folder=../../dcc_bins/%s/%s/\n' %(params['SENSOR_DCC_NAME'], params['WDR_MODE']))
    handle.write('if [ ! -d "$bin_folder" ]\n')
    handle.write('then\n')
    handle.write('    mkdir "$bin_folder"\n')
    handle.write('fi\n')
    handle.write('\n')

    handle.write('rm -f $OUT_PATH/dcc_viss_%s%s.h\n' %(params['SENSOR_NAME'], wdr_suffix))
    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_rgb2rgb_dcc.xml\n' %params['SENSOR_NAME'])
    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_h3a_aewb_dcc.xml\n' %params['SENSOR_NAME'])
    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_viss_nsf4.xml\n' %params['SENSOR_NAME'])
    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_viss_blc.xml\n' %params['SENSOR_NAME'])
    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_cfa_dcc.xml\n' %params['SENSOR_NAME'])
    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_viss_gamma_dcc.xml\n' %params['SENSOR_NAME'])

    if 'LSC_LENS_SPEC_FILE' in sys_params:
        handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_rawfe_lsc_dcc.xml\n' %params['SENSOR_NAME'])

    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_%s_decompand_dcc.xml\n' %(params['SENSOR_NAME'], params['WDR_MODE']))
    if(params['WDR_MODE'] == 'wdr'):
        handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_wdr_glbce_dcc.xml\n' %params['SENSOR_NAME'])
    if(params['COLOR_PATTERN_4x4'] > 0):
        handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_viss_pcid.xml\n' %params['SENSOR_NAME'])

    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_h3a_mux_luts_dcc.xml\n' %params['SENSOR_NAME'])
    handle.write('cp *.bin %s/\n' %"$bin_folder")
    handle.write('cat *.bin > ../../dcc_bins/dcc_viss%s.bin\n' %wdr_suffix)
    handle.write('$DCC_TOOL_PATH/dcc_bin2c ../../dcc_bins/dcc_viss%s.bin $OUT_PATH/dcc_viss_%s%s.h dcc_viss_%s%s\n' %(wdr_suffix, params['SENSOR_NAME'], wdr_suffix, params['SENSOR_NAME'], wdr_suffix))
    handle.write('echo; echo\n')
    handle.write('rm -f *.bin\n')

    handle.write('rm -f $OUT_PATH/dcc_2a_%s%s.h\n' %(params['SENSOR_NAME'], wdr_suffix))
    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_awb_alg_ti3_tuning.xml\n' %params['SENSOR_NAME'])
    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_h3a_aewb_dcc.xml\n' %params['SENSOR_NAME'])
    handle.write('cp *.bin %s/\n' %"$bin_folder")
    handle.write('cat *.bin > ../../dcc_bins/dcc_2a%s.bin\n'  %wdr_suffix)
    handle.write('$DCC_TOOL_PATH/dcc_bin2c ../../dcc_bins/dcc_2a%s.bin $OUT_PATH/dcc_2a_%s%s.h dcc_2a_%s%s\n' %(wdr_suffix, params['SENSOR_NAME'], wdr_suffix, params['SENSOR_NAME'], wdr_suffix))
    handle.write('echo; echo\n')
    handle.write('rm -f *.bin\n')
    
    handle.write('rm -f $OUT_PATH/dcc_ldc_%s%s.h\n' %(params['SENSOR_NAME'], wdr_suffix))
    handle.write('$DCC_TOOL_PATH/dcc_gen_linux %s_mesh_ldc_dcc.xml\n' %params['SENSOR_NAME'])
    handle.write('cp *.bin %s/\n' %"$bin_folder")
    handle.write('cat *.bin > ../../dcc_bins/dcc_ldc%s.bin\n' %wdr_suffix)
    handle.write('$DCC_TOOL_PATH/dcc_bin2c ../../dcc_bins/dcc_ldc%s.bin $OUT_PATH/dcc_ldc_%s%s.h dcc_ldc_%s%s\n' %(wdr_suffix, params['SENSOR_NAME'], wdr_suffix, params['SENSOR_NAME'], wdr_suffix))
    handle.write('echo; echo\n')
    handle.write('rm -f *.bin\n')

    return

def defaultXMLGen_Simulator(sensor_config_file):
    print('\n Default Components : Version [%s]' %version)
    print('\n Config File  %s' %sensor_config_file)
    """
    System Parameters Related to Project
    """
    sys_params = utils.get_params(sensor_config_file)
    
    GEN_Params = init_generic_params(sys_params)
    wdr_mode = GEN_Params['WDR_MODE']
    rgbir = GEN_Params['COLOR_PATTERN']
    
    GEN_Params['COLOR_PATTERN_4x4'] = -1
    if 10 <= rgbir < 18:
        PCID_Params = {}
        init_dcc_regions(sys_params, PCID_Params)
        generate_pcid_xml(sys_params, PCID_Params, GEN_Params)

        GEN_Params['COLOR_PATTERN_4x4'] = GEN_Params['COLOR_PATTERN']
        GEN_Params['COLOR_PATTERN'] = dccxml_viss_pcid.GenVISS_PCID_Bayer_out(rgbir)

    viss_blc_Params = {}
    init_dcc_regions(sys_params, viss_blc_Params)
    generate_viss_blc_xml(sys_params, viss_blc_Params, GEN_Params)
    
    DECMP_Params = {}
    init_dcc_regions(sys_params, DECMP_Params)
    generate_decmp_xml(sys_params, DECMP_Params, GEN_Params)

    RGB2RGB1_Params = {}
    init_dcc_regions(sys_params, RGB2RGB1_Params)
    generate_rgb2rgb1_xml(sys_params, RGB2RGB1_Params, GEN_Params)

    AWB_Params = {}
    init_dcc_regions(sys_params, AWB_Params)
    generate_awb_xml(sys_params, AWB_Params, GEN_Params)

    CFAI_Params = {}
    init_dcc_regions(sys_params, CFAI_Params)
    generate_cfai_xml(sys_params, CFAI_Params, GEN_Params)

    H3A_AEWB_Params = {}
    init_dcc_regions(sys_params, H3A_AEWB_Params)
    generate_h3a_aewb_xml(sys_params, H3A_AEWB_Params, GEN_Params)

    LDC_Params = {}
    init_dcc_regions(sys_params, LDC_Params)
    generate_ldc_xml(sys_params, LDC_Params, GEN_Params)

    LSC_Params = {}
    init_dcc_regions(sys_params, LSC_Params)
    if 'LSC_LENS_SPEC_FILE' in sys_params:
        generate_lsc_xml(sys_params, LSC_Params, GEN_Params)

    NSF4_Params = {}
    init_dcc_regions(sys_params, NSF4_Params)
    generate_nsf4_xml(sys_params, NSF4_Params, GEN_Params)

    H3AMUX_Params = {}
    init_dcc_regions(sys_params, H3AMUX_Params)
    generate_h3amux_xml(sys_params, H3AMUX_Params, GEN_Params)

    GAMMA_Params = {}
    init_dcc_regions(sys_params, GAMMA_Params)
    generate_gamma_xml(sys_params, GAMMA_Params, GEN_Params)

    if wdr_mode == 'wdr':
        GLBCE_Params = {}
        init_dcc_regions(sys_params, GLBCE_Params)
        generate_glbce_xml(sys_params, GLBCE_Params, GEN_Params)
    
    generate_dcc_gen_script(sys_params, GEN_Params)
    
    return

if __name__ == '__main__':
    if (len(sys.argv) < 2 ) :
        utils.error('Usage: python ctt_def_xml_gen.py <sensor_properties_filename>')
    else:
        sensor_config_file = sys.argv[1]

        defaultXMLGen_Simulator(sensor_config_file)
        
