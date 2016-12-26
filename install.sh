#!/bin/bash
test -e /etc/debian_version && DIST="Debian"
grep Ubuntu /etc/lsb-release &> /dev/null && DIST="Ubuntu"

if [[ $DIST == Ubuntu || $DIST == Debian ]]; then
    update='sudo apt-get update'
    install='sudo apt-get -y install'
    remove='sudo apt-get -y remove'
    pkginst='sudo dpkg -i'
    # Prereqs for this script
    if ! which lsb_release &> /dev/null; then
        $install lsb-release
    fi
fi


function forwarder {
    if [[ $cxx != true ]]; then
        ndncxx
        cxx="true"
    fi

    if [[ $DIST == Ubuntu || $DIST == Debian ]]; then
        $install libpcap-dev pkg-config
    fi


    git clone --depth 1 https://github.com/named-data/NFD
    cd NFD
    ./waf configure --without-websocket
    ./waf
    sudo ./waf install
    cd ../
}

function routing {
    if [[ $cxx != true ]]; then
        ndncxx
        cxx="true"
    fi

    if [[ $DIST == Ubuntu ]]; then
        $install liblog4cxx10-dev libprotobuf-dev protobuf-compiler
    fi


    git clone --depth 1 https://github.com/named-data/NLSR
    cd NLSR
    ./waf configure
    ./waf
    sudo ./waf install
    cd ../
}

function ndncxx {
    if [[ updated != true ]]; then
        $update
        updated="true"
    fi

    if [[ $DIST == Ubuntu || $DIST == Debian ]]; then
        $install git libsqlite3-dev libboost-all-dev make g++
        crypto
    fi


    git clone --depth 1 https://github.com/named-data/ndn-cxx
    cd ndn-cxx
    ./waf configure
    ./waf
    sudo ./waf install
    sudo ldconfig
    cd ../
}

function crypto {
    mkdir crypto
    cd crypto
    $install unzip
    wget http://www.cryptopp.com/cryptopp562.zip
    unzip cryptopp562.zip

    # Uncomments flags to build shared object
    sed -i '/^# CXXFLAGS += -fPIC/s/^# //' GNUmakefile

    make static dynamic
    sudo make install
    cd ../
}

function tools {
    if [[ $cxx != true ]]; then
        ndncxx
        cxx="true"
    fi

    git clone --depth 1 https://github.com/named-data/ndn-tools
    cd ndn-tools
    ./waf configure
    ./waf
    sudo ./waf install
    cd ../
}

function ndndump {
    if [[ $cxx != true ]]; then
        ndncxx
        cxx="true"
    fi

    git clone --depth 1 https://github.com/charifmahmoudi/ndndump
    cd ndndump
    ./waf configure
    ./waf
    sudo ./waf install
    cd ../
}

function mininet {
    if [[ updated != true ]]; then
        $update
        updated="true"
    fi

    if [[ $pysetup != true ]]; then
        pysetup="true"
    fi

    git clone --depth 1 https://github.com/mininet/mininet
    cd mininet
    sudo ./util/install.sh -fnv
    cd ../
}

function ndnsim {
	if [[ $cxx != true ]]; then
        ndncxx
        cxx="true"
    fi
    
    if [[ updated != true ]]; then
        $update
        updated="true"
    fi

    if [[ $pysetup != true ]]; then
        pysetup="true"
    fi
    
    if [[ $DIST == Ubuntu ]]; then
        $install python-dev python-pygraphviz python-kiwi
        $install python-pygoocanvas python-gnome2
        $install python-rsvg ipython
    fi

	mkdir ndnSIM
	cd ndnSIM
#	git clone https://github.com/named-data/ndn-cxx.git ndn-cxx
	git clone https://github.com/cawka/ns-3-dev-ndnSIM.git ns-3
	git clone https://github.com/cawka/pybindgen.git pybindgen
	git clone --recursive https://github.com/named-data/ndnSIM.git ns-3/src/ndnSIM
	
#	cd ndnSIM/ndn-cxx
#	./waf configure
#	./waf
#	sudo ./waf install
	
	cd pybindgen
	./waf configure 
	./waf
	
    cd ../ns-3
	./waf configure --enable-examples
	./waf
	
    cd ../
}

function minindn {
    if [[ updated != true ]]; then
        if [ ! -d "build" ]; then
            $update
            updated="true"
        fi
    fi

    if [[ $pysetup != true ]]; then
        $install python-setuptools
        pysetup="true"
    fi
    install_dir="/usr/local/etc/mini-ndn/"
    
    git clone https://github.com/named-data/mini-ndn
	cd mini-ndn
	rm -f install.sh
    sudo mkdir -p "$install_dir"
    sudo cp ndn_utils/client.conf.sample "$install_dir"
    sudo cp ndn_utils/nlsr.conf "$install_dir"
    sudo cp ndn_utils/topologies/default-topology.conf "$install_dir"
    sudo cp ndn_utils/topologies/minindn.testbed.conf "$install_dir"
    sudo python setup.py install
    
    cd ../
}


function usage {
    printf '\nUsage: %s [-mfrtids]\n\n' $(basename $0) >&2

    printf 'options:\n' >&2
    printf -- ' -f: install NFD\n' >&2
    printf -- ' -i: install mini-ndn\n' >&2
    printf -- ' -m: install mininet and dependencies\n' >&2
    printf -- ' -r: install NLSR\n' >&2
    printf -- ' -t: install tools\n' >&2
    printf -- ' -d: install ndndump\n' >&2
    printf -- ' -s: install ndnSIM and dependencies\n' >&2
    exit 2
}

if [[ $# -eq 0 ]]; then
    usage
else
    while getopts 'mfrtids' OPTION
    do
        case $OPTION in
        f)    forwarder;;
        i)    minindn;;
        m)    mininet;;
        r)    routing;;
        t)    tools;;
        d)    ndndump;;
        s)    ndnsim;;
        ?)    usage;;
        esac
    done
    shift $(($OPTIND - 1))
fi
