import TdcAccess as oa
s=oa.TdcAccess("FEB_1112_9")


chm=[0,1,2,3,25,27,29,31]

for c in chm:
  s.Change6BDac(11*256+10,1,c,0)
  s.ChangeMask(11*256+10,1,c,1)
  s.Change6BDac(11*256+10,2,c,0)
  s.ChangeMask(11*256+10,2,c,1)
  s.Change6BDac(12*256+10,1,c,0)
  s.ChangeMask(12*256+10,1,c,1)
  s.Change6BDac(12*256+10,2,c,0)
  s.ChangeMask(12*256+10,2,c,1)

c=30
s.Change6BDac(11*256+10,1,c,31)
s.ChangeMask(11*256+10,1,c,0)
s.Change6BDac(11*256+10,2,c,31)
s.ChangeMask(11*256+10,2,c,0)
s.Change6BDac(12*256+10,1,c,31)
s.ChangeMask(12*256+10,1,c,0)
s.Change6BDac(12*256+10,2,c,31)
s.ChangeMask(12*256+10,2,c,0)

  

#cor11_1= [0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, 0, -1, 0, 0, 0]
#cor11_2= [0, 0, 0, -3, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, 0, -1, 0, 0, 0]

#s.Correct6BDac(11*256+10,1,cor11_1)
#s.Correct6BDac(11*256+10,2,cor11_2)
#s.Correct6BDac(12*256+10,1,cor12_1)
#s.Correct6BDac(12*256+10,2,cor12_2)


s.uploadChanges()
