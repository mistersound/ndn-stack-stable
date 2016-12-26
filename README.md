NDN Stack Installing Instructions
================================
## Hardware and OS environment 

A laptop/desktop with a recent Linux distro (Ubuntu) is needed.
We recommend Ubuntu. For this guide, the _Ubuntu 14.04 LTS_ was used.
Also, note that you'll need administrative privileges.

Thanks to the Mini-NDN team for the initial script

## Installing NDN

The proposed NDN stack includes several components, a subset of these components can be installed to meet specific needs.

Each node needs to have the NDN stack installed to be able to talk NDN. 

### C++ library with eXperimental eXtensions

[ndn-cxx](https://github.com/named-data/ndn-cxx) is a C++ library, implementing NDN protocol. it provides the libraries needed to build and run other NDN applications.

### Named Data Networking Forwarding Daemon 

[NFD](https://github.com/named-data/nfd) is a network forwarder that implements NDN forwarding. 

### Named Data Link State Routing Protocol

[NLSR](https://github.com/named-data/NLSR) is a routing protocol in NDN that populates NDN's Routing Information Base.

### NDN Essential Tools

[ndn-tools](https://github.com/named-data/ndn-tools) is a collection of essential tools for NDN including :

* **peek** : transmit a single packet between a consumer and a producer
* **chunks** : segmented file transfer between a consumer and producer
* **ping** : test reachability between two nodes
* **dump** : analyze traffic on wire
* **dissect** : inspect TLV structure of NDN packet format
* **dissect-wireshark** : Wireshark extension to inspect TLV structure of NDN packets
* **pib** : a service to manage the public information of keys and publish certificates

### Dump CSV format

this fork of [ndndump](https://github.com/charifmahmoudi/ndndump) enable the generation of CSV files from the wire trafic capture.

### Mininet (NOT NDN but needed by Mini-NDN)

[Mininet](https://github.com/mininet/mininet) emulates a complete network of hosts, links, and switches on a single machine. it is useful to emulate a network on a single machine

### Mini-NDN
[Mini-NDN](https://github.com/named-data/mini-ndn) is a lightweight networking emulation tool for based on Mininet. 

### NS-3

[ns-3](http://www.nsnam.org/) is a discrete-event network simulator targeted for network simulation. This Custom [fork](https://github.com/cawka/ns-3-dev-ndnSIM) contains necessary patches for ndnSIM.

### ndnSIM

[ndnSIM](http://ndnsim.net/2.0/getting-started.html) is an NS-3 based NDN simulator

## Running the setup script

to install only one component (ndndump for instance)

    sudo ./install.sh -d

To install the full stack :

    sudo ./install.sh -mrftids
    
To list the possible options 

	./install.sh
	
The full list is 

	Usage: install.sh [-mfrtids]

	options:
	 -f: install NFD
	 -i: install mini-ndn
	 -m: install mininet and dependencies
	 -r: install NLSR
	 -t: install tools
	 -d: install ndndump
	 -s: install ndnSIM and dependencies
	

## Verification

You can use these steps to verify your installation.

### NFD

set up the configuration file

	sudo cp /usr/local/etc/ndn/nfd.conf.sample /usr/local/etc/ndn/nfd.conf

create a log folder 

	sudo mkdir -p /var/log/ndn/nfd
	sudo chown -R $USER /var/log/ndn/nfd

edit the start script

	sudo nano /usr/local/bin/nfd-start

replace the start command
	
	sudo /usr/local/bin/nfd &
	
with

	sudo /usr/local/bin/nfd 2>> /var/log/ndn/nfd/nfd.log&
	
start the daemon

	nfd-start

show the status

	nfd-status

### ndnSIM

go to ndnSIM folder
	
	cd ndnSIM/ns-3
	
run a sample experiment
	
	NS_LOG=ndn.Producer:ndn.Consumer ./waf --run=ndn-simple --vis


### Mini-NDN

Run the ping experiment

	sudo minindn --experiment=pingall --nPings=50
	
exit mininet once the experiment is finished

	mininet>exit
	
check the results in /tmp for example
	
	cat /tmp/a/ping-data/a.txt
	
