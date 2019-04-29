import TdcAccess as oa
s=oa.TdcAccess("FEB_1112_11")

cor12_1= [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, 0, 0, 0, 0, 0, 0, 0, 0, 0]


#s.Correct6BDac(11*256+10,1,cor11_1)
#s.Correct6BDac(11*256+10,2,cor11_2)
s.Correct6BDac(12*256+10,1,cor12_1)
#s.Correct6BDac(12*256+10,2,cor12_2)


s.uploadChanges()
