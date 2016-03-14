# Student Photo ID

Uses OpenCV 2 and Tesseract OCR to sort through a folder of photographs and automatically rename them using a name plate in the image. Print each person's nameplate onto red paper using OCR-B font.

Compiles on Ubuntu Linux, with Windows coming soon. 

## OpenCV Processing Flow
 
Each image has the red channel subtracted from the green channel which results in a distinct separation of the red name tag from the rest of the photo. Running the subtracted layers through a binary threshold typically results in a large contour which can be cropped and straightened for use in the OCR.
  
Straightening is performed using [OpenCV's Hough Lines transform](http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html) and has been tuned to give the best results with the name plate provided, although some characteristics can be adjusted using the advanced settings tab.

![Mask Preparation](images/mask-preparation-stages.png)

## Compiling

These instructions are here to be helpful, if they don't work then I'm afraid you are on your own. 

### On Ubuntu 14.04

This was tested with a fresh install of 64 bit Ubuntu 14.04 "Trusty Tahr".

 * Install OpenCV 2 by following [these instructions](http://computervisionwithvaibhav.blogspot.co.uk/2015/07/installing-opencv-300-on-ubuntu-1404_24.html). In step 3, **do not** include `-D WITH_QT=ON` when running cmake.
 * In a terminal window run `$ sudo apt-get install qtcreator` to install QT Creator and the QT5 development environment.
 * Then install Tesseract-OCR and the english language file: `$ sudo apt-get install tesseract-ocr tesseract-ocr-eng libtesseract-dev`

## To Do

* Compile for Windows
* This code is messy, and it needs tidying up.
* Move processing code to separate class, and out of the interface code.
* Create a new thread for the processing code.

## Screenshots

![Main Page](images/utility-main-page.png)

![Advanced Settings Page](images/utility-advanced-page.png)



## Icon

[Running Man](https://www.elegantthemes.com/blog/freebie-of-the-week/beautiful-flat-icons-for-free)