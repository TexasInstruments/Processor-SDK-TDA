#!/bin/sh

# eth1 is assumed to be the Virtual Switch Port Interface
# tap0 is assumed to be the Shared Memory Interface
# The following commands create and setup the bridge interface
# as the master of eth1 and tap0.

TAP_IF_DIR="/sys/class/net/tap0"
get_operstate () {
	echo $(cat $TAP_IF_DIR/operstate);
}

# Wait for tap0 to be created
while [ ! -d "$TAP_IF_DIR" ]
do
	sleep 1;
done

# Wait for tap0 to be up
operstate=$(get_operstate);
while [ "$operstate" != "up" ]
do
	sleep 1;
	operstate=$(get_operstate);
done

# Bring down eth1 and tap0
ifconfig eth1 down
ifconfig tap0 down

# Create the bridge interface br0
brctl addbr br0

# Enable Spanning Tree Protocol
brctl stp br0 on

# Add br0 as the master of eth1 and tap0
brctl addif br0 eth1
brctl addif br0 tap0

# Assign eth1's MAC Address to br0
mac_eth1=$(cat /sys/class/net/eth1/address);
ifconfig br0 hw ether $mac_eth1

# Bring up all interfaces
ifconfig eth1 up
ifconfig tap0 up
ifconfig br0 up

# Fetch IP Address for bridge interface
udhcpc -i br0

# Assign bridge interface's IP Address to eth1
# in order to register bridge's IP with EthFw
ip_br=$(ifconfig br0 | grep "inet " | awk '{print $2}');
ifconfig eth1 $ip_br
