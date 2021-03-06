#include <sstream>
#include <string>
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <iostream>
//#include <opencv2\highgui.h>
#include "opencv2/highgui/highgui.hpp"
//#include <opencv2\cv.h>
#include "opencv2/opencv.hpp"
#include <unistd.h>
using namespace std;
using namespace cv;
//initial min and max HSV filter values.
//these will be changed using trackbars

int xmeu,ymeu,xadv,yadv,xmeu1,ymeu1;



int H_MIN2 = 0;
int H_MAX2 = 256;
int S_MIN2 = 148;
int S_MAX2 = 256;
int V_MIN2 = 0;
int V_MAX2 = 256; 

int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 148;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

int H_MIN1=169;
int H_MAX1=256;
int S_MIN1=0;
int S_MAX1=256;
int V_MIN1=0;
int V_MAX1=256;



//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const std::string windowName = "Original Image";
const std::string windowName1 = "HSV Image";
const std::string windowName2 = "Thresholded Image";
const std::string windowName3 = "After Morphological Operations";
const std::string trackbarWindowName = "Trackbars";


void on_mouse(int e, int x, int y, int d, void *ptr)
{
	if (e == EVENT_LBUTTONDOWN)
	{
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}
}

void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed
}

string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}

void createTrackbars() {
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH),
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);


}
void drawObject(int x, int y, Mat &frame) {

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//UPDATE:JUNE 18TH, 2013
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25 > 0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25 < FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25 > 0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25 < FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);
	//cout << "x,y: " << x << ", " << y;

}
void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed) {

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area > MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				//cout << x << "," << y;
				drawObject(x, y, cameraFeed);

			}


		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}
char message[2];
void trimite(char s[]){
   
   struct sockaddr_in server;
   int sock;
   sock = socket(AF_INET , SOCK_STREAM , 0);
   server.sin_addr.s_addr = inet_addr("193.226.12.217");
   server.sin_family = AF_INET;
   server.sin_port = htons( 20232 );
   
   
   if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
   {
        cout<<"connect failed. Error";
   }

    for(int i=0;i<strlen(s);i++)
    cout<<s[i]<<" ";
    cout<<endl;
    for(int i=0;i<strlen(s);i++)
    {
      if(s[i]=='f'||s[i]=='s'||s[i]=='l'||s[i]=='r'||s[i]=='b')
      {cout<<"aici "<<s[i]<<endl;
        
        sprintf(message,"%c",s[i]);
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            cout<<"Send failed";
        }
        sleep(1);
      }
   }
   sprintf(message,"s");
   send(sock,message,strlen(message),0);
   
}



int main(int argc, char* argv[])
{

	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = true;

	Point p;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
  Mat threshold1;
	//x and y values for the location of the object
	int x = 0, y = 0;
	//create slider bars for HSV filtering
 
	createTrackbars();
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	capture.open("rtmp://172.16.254.99/live/nimic");
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop

     int counter=0;
     int counter1=0;
      //trimite("f");

		while (1) {
    
    		//store image to matrix
    		capture.read(cameraFeed);
    		//convert frame from BGR to HSV colorspace
    		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
    		//filter HSV image between values and store filtered image to
    		//threshold matrix
    		if(counter==0){
    			inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
          inRange(HSV, Scalar(H_MIN2,S_MIN2,V_MIN2), Scalar(H_MAX2,S_MAX2,V_MAX),threshold1);
    			counter=1;
    		}
    		else {
    
    			inRange(HSV,Scalar(H_MIN1,S_MIN1,V_MIN1),Scalar(H_MAX1,S_MAX1,V_MAX1),threshold);
    			counter=0;	
    		}
    		//perform morphological operations on thresholded image to eliminate noise
    		//and emphasize the filtered object(s)
    		if (useMorphOps)
    			morphOps(threshold);
    		//pass in thresholded frame to our object tracking function
    		//this function will return the x and y coordinates of the
    		//filtered object
    		if (trackObjects){
    			trackFilteredObject(x, y, threshold, cameraFeed);//aici detectez centrul robotului
          if(counter==1){
           trackFilteredObject(xmeu1, ymeu1, threshold1, cameraFeed);//aici am coordonatele de la fata
          }  
       }
       if(counter==1){
         xmeu=x;
         ymeu=y;
         
         
         int distantax=xmeu-xadv;
         int distantay=ymeu-yadv;
         
         if(xmeu1-xmeu0>=0&&ymeu1-ymeu>=0){
           cadranulmeu=1;
           //cadranul 1
           if(distantax>=0&&distantay<0){
           
             //turn left multiple times(pana ajungi in cadranul 2
           }
           else if(distantax>=0&&distantay>=0){
             
             //turn right multiple times(pana ajungi in cadranul 3
           
           }
           else if(distantax<0&&distantay>=0){
           
             //turn right
           
           }
           else if(distantax<0&&distantay<0){
           
             //turn forward
           
           }
         
         }
         else if(xmeu1-xmeu>=0&&ymeu1-ymeu<0){
           cadranulmeu=4;
         //cadranul 4
         
          if(distantax>=0&&distantay<0){
           
             //turn left multiple times(pana ajungi in cadranul 2
           }
           else if(distantax>=0&&distantay>=0){
             
             //turn left (pana ajungi in cadranul 3
           
           }
           else if(distantax<0&&distantay>=0){
           
             //go forward
           
           }
           else if(distantax<0&&distantay<0){
           
             //turn right
           
           }
         
         }
         else if(xmeu1-xmeu<0&&ymeu1-ymeu<0){
           cadranulmeu=3;
         //cadranul 3
         
            if(distantax>=0&&distantay<0){
           
             //turn right(pana ajungi in cadranul 2
           }
           else if(distantax>=0&&distantay>=0){
             
             //go forward
           
           }
           else if(distantax<0&&distantay>=0){
           
             //turn left
           
           }
           else if(distantax<0&&distantay<0){
           
             //turn left multiple time
           
           }
         
         
         }
         else if(xmeu1-xmeu<0&&ymeu1-ymeu>=0){
          cadranulmeu=2;
         //cadranul 2
         
            if(distantax>=0&&distantay<0){
           
             //go forward
           }
           else if(distantax>=0&&distantay>=0){
             
             //turn left
           
           }
           else if(distantax<0&&distantay>=0){
           
             //turn right multiple time
           
           }
           else if(distantax<0&&distantay<0){
           
             //turn right
           
           }
         
         }
       }
       else if(counter==0){
         xadv=x;
         yadv=y;
         }
       }
    		//show frames
    		imshow(windowName2, threshold);
    		imshow(windowName, cameraFeed);
    		imshow(windowName1, HSV);
    		setMouseCallback("Original Image", on_mouse, &p);
    		//delay 30ms so that screen can refresh.
    		
    		//image will not appear without this waitKey() command
    		waitKey(30);
    	}

	return 0;
}
