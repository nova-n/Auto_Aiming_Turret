import cv2 as cv
import numpy as np
import serial
import time
from picamera import PiCamera
import os
import multiprocessing
import threading
import queue
import concurrent.futures 

initStartTime = None
timeAtStartReadThread = None
latencyTime = None
readyToStart = False
resolutionVerification = None
beginProgramLine = None

webCamCapture = cv.VideoCapture(cv.CAP_V4L)
print(webCamCapture.isOpened())
if webCamCapture.isOpened():
    vidWidth = int(webCamCapture.get(cv.CAP_PROP_FRAME_WIDTH)) #will return a float without int()
    vidHeight = int(webCamCapture.get(cv.CAP_PROP_FRAME_HEIGHT)) #will return a float without int()
    print("camera width = " + str(vidWidth) + " , camera height = " + str(vidHeight) )
    res = "<X{fW},Y{fH}>".format(fW=vidWidth,fH=vidHeight)
haar_cascade_faces = cv.CascadeClassifier("/home/nova-n/git/Auto_Aiming_Turret/Haarcascades/haarcascade_frontalface_default.xml") 
haar_cascade_fullBody = cv.CascadeClassifier("/home/nova-n/git/Auto_Aiming_Turret/Haarcascades/haarcascade_fullbody.xml")

arduino = serial.Serial('/dev/ttyACM0',9600,timeout=0.1) #port,baudrate,timeout
print("arduino connected")
arduino.reset_input_buffer()
arduino.reset_output_buffer()

while arduino.in_waiting == 0:
	pass #just wait for input
print("past wait")
while True:
	beginProgramLine = arduino.readline() #read the entire line terminated by an end marker \0
	if beginProgramLine.decode('utf-8') != 'Buffer Cleared, Ready To Start!': #DO NOT INCLUDE THE END MARKER!!!
		if arduino.in_waiting == 0:
			print("Buffer Is Empty!");
			pass
		else:
			print("not equal")
			print(beginProgramLine.decode('utf-8'))
	else:
		break
print(beginProgramLine.decode('utf-8'))
print("Sending Resolution String!")
arduino.write(res.encode())
while readyToStart == False:
	if arduino.in_waiting > 0:
		resolutionVerification = arduino.readline()
		readyToStart = True
print(resolutionVerification.decode('utf-8'))

#will loop forever, and not allow the code to continue until the arduino sends confirmation

centerCoord = [vidWidth/2,vidHeight/2]
print("Center Coord: (" + str(centerCoord[0]) + "," + str(centerCoord[1]) +")")

#This class will crete objects that have methods to read and video
#The reading and showing are I/O bound operations, so they will be run in separate threads
#No need to lock threads, because the reader just gets from the buffer, and outputs a frame
#The show takes the frame from the reader
#Best to do these in objects, because while functions can return the frames of the webcam, when starting a thread,
#there is no way to get the output of a function
#the object can hold these frames instead, and these will be accessed

#The program takes a while to start up, but it is enough time for the camera to put frames in the buffer
#Seems like when starting up, there is JUST enough time for a frame to be ready from the object, which the main
#program can use

frameFinishedReading = threading.Event()
frameQueue = queue.Queue()
gotFrameTrueFalseQueue = queue.Queue()
#communication between threads, and also, a way for the main thread
#(the main program) to know if vidReader is finished reading, and has outputted a frame

class videoReader:
	def __init__(self,cam,lock): #initial state of an object's attributes
		#capture.read() returns two outputs: a boolean that says if a frame is succesfully returned, and each frame
		self.camera = cam
		(self.gotFrame , self.frame) = self.camera.read()
		self.stop = False
		self.timeAtBeginLoop = None
		self.timeAtEndLoop = None
	#can't use self.camera as a default value for the function
	def getAndReadFrame(self,camToRead = None): #the function that needs to be continuously called
		#the self.frame is just the initial value, and __init__ is not a 
		print("called getAndReadFrame")
		camToRead = self.camera
		global latencyTime #must declare global in the actual function that accesses it
		while self.stop == False:
			self.timeAtBeginLoop = time.time()
			if self.gotFrame == False or (cv.waitKey(1) & 0xFF == ord("d")):
				self.stop == True
				gotFrameTrueFalseQueue.put(False)
				break
				return
			else:
				lock.acquire()
				(self.gotFrame , self.frame) = camToRead.read()
				lock.release()
				#WAITING FOR EVENT CAUSES DELAY AGAIN
				#frameFinishedReading.set() #Event Set
				#print("event set")
				#frameFinishedReading.clear()
			self.timeAtEndLoop = time.time()
			latencyTime = self.timeAtEndLoop - self.timeAtBeginLoop
		return

#Because the video displayer takes form a global queue, its best to leave this as a function, instead of an object
def videoDisplayer(displayerKeepGoing = True):
	while displayerKeepGoing:
		displayerKeepGoing = gotFrameTrueFalseQueue.get()
		if displayerKeepGoing == False:
			break
			return
		elif frameQueue.qsize() > 0:
			cv.imshow("video with target",frameQueue.get())
		else:
			pass
	if displayerKeepGoing == False:
			return
	return
	
print("defined class videoReader")
lock = threading.Lock()		    
vidReader = videoReader(webCamCapture,lock)
vidReadThread = threading.Thread(target=vidReader.getAndReadFrame,args=(webCamCapture,))
vidReadThread.start()
timeAtStartReadThread = time.time()
#latencyTime = timeAtStartReadThread - initStartTime
del initStartTime,timeAtStartReadThread #will never be used after, delete to clear up memory
print(f"made reader thread at: {latencyTime} secs")

# print("defined class videoDisplayer")		    
# vidDisplayer = videoDisplayer()
# vidShowThread = threading.Thread(target=vidDisplayer.showVideo)
# vidShowThread.start()
# print("made displayer thread")

vidShowThread = threading.Thread(target=videoDisplayer)
vidShowThread.start()
print("made displayer thread")

while True: #use while loop to read video
	#print("In main Loop")
	#frameFinishedReading.wait()
	lock.acquire()	
	frame = vidReader.frame
	isTrue = vidReader.gotFrame
	lock.release()
	
	#print(f"latency time: {latencyTime} secs") 
	
	grayFrame = cv.cvtColor(frame,cv.COLOR_BGR2GRAY)
	bilatGrayFrame = cv.bilateralFilter(grayFrame,7,35,25)
    #cv.imshow( "Video", frame)

	faces_rectanglesVid = haar_cascade_faces.detectMultiScale(bilatGrayFrame, scaleFactor = 1.4, minNeighbors = 2)
	fullyBodies_rectanglesVid = haar_cascade_fullBody.detectMultiScale(bilatGrayFrame, scaleFactor = 1.1, minNeighbors = 1)

    #allTargetsArray = faces_rectanglesVid + fullyBodies_rectanglesVid
	for (x,y,w,h) in faces_rectanglesVid:
		#cv.rectangle(frame,(x,y),(x+w,y+h), (0,0,255),2)
		cv.circle(frame,(x+w//2,y+h//2),(w+h)//4,(0,0,255),2)
		cv.line(frame,(x+w//2,y-h//2),(x+w//2,y+3*h//2),(0,0,255),2)
		cv.line(frame,(x-w//2,y+h//2),(x+3*w//2,y+h//2),(0,0,255),2)
        #sending coords to arduino
        #Note, I want the center of the screen to be (0,0), but it defines the top left as 0,0, so I must modify the code
		coords = "<X{fX},Y{fY}>".format(fX = x+w//2 - centerCoord[0] , fY = y+h//2 - centerCoord[1])
		print(coords)
		arduino.write(coords.encode())
        #print(coords.encode)
         
	frameQueue.put(frame) #safe way to send data between threads
	gotFrameTrueFalseQueue.put(isTrue)
	if cv.waitKey(1) & 0xFF == ord("d"):
		print("should exit")
		print(vidReadThread.is_alive())
		print(vidShowThread.is_alive())
		break
    #if using video,
    #will get a 215 error at end of video, since could not find frame after last one for video
    #next, should make it return the largest target first. just for ease of beginning

webCamCapture.release()
cv.destroyAllWindows()

cv.waitKey(0)
