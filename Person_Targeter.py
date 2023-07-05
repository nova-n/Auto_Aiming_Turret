import cv2 as cv
import numpy as np
import serial
import time
from picamera import PiCamera
import os
import multiprocessing
import threading
import concurrent.futures

initStartTime = None
timeAtStartReadThread = None
timeAtBeginLoop = None
timeAtEndReading = None
timeAtEndLoop = None
timeBufferSpacing = None
latencyTime = None
#seems like can't get actual total latency time...

arduino = serial.Serial('/dev/ttyACM0',9600,timeout=0.1) #port,baudrate,timeout
#startMain = False
startMain = True
time.sleep(2) #delay of 2s
print("arduino connected")
initStartTime = time.time()
webCamCapture = cv.VideoCapture(cv.CAP_V4L)
fps = webCamCapture.get(cv.CAP_PROP_FPS)
timeBufferSpacing = 1/fps
print(f"fps: {fps}")
print(f"time between frames in secs: {timeBufferSpacing}")
#webCamCapture.set(cv.CAP_PROP_BUFFERSIZE,1) # how many frames allowed to sit in the buffer at a time. Lesser for lower latency
#webCamCapture = cv.VideoCapture(0,cv.CAP_MSMF) #if want to ue webcam, use integer argument
print(webCamCapture.isOpened())
haar_cascade_faces = cv.CascadeClassifier("/home/nova-n/git/Auto_Aiming_Turret/Haarcascades/haarcascade_frontalface_default.xml") 
haar_cascade_fullBody = cv.CascadeClassifier("/home/nova-n/git/Auto_Aiming_Turret/Haarcascades/haarcascade_fullbody.xml")

if webCamCapture.isOpened():
    vidWidth = int(webCamCapture.get(3)) #will return a float without int()
    vidHeight = int(webCamCapture.get(4)) #will return a float without int()
    print("camera width = " + str(vidWidth) + " , camera height = " + str(vidHeight) )
    res = "Resolution: <X{fW}Y{fH}>".format(fW=vidWidth,fH=vidHeight)

arduino.write(res.encode())

centerCoord = [vidWidth/2,vidHeight/2]
print("Center Coord: (" + str(centerCoord[0]) + "," + str(centerCoord[1]) +")")

byteLine = arduino.read() #read a byte
startString = byteLine.decode() #turn byte into string

#if startString == "F": #read a byte
 #   print("F")
  #  arduino.write(res.encode()) 
   # startMain = True

timeAtStartReadThread = time.time()
latencyTime = timeAtStartReadThread - initStartTime
del initStartTime, timeAtStartReadThread
print(f"latencyTime: {latencyTime} in secs")

while True & startMain == True: #use while loop to read 
    #fps = webCamCapture.get(cv.CAP_PROP_FPS)
    #timeBufferSpacing = 1/fps

    timeAtBeginLoop = time.time()
    #print(f"TimeAtBeginLoop: {timeAtEndLoop} in secs")
    #capture.read() returns two outputs: a boolean that says if a frame is succesfully returned, and each frame
    isTrue , frame = webCamCapture.read()
    
    timeAtEndReading = time.time()
    timeBufferSpacing = timeAtEndReading - timeAtBeginLoop
    print(f"read time: {timeBufferSpacing} in secs")
    # if isTrue == False:
        # print("Frame Not Recieved")
    # else:
        # print("Frame Recieved")
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
        #print(coords.encode())
    cv.imshow("video with target",frame)
    #prevent form infinite loop
    #0xFF == ord("d") means key input "d"
    
    timeAtEndLoop = time.time()
    #latencyTime += (timeAtEndLoop - timeAtBeginLoop) - timeBufferSpacing
    #print(f"loop time: { timeAtEndLoop - timeAtBeginLoop} in secs")
    latencyTime = timeAtEndLoop - timeAtBeginLoop
    print(f"latencyTime: {latencyTime} in secs")
    
    
    if cv.waitKey(1) & 0xFF == ord("d"):
        break
    #if using video,
    #will get a 215 error at end of video, since could not find frame after last one for video
    #next, should make it return the largest target first. just for ease of beginning

webCamCapture.release()
cv.destroyAllWindows()

cv.waitKey(0)
