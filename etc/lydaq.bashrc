if [ -f /usr/local/zdaq/etc/zdaq.bashrc ]; then
    .  /usr/local/zdaq/etc/zdaq.bashrc
fi
export LD_LIBRARY_PATH=/usr/local/lydaq/lib:$LD_LIBRARY_PATH
export PATH=/usr/local/lydaq/bin:$PATH
export PYTHONPATH=/usr/local/lydaq/share:$PYTHONPATH