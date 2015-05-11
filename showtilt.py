#!/usr/bin/env python



import sys, types, os
import serial
import math
from time import localtime
from datetime import timedelta,datetime
from math import sin, cos, pi, atan2
from threading import Thread

from Tkinter import * 


class showtilt:
	

	def __init__ (self,root):
		global ser	
		self.root = root		
 		print("showtilt start")

		self.canvas = Canvas(root, width =100, height = 100, background = "white")
		self.canvas.pack(fill=BOTH, expand=YES)			
		self.root.after(200,self.poll)
		
		ser = serial.Serial('/dev/ttyUSB0',115200 , timeout=1)
		self.ser_init()		
		#serial interface		
		#ser = serial.Serial('/dev/ttyUSB0',115200 , timeout=0)
		#>>> x = ser.read()          # read one byte
		#>>> s = ser.read(10)        # read up to ten bytes (timeout)
		#>>> line = ser.readline()   # read a '\n' terminated line


	
	
	def __del__ (self):
		print("showtilt end")
		ser.close()

	def ser_init(self):
		
		ser.write("Y30\n\r" ) 
		ser.write("T100000\n\r")
		ser.write("?\n\r")
		params= ser.read(200 )
		print( params)
		ser.write("s d0 6B 01 p\n\r") # Power management
		ser.write("S D0 75 P S D1 01 P\n\r") #who am i
		params= ser.readline()
		print( params)
		ser.write("s d0 1B 00 p\n\r") # +-250 degree /scale
		ser.write("s d0 1A 06 p\n\r") #bandwidth 10hz
		ser.write("s d0 1C 00 p\n\r") # +-2g 

		self.AccSensitivity = 2*9.81/ 0x8000
		self.GyroSensitivity =250 / 0x8000   #250 degree per second
		self.GyroAngle = 0.0


	def readGyro(self,direction ):
		
		if direction == 'x': 
			msbbyte = "43";
			lsbbyte = "44";
		
		if direction == 'y':
			msbbyte = "45";
			lsbbyte = "46";
		if direction == 'z': 
			msbbyte = "47";
			lsbbyte = "48";

		msbstring = "S D0 " + msbbyte + " P S D1 01 P\n\r"
  		lsbstring = "S D0 " + lsbbyte + " P S D1 01 P\n\r"
		ser.write(msbstring) #most significant byte 
		p1= ser.read(5 )
		print( p1)
		ser.write(lsbstring) #least significant byte 
		p2= ser.read(5 )
		print( p2)
		ip1=int('0x' +p1,16)
		ip2=int('0x' +p2,16)
		ip = ip2+256*ip1;
		if ip >= 0x8000 : 
			ip= -(0xffff- ip)
		return( ip * self.GyroSensitivity)



	def readAcc(self,direction ):
		
		if direction == 'x': 
			msbbyte = "3B";
			lsbbyte = "3C";
		
		if direction == 'y':
			msbbyte = "3D";
			lsbbyte = "3E";
		if direction == 'z': 
			msbbyte = "3F";
			lsbbyte = "40";

		msbstring = "S D0 " + msbbyte + " P S D1 01 P\n\r"
  		lsbstring = "S D0 " + lsbbyte + " P S D1 01 P\n\r"
		ser.write(msbstring) #most significant byte 
		p1= ser.read(5 )
		print( p1)
		ser.write(lsbstring) #least significant byte 
		p2= ser.read(5 )
		print( p2)
		ip1=int('0x' +p1,16)
		ip2=int('0x' +p2,16)
		ip = ip2+256*ip1;
		if ip >= 0x8000 : 
			ip= -(0xffff- ip)

		return( ip* self.AccSensitivity)


	def setmotor(self, angle ):
		
		iangle = int (2047.0/360.0 *angle) 

		lsbbyte = iangle & 255 
		msbbyte = (iangle & 0xff00 ) >> 8
		msbstring ="S A0 61 %02x %02x P\n\r" % (  msbbyte,lsbbyte)
  		#print("motor ",angle,"  ", iangle," ", msbstring)
		ser.write(msbstring) #most significant byte 



	def poll(self):
		self.canvas.delete(ALL) 
		#print("poll again\n")
		#ser.write("S D0 43 P S D1 01 P\n\r") #msb gyro X
 		#ser.write("S D0 3F P S D1 01 P\n\r") #msb acceleration 
		#p1= ser.read(5 )
		#print( p1)


		#ser.write("S D0 44 P S D1 01 P\n\r") #lsb 
 		#ser.write("S D0 40 P S D1 01 P\n\r") #msb acceleration 
		#p2= ser.read(5 )
		#print( p2)
		#ip1=int('0x' +p1,16)
		#ip2=int('0x' +p2,16)
		#ip = ip2+256*ip1;
		xAcc=self.readAcc('x')		
		zAcc=self.readAcc('z')
		print("xAcc",xAcc )
		print("zAcc",zAcc )

		#angle = ip/91 *pi/180;

		angle= math.atan2( xAcc,zAcc )
		if angle < 0:
			angle = 2*pi + angle;
		print("Angle ",angle*180.0/pi )

		xGyro=self.readAcc('x')
		print("GyroX", xGyro )
		
		self.dt =1
		self.GyroAngle += xGyro *pi /180 *self.dt

		x, y = cos(angle)*40,sin(angle)* 40   
		scl = self.canvas.create_line( 40 ,40, 40 + x, 40 + y , fill="blue", width=3)

		x, y = cos(self.GyroAngle)*40,sin(self.GyroAngle)* 40   
		scl = self.canvas.create_line( 40 ,40, 40 + x, 40 + y , fill="red", width=3)
		
		self.setmotor( angle*180/pi )
		 
		
		self.root.after(20,self.poll)
		

	
if __name__== '__main__':
	root = Tk()
   
    
	s = showtilt(root)		
	#s.poll()
	print("main")
	root.mainloop()




