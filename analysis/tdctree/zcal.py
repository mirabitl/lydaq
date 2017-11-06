from math import *
def calc(r,l,c,g,o):
    a=(r*r+o*o*l*l)*(g*g+o*o*c*c)
    #print a
    alpha=1./2*(sqrt(a)+r*g-l*c*o**2)
    print "alpha=%f" % alpha
    alpha=sqrt(alpha)
    beta=1./2*(sqrt((r**2+o**2*l**2)*(g**2+o**2*c**2))-r*g+l*c*o**2 )
    beta=sqrt(beta)
    print "beta=%f" % beta
    return [alpha,beta]
