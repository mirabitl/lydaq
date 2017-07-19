#!/bin/bash
rsync -axv /opt/zdaq/lib/ $1:/opt/dhcal/lib/
rsync -axv /opt/zdaq/bin/ $1:/opt/dhcal/bin/
rsync -axv /opt/zdaq/etc/ $1:/opt/dhcal/etc
rsync -axv /opt/lydaq/drivers/lib/ $1:/opt/dhcal/lib/
rsync -axv /opt/lydaq/apps/lib/ $1:/opt/dhcal/lib/
rsync -axv /opt/lydaq/analysis/lib/ $1:/opt/dhcal/lib/
rsync -axv /opt/lydaq/analysis/bin/ $1:/opt/dhcal/bin/
rsync -axv /opt/lydaq/apps/bin/ $1:/opt/dhcal/bin
rsync -axv /usr/local/lib/libzmq.so* $1:/opt/dhcal/lib/
