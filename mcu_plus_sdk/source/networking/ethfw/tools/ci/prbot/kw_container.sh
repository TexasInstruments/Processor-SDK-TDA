export PATH="$PATH:/home/mcusdkdev/ti/mcusdk_kw_new_install/klocwork/bin"

# then cd to the path which is mounted 
cd mcu_sdk

npm config set proxy http://webproxy.ext.ti.com:80 
npm config set https-proxy http://webproxy.ext.ti.com:80 
npm config set registry https://registry.npmjs.org/ 
npm config set strict-ssl false 
npm install


make kw-setup 
make kw-all TI_SDK_DEVICE=am13x
# run the make kw commands and generate report 
