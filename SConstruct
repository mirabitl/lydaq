import os
dlist=['drivers/SConstruct','apps/SConstruct']

Use_ROOT=os.path.isdir("/opt/dhcal/root")
Use_Oracle=os.path.isdir("/opt/ILCConfDB")
Use_LCIO=os.path.isdir("/opt/dhcal/lcio")
if (Use_ROOT and Use_LCIO):
  dlist.append("analysis/SConstruct")
SConscript(dlist)
