#
#
# Copyright (c) 2021 Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided # that the following conditions are met:
#
# #       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
# #       any redistribution and use are licensed by TI for use only with TI Devices.
#
# #       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
# #       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
# #       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

#!/bin/bash
# Shell Script to add the tap device and start the
# user-space application.
# All configuration data will be fetched from the
# configuration file located at:
CONFIG_FILE_PATH="/etc/J7ic.conf"
# Load the variables from the config file using source
source $CONFIG_FILE_PATH

# Help function to display usage instructions
display_help() {
	echo	"Usage:" >&2
	echo
	echo	"-a		Name for the TAP device to be created and used"
	echo	"-b		Receiving Queue ID"
	echo	"-c		Transmitting Queue ID"
	echo	"-d		Total number of Queues in shared memory"
	echo	"-e		Base address of queue region in hex format"
	echo	"-f		Length of Queue Region in hex format "
	echo	"		[Enter 0 for automatic calculation]"
	echo	"-g		Polling interval for Transmit and Receive tasks in microseconds"
	echo	"-h		Display help"
	echo	"-i		Total number of Bufpools in shared memory"
	echo	"-j		Base address of bufpool region in hex format"
	echo	"-k		Length of Bufpool Region in hex format" 
	echo	"		[Enter 0 for automatic calculation]"
	echo	"-l		Transmitting Bufpool ID"
	echo	"-m		MAC address for TAP device"
	echo	"-n		IP address for TAP device" 
	echo	"		[Pass empty string \"\" for Dynamic IP]"
	echo	"-o		Maximum number of frames to transmit per wakeup cycle" 
	echo	"		[Must be >= 1]"
	echo	"-p		Maximum number of frames to receive per wakeup cycle"
	echo	"		[Must be >= 1]"
	echo 
	exit 1
}

# Load the Queue and Bufpool Regions'                                           
# Base addresses and lengths from the                                           
# device-tree8
for memory in /proc/device-tree/reserved-memory/*
do
        if [[ -f $memory/status ]];
        then
                region_status=`tr -d '\0' < $memory/status`
                if [[ "$region_status" -eq "disabled" ]];
                then
                    continue;
                fi
        fi
        NAME="$(cut -d'/' -f5 <<<$memory)"
        NUMBER="$(echo "$NAME" | tr -cd '@' | awk '{ print length; }')"
        if [[ "$NUMBER" -eq 1 ]];
        then
                REGION_NAME="$(echo "$(cut -d'@' -f1 <<<$NAME)")"
                if [[ "$REGION_NAME" == "r5f-virtual-eth-queues" || "$REGION_NAME" == "vision-apps-r5f-virtual-eth-queues" ]];
                then
                        ICQ_BASE_ADDR="$(echo 0x"$(cut -d'@' -f2 <<<$NAME)")"
                        echo "Discovered Queue Base Address at $ICQ_BASE_ADDR from device tree"
                        ICQ_MEM_LEN="0x$(xxd -p -s 12 $memory/reg)"
                        echo "And the Queue region length is $ICQ_MEM_LEN"
                elif [[ "$REGION_NAME" == "r5f-virtual-eth-buffers" || "$REGION_NAME" == "vision-apps-r5f-virtual-eth-buffers" ]];
                then
                        BUFPOOL_BASE_ADDR="$(echo 0x"$(cut -d'@' -f2 <<<$NAME)")"
                        echo "Discovered Bufpool Base Address at $BUFPOOL_BASE_ADDR from device tree"
                        BUFPOOL_MEM_LEN="0x$(xxd -p -s 12 $memory/reg)"
                        echo "And the Bufpool region length is $BUFPOOL_MEM_LEN"
                fi
        fi
done

# Parse command line arguments
while getopts ":a:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:" opt; do
	case ${opt} in
		a ) # process option TAP_DEVICE_NAME
			TAP_DEVICE_NAME=$OPTARG
			;;
		b ) # process option ICQ_MCU2_0_TO_A72_ID
			ICQ_MCU2_0_TO_A72_ID=$OPTARG
			;;
		c ) # process option ICQ_A72_TO_MCU2_0_ID
			ICQ_A72_TO_MCU2_0_ID=$OPTARG
			;;
		d ) # process option ICQ_MAX_NUM_QUEUES
			ICQ_MAX_NUM_QUEUES=$OPTARG
			;;
		e ) # process option ICQ_BASE_ADDR
			ICQ_BASE_ADDR=$OPTARG
			;;
		f ) # process option ICQ_MEM_LEN
			ICQ_MEM_LEN=$OPTARG
			;;
		g ) # process option Q_POLLING_INTERVAL_MICROSECONDS
			Q_POLLING_INTERVAL_MICROSECONDS=$OPTARG
			;;
		h ) # process option HELP
			display_help
			;;
		i ) # process option BUFPOOL_MAX_NUM_POOLS
			BUFPOOL_MAX_NUM_POOLS=$OPTARG
			;;
		j ) # process option BUFPOOL_BASE_ADDR
			BUFPOOL_BASE_ADDR=$OPTARG
			;;
		k ) # process option BUFPOOL_MEM_LEN
			BUFPOOL_MEM_LEN=$OPTARG
			;;
		l ) # process option BUFPOOL_A72_ID
			BUFPOOL_A72_ID=$OPTARG
			;;
		m ) #process option TAP_MAC
			TAP_MAC=$OPTARG
			;;
		n ) #process option TAP_IP
			TAP_IP=$OPTARG
			;;
		o ) #process option MAX_NUM_PACKET_TRANSMITS_PER_CYCLE
			MAX_NUM_PACKET_TRANSMITS_PER_CYCLE=$OPTARG
			;;		
		p) #process option MAX_NUM_PACKET_RECEIVES_PER_CYCLE
			MAX_NUM_PACKET_RECEIVES_PER_CYCLE=$OPTARG
			;;
		: ) # option argument missing
			if [ $OPTARG == h ]
			then
				display_help
			else
				echo "Invalid option: $OPTARG requires an argument" 1>&2
				echo
				display_help
			fi
			exit ;;
		\? )  # incorrect option
			echo "Incorrect Usage!" 1>&2
			echo
			display_help
			;;
	esac
done
shift $((OPTIND -1))

echo "------------------------------------------------"
echo "Selected Configuration:"
echo "------------------------------------------------"
echo "TAP Device name $TAP_DEVICE_NAME"
echo "RX Queue Id $ICQ_MCU2_0_TO_A72_ID"
echo "TX Queue Id $ICQ_A72_TO_MCU2_0_ID"
echo "Maximum number of queues $ICQ_MAX_NUM_QUEUES"
echo "Queue Base $ICQ_BASE_ADDR"
echo "Queue Len $ICQ_MEM_LEN"
echo "Polling interval $Q_POLLING_INTERVAL_MICROSECONDS"
echo "Maximum number of bufpools $BUFPOOL_MAX_NUM_POOLS"
echo "Bufpool base $BUFPOOL_BASE_ADDR"
echo "Bufpool len $BUFPOOL_MEM_LEN"
echo "TX Bufpool ID $BUFPOOL_A72_ID"
echo "TAP IP $TAP_IP"
echo "TAP MAC $TAP_MAC"
echo "MAX TX $MAX_NUM_PACKET_TRANSMITS_PER_CYCLE"
echo "MAX RX $MAX_NUM_PACKET_RECEIVES_PER_CYCLE"
echo "------------------------------------------------"

# Create the TAP device with the name from config file
ip tuntap add mode tap dev $TAP_DEVICE_NAME
# Fetch the IP address from DHCP server
# if IP address in config file is empty
# Else use static IP address from config file
ip link set address $TAP_MAC dev $TAP_DEVICE_NAME
ip link set dev $TAP_DEVICE_NAME up
# Now the TAP device is up and we can start the
# user-space application, passing it config values
./tapif "$TAP_DEVICE_NAME" "$ICQ_MCU2_0_TO_A72_ID" "$ICQ_A72_TO_MCU2_0_ID" "$ICQ_MAX_NUM_QUEUES" "$ICQ_BASE_ADDR" "$ICQ_MEM_LEN" "$Q_POLLING_INTERVAL_MICROSECONDS" "$BUFPOOL_MAX_NUM_POOLS" "$BUFPOOL_BASE_ADDR" "$BUFPOOL_MEM_LEN" "$BUFPOOL_A72_ID" "$MAX_NUM_PACKET_TRANSMITS_PER_CYCLE"	 "$MAX_NUM_PACKET_RECEIVES_PER_CYCLE" &
APPLICATION_PID=$!
# Wait for a second to let the user-space application start
sleep 1
# Check if IP address is statically assigned
# or has to be fetched dynamically
if [ -z "$TAP_IP" ]
then
	echo "Fetching IP address from DHCP server"
	udhcpc -i $TAP_DEVICE_NAME
else
	echo "Setting IP address from configuration file"
	ip addr add $TAP_IP/24 dev $TAP_DEVICE_NAME
fi
# Wait till application is killed
wait $APPLICATION_PID
echo "TAP Application has been stopped in an unsafe manner!"
echo "Starting Cleanup"
/bin/bash /usr/bin/cleantapif.sh
echo "Cleanup completed!"
