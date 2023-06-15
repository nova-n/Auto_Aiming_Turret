import cv2 as cv
import numpy as np

face1 = cv.imread("C:/Users/nathan/Pictures/face example.PNG")
cv.imshow("my face",face1)
facesMany = cv.imread("C:/Users/nathan/Pictures/1280px-The_class_the_stars_fell_on_(2).jpg")
capture1 = cv.VideoCapture("C:/Users/nathan/Pictures/Camera Roll/WIN_20210128_08_12_54_Pro_Trim.mp4")
capture2 = cv.VideoCapture(0) #if want to ue webcam, use integer argument

def rescaleFrame(frame, scale): #is a created function that rescales
    #works for images, video, live video
    #convert floating points to ints because pixel counts must be integers
    width = int(frame.shape[1] * scale) # frame.shape[1] means width
    height = int(frame.shape[0] * scale) # frame.shape[0] means height
    dimensions = (width,height) #tuple
    return cv.resize(frame, dimensions, interpolation = cv.INTER_AREA)

def changeResolution(width, height):#is a created function that changes resolution
    #only works for live video
    capture.set(3,width) #for capture class, 3 is width, 4 is height
    capture.set(4,height)

#Face detection uses the edges and contours to detect a face

#must convert to grayscale for face detection
grayFace1 = cv.cvtColor(face1,cv.COLOR_BGR2GRAY)
cv.imshow("gray my face",grayFace1)
grayManyFaces = cv.cvtColor(facesMany,cv.COLOR_BGR2GRAY)
cv.imshow("gray many faces",grayManyFaces)

#must create haarcascades variable
haar_cascade = cv.CascadeClassifier("C:/Users/nathan/Python Projects/Haarcascades/haarcascade_frontalface_default.xml") 
faces_rectangles = haar_cascade.detectMultiScale(grayManyFaces, scaleFactor = 1.1, minNeighbors = 3)
#gray face img, scale factor, neighbors
#the minNeighbors is to help detect false positives
#will detect face and return list of coordinates
print(f"Number of Faces Found: {len(faces_rectangles)}")

#will draw a rectangle over each face
for (x,y,w,h) in faces_rectangles:
    cv.rectangle(facesMany,(x,y),(x+w,y+h), (0,0,255),2)
cv.imshow("boxed in",facesMany)

#gonna try face recoginition in video, per frame.
while True: #use while loop to read video
    #capture.read() returns two outputs: a boolean that says if a frame is succesfully returned, and each frame
    isTrue , frame = capture1.read()
    grayFrame = cv.cvtColor(frame,cv.COLOR_BGR2GRAY)
    #cv.imshow( "Video", frame)

    faces_rectanglesVid = haar_cascade.detectMultiScale(grayFrame, scaleFactor = 1.1, minNeighbors = 3)
    for (x,y,w,h) in faces_rectanglesVid:
        #cv.rectangle(frame,(x,y),(x+w,y+h), (0,0,255),2)
        cv.circle(frame,(x+w//2,y+h//2),(w+h)//4,(0,0,255),2)
        cv.line(frame,(x+w//2,y-h//2),(x+w//2,y+3*h//2),(0,0,255),2)
        cv.line(frame,(x-w//2,y+h//2),(x+3*w//2,y+h//2),(0,0,255),2)

    #prevent form infinite loop
    #0xFF == ord("d") means key input "d"
    if cv.waitKey(20) & 0xFF == ord("d"):
        break
    #if using video,
    #will get a 215 error at end of video, since could not find frame after last one for video
    cv.imshow("video with target",frame)
    

capture1.release()
cv.destroyAllWindows()

cv.waitKey(0)