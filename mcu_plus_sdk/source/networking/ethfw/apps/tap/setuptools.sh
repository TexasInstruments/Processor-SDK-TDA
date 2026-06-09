#
# Utility script to install all dependant components not included in the
# SDK
#
# - Make sure wget, apt-get, curl proxies are set
#   if required to access these tools from behind corporate firewall.
# - Tested on Ubuntu 18.04
#
# IMPORANT NOTE:
#   TI provides this as is and may not work as expected on all systems,
#   Please review the contents of this script and modify as needed
#   for your environment
#
#

WGET_PATH=https://developer.arm.com/-/media/Files/downloads/gnu-a/9.2-2019.12/binrel
COMP_VER=gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu

echo "[$COMP_VER] Checking ..."
if [ ! -d $COMP_VER ]
then
    echo "[$COMP_VER] Installing ..."
    wget $WGET_PATH/$COMP_VER.tar.xz --no-check-certificate
    tar xf $COMP_VER.tar.xz > /dev/null
    rm $COMP_VER.tar.xz
fi
echo "[$COMP_VER] Done"

