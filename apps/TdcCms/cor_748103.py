import TdcAccess as oa
s=oa.TdcAccess("FEB_1112_10")

cor11_1= [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -8, 0]
cor12_1= [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -7, 0]


s.Correct6BDac(11*256+10,1,cor11_1)
#s.Correct6BDac(11*256+10,2,cor11_2)
s.Correct6BDac(12*256+10,1,cor12_1)
#s.Correct6BDac(12*256+10,2,cor12_2)


s.uploadChanges()
