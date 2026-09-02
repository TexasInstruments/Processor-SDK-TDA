*** PyTI_PSDK_RTOS ***

This is a collection of python utilities and APIs for
Processor SDK RTOS Autmotive

Installation
============
1. Install Python 3.5+ and pip (https://www.python.org/)
   - On Ubuntu 16.04 this can be done via below command
     # sudo apt-get update
     # sudo apt-get install python3.5 curl
     # curl "https://bootstrap.pypa.io/get-pip.py" -o "get-pip.py"
     # python3.5 get-pip.py
     # rm get-pip.py
   - Do below, to confirm "python" and "pip" are in your install path
     # python3.5 --version
     # pip --version

3. Install this module by executing below command at the folder in which this README.txt is located,
   # pip install -e . --user

4. Now this python module can be imported and used in your python script by doing below,
   from ti_psdk_rtos_tools import *
