import cv2 as cv

webCamCapture = cv.VideoCapture(0) #if want to ue webcam, use integer argument
haar_cascade = cv.CascadeClassifier("C:/Users/nathan/Python Projects/Haarcascades/haarcascade_frontalface_default.xml") 

while True: #use while loop to read video
    #capture.read() returns two outputs: a boolean that says if a frame is succesfully returned, and each frame
    isTrue , frame = webCamCapture.read()
    grayFrame = cv.cvtColor(frame,cv.COLOR_BGR2GRAY)
    bilatGrayFrame = cv.bilateralFilter(grayFrame,7,35,25)
    #cv.imshow( "Video", frame)

    faces_rectanglesVid = haar_cascade.detectMultiScale(bilatGrayFrame, scaleFactor = 1.1, minNeighbors = 1)
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

webCamCapture.release()

cv.waitKey(0)