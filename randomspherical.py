import random
import math


numgen = int(input('number of points to generate on unit sphere:\n'))
while numgen > 0:
    theta = random.random()*2*math.pi
    phi = random.random()*2*math.pi
    print "(" + str(math.cos(theta) * math.sin(phi)) + ", " + str(math.sin(theta) * math.sin(phi)) + ", " + str(math.cos(phi)) + ")"
    
    numgen = numgen-1
