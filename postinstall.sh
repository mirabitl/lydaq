#!/bin/bash
cd /opt/lydaq
mkdir -p /opt/dhcal/script
find . -name '*.py' -exec ln -sf /opt/lydaq/{} /opt/dhcal/script/ \;
cd /opt/zdaq
find . -name '*.py' -exec ln -sf /opt/zdaq/{} /opt/dhcal/script/ \;
echo "export PYTHONPATH=/opt/dhcal/script/:${PYTHONPATH}" >> ~/.bashrc
echo "export PYTHONSTARTUP=/opt/lydaq/etc/.pythonrc" >> ~/.bashrc
touch ${HOME}/.python_history
cd /usr/local/bin
sudo ln -sf /opt/zdaq/scripts/mgjob .
sudo ln -sf /opt/lydaq/apps/mongoroc/mgroc .
sudo ln -sf /opt/lydaq/monitoring/scripts/mgslow .
sudo ln -sf /opt/lydaq/monitoring/scripts/slaccess .
sudo ln -sf /opt/lydaq/pycontrol/febdaq .
sudo ln -sf /opt/lydaq/pycontrol/difdaq .
sudo ln -sf /opt/lydaq/pycontrol/combdaq .
