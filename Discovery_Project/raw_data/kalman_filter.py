# -*- coding: utf-8 -*-
import numpy as np
import matplotlib.pyplot as plt
import random as rd
import math
import sys
#from tempData import voltTemp
#from tempData import accelX
from tempData import accelY
# from tempData import accelZ

#initialTemp = 1000.0
initialX = 0.0
initialY = 0.0
initialZ = 1000.0

class KalmanFilter(object):
	q = np.float32(0.0)
	r = np.float32(0.0)
	x = np.float32(0.0)
	p = np.float32(0.0)
	k = np.float32(0.0)

	def __init__(self, q, r, p=np.float32(0.0), k=np.float32(0.0), initial_value=np.float32(initialY)):
		self.q = q
		self.r = r
		self.p = p
		self.x = initial_value

	def update(self, measurement):
		self.p = np.float32(self.p +self.q)
		self.k = np.float32(self.p / (self.p + self.r))
		self.x = np.float32(self.x + self.k * (measurement - self.x))
		self.p = np.float32((1-self.k)* self.p)

		#return "p: " +str(self.p) + ", k: " +str(self.k) + ", output x: "+ str(self.x)
		return self.x

#for voltTemp
# qFilt = 1.0
# rFilt = 1.0
tempVect = accelY
#for accelX
qFilt = 1
rFilt = 1
#for accelY
# q = 1.0
# r = 1.0
#for accelZ
# q = 1.0
# r = 1.0
kalman1 = KalmanFilter(q=qFilt, r=rFilt);

#tempArray = []
length = len(tempVect)
emptyArray = [0.0]*(length-1)
diffArray = [0.0]*(length-1)

inputArray = np.array(tempVect, dtype='float32')
outputArray = np.array(emptyArray, dtype='float32')


#Get output array
for i in range(0,length-1):	
	outputArray[i]=kalman1.update(inputArray[i])
	diffArray[i]= outputArray[i] - inputArray[i]
#print "outputArray: " , outputArray

inputPlot, = plt.plot(inputArray, label="Input");
outputPlot, = plt.plot(outputArray, label="Filtered Output");
plt.ylabel('Angle')
plt.xlabel('Samples')
plt.title("Accelerometer Data y-axis")
plt.setp(inputPlot, color='g', linewidth=2.5)
plt.setp(outputPlot, color='r', linewidth=1.5)
plt.text(10, 200, r'r='+str(rFilt) +', q='+str(qFilt)+' stanDev= '+str(np.std(diffArray)))
plt.grid(True)
plt.legend( (inputPlot, outputPlot),
           ('Raw Input Angle', 'Filtered Angle'),
           'lower right' )
plt.show()
#sys.stdout = open('/Users/Xavier/Documents/workspace/laserfiche/tempData.py', 'w')
#print inputArray
#print tempArray