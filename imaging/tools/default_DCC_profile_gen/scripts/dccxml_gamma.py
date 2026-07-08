"""
    DCC XML Generator Functions

    YUV Gamma
"""

import os
import dcc
import dccxml
import shutil
import utils

def GenGAMMAParams(handle, gamma_params, cls_id, tuning_params):

    handle.write('                        1     // enable\n')
    handle.write('                        10    // bitdepth\n')
    handle.write('                        {\n')
    handle.write('                            #include "lut_viss_fcc_gamma.txt"\n')
    handle.write('                        }\n')

def GenGAMMAXML(directory, filebase, params, gamma_params):

    if (os.path.exists(directory) == False):
        print ('Creating directory:  %s\n' %directory)
        try:
            os.makedirs(directory)
        except OSError as err:
            utils.error('%s: %s' %(err.strerror, err.filename), skip=True)

    filename = os.path.join(directory, '%s_%s.xml' %(params['SENSOR_NAME'], filebase));
    print ('Creating XML File:  %s\n' %filename)

    module_params = {}
    module_params['NAME']   = 'VISS_GAMMA_cfg'
    module_params['STRUCT_NAME'] = 'cfg_viss_gamma'
    module_params['DCC_ID'] = dcc.DCC_ID_VISS_FCC_GAMMA
    module_params['FUNC_GENPARAMS'] = GenGAMMAParams

    handle = dccxml.OpenFile(filename)
    dccxml.GenHeader(handle, params, module_params)

    # Create the DCC Structure definition
    handle.write('  <!--=======================================================================-->\n')
    handle.write('  <typedef>\n')
    handle.write('      <cfg_viss_gamma type="struct">\n')
    handle.write('        <gamma_cfg_enable     type="uint32">          </gamma_cfg_enable>\n')
    handle.write('        <gamma_cfg_bitdepth   type="uint32">          </gamma_cfg_bitdepth>\n')
    handle.write('        <gamma_cfg_lut        type="uint32[513]">     </gamma_cfg_lut>\n')
    handle.write('      </cfg_viss_gamma>\n')
    handle.write('  </typedef>\n')
    handle.write('  <!--=======================================================================-->\n')

    # Create a DCC Use Case
    for i in range(gamma_params['NUM_USE_CASE']):
        module_params['tuning_params'] = {
                        'BIT_DEPTH': params['BIT_DEPTH'],
                }
        dccxml.AddUseCase(handle, module_params, gamma_params['USE_CASE'][i])

    r_table = '../tables/lut_gamma_BT709.txt'
    if params.get('YUV_GAMMA', 0) == 1:
        r_table = '../tables/lut_gamma_high_contrast.txt'

    shutil.copy(r_table, os.path.join(directory, 'lut_viss_fcc_gamma.txt'))


    dccxml.GenFooter(handle, module_params)
    dccxml.CloseFile(handle)
