#!/bin/bash
apt update
apt install -y libftdi-dev snmp snmpd
cd /opt/
git clone http://github.com/mirabitl/lydaq
export ZDAQROOT=/opt/zdaq
export LYDAQROOT=/opt/lydaq

# CAEN driver
cd /tmp; tar zxv /opt/lydaq/drivers/utils/packages/CAENHVWrapper-6.0-build20190117.tgz 
cd /tmp; tar zxvf /opt/lydaq/drivers/utils/packages/CAENHVWrapper-6.0-build20190117.tgz 
cd CAENHVWrapper-6.0/
./install.sh
cd  $LYDAQROOT
rm -rf /tmp/CAENHVWrapper-6.0/

# drivers
cd  $LYDAQROOT/drivers/
scons install -j4

# WIENER SNMP
cd Wiener
mkdir -p /usr/share/snmp/mibs/
cp WIENER-CRATE-MIB.txt /usr/share/snmp/mibs/

# applications
cd  $LYDAQROOT/apps/
scons install -f SConstruct.docker  -j4

# monitoring plugins
cd  $LYDAQROOT/monitoring/
scons install -j4

mkdir -p /usr/local/share/lydaq/
find /opt/lydaq -name '*.py' -exec ln -sf {} /usr/local/share/lydaq/ \;
ln -sf /opt/lydaq/apps/mongoroc/mgroc /usr/local/bin/


find /opt/lydaq/drivers/ -name 'lib*.so' -exec ln -sf {} /usr/local/lib/ \;
find /opt/lydaq/apps/ -name 'lib*.so' -exec ln -sf {} /usr/local/lib/ \;
find /opt/lydaq/monitoring/ -name 'lib*.so' -exec ln -sf {} /usr/local/lib/ \;

find /opt/lydaq/apps/bin/ -wholename '*/bin/*' -type f -executable  -exec ln -sf {} /usr/local/bin \;


find /opt/lydaq -name '*.os' -exec rm {} \;
find /opt/lydaq -name '*.o' -exec rm {} \;

