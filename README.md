# Steganography
This is a school project: a C++ program that can hide an image inside another image (steganography), using OpenCV.

This code implements the technique described in the following article: "".

The encoding part is run first, showing the two original images (data/lena.png and data/opencv.jpg), and then the encoded image. The latter is saved under encoded_image.png in the current folder. 

It also saves a structure containing the dimensions of the original image to encode and the number of Least Significative Bits used. This data is saved in encoding_info.txt.

It then waits for the user to press a key (waitKey(0)), and starts decoding from the saved image.

Original image :

![](data/lena.png)

Image to encode :
![](data/opencv.jpg)

Encoded image : 
![](encoded_image.png)

## Instructions

Navigate to the Exercice2/ folder.

To build the program, use the commands: "cmake ." and then "make" .

To run the program, execute Steganographie . 

(OpenCV must be installed.)
