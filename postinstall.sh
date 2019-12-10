#!/bin/bash
cd /opt/lydaq
mkdir -p /opt/dhcal/script
find . -name '*.py' -exec ln -sf /opt/lydaq/{} /opt/dhcal/script/ \;
echo "export PYTHONPATH=/opt/dhcal/script/:${PYTHONPATH}" >> ~/.bashrc
echo "export PYTHONSTARTUP=/opt/lydaq/etc/.pythonrc" >> ~/.bashrc
touch ${HOME}/.python_history
