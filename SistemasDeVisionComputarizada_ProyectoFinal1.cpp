/**
  @file videocapture_basic.cpp
  @brief Computer Vision Systems
  @author JĹ.Enríquez & AJ. González
  @date Nov 03, 2018
*/
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/core/utility.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace cv;
using namespace std;

int const erosion_size=2;
int const dilation_size=5;
int const erosion_size2=3;

Mat frame,frame1, mask,src,src_gray,src_hsv,dst,cdst,segb,segb_dst,segb_di,segb_f,segbF,segbRGB,holi;

void help(char** av) {
    cout << "The program captures frames from a video file, image sequence (01.jpg, 02.jpg ... 10.jpg) or camera connected to your computer." << endl
         << "Usage:\n" << av[0] << " <video file, image sequence or device number>" << endl
         << "q,Q,esc -- quit" << endl
         << "space   -- save frame" << endl << endl
         << "\tTo capture from a camera pass the device number. To find the device number, try ls /dev/video*" << endl
         << "\texample: " << av[0] << " 0" << endl
         << "\tYou may also pass a video file instead of a device number" << endl
         << "\texample: " << av[0] << " video.avi" << endl
         << "\tYou can also pass the path to an image sequence and OpenCV will treat the sequence just like a video." << endl
         << "\texample: " << av[0] << " right%%02d.jpg" << endl;
}

int process(VideoCapture& capture) 
{
  cout << "press space to save a picture. q or esc to quit" << endl;
      capture >> frame;
  Size size(frame.cols, frame.rows);
    resize(mask, mask, size);
  
    /// Apply the erosion operation
      Mat element = getStructuringElement( MORPH_RECT,
                       Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                       Point( erosion_size, erosion_size ) );

    /// Apply the dilation operation
        Mat element2 = getStructuringElement( MORPH_RECT,
                         Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                         Point( dilation_size, dilation_size ) );

    /// Apply the erosion operation
        Mat element3 = getStructuringElement( MORPH_RECT,
                         Size( 2*erosion_size2 + 1, 2*erosion_size2+1 ),
                         Point( erosion_size2, erosion_size2 ) );

  for (;;) 
  {
    capture >> frame;
    if (frame.empty())break;
    
    GaussianBlur( frame, frame, Size(5,5), 3, 3, BORDER_DEFAULT );
    frame.copyTo(frame1, mask);
    //imshow("SOURCE", frame1);
    src = frame1;
    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    cvtColor( src, src_hsv, COLOR_BGR2HSV );

    segb = Mat::zeros(src_gray.size(), src_gray.type());

    for( int y = 560; y < src.rows; y++ )
    {
      for( int x = 0; x < src.cols; x++ )
      {
        Vec3b intensity = src_hsv.at<Vec3b>(y, x);
        uchar h = intensity.val[0];
        uchar s = intensity.val[1];
        uchar v = intensity.val[2];
        if (h>=20 && h <=40 && s>70 && v>100)
        {
          frame.at<Vec3b>(y, x)[0]= 0;
          frame.at<Vec3b>(y, x)[1]= 255;
          frame.at<Vec3b>(y, x)[2]= 255;
          segb.at<uchar>(y, x) = 1;
        }
        else if ( v>=150)
        {
          frame.at<Vec3b>(y, x)[0]= 255;
          frame.at<Vec3b>(y, x)[1]= 255;
          frame.at<Vec3b>(y, x)[2]= 255;
          segb.at<uchar>(y, x) = 1;
        }
      }
    }



      erode( segb, segb_dst, element );


      dilate( segb_dst, segb_di, element2 );


      erode( segb_di, segb_f, element );



    /// Stitching

      cvtColor( segb_f, segbRGB, COLOR_GRAY2BGR );
      cv::multiply(src, (segbRGB), segbF);
      //segbRGB.copyTo(segbF, src);

    ///HoughLines

        Canny(segbF, holi, 100, 200, 3);
        vector<Vec4i> lines;
        HoughLinesP(holi, lines, 1, CV_PI/180, 50, 50, 10 );
        for( size_t i = 0; i < lines.size(); i++ )
        {
          Vec4i l = lines[i];
          line( frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255,0,0), 3, CV_AA);
        }

        imshow("dst",frame);

      if (waitKey(5) >= 0)
          break;
  }
  return 0;
}

int main(int ac, char** av) 
{
  mask = imread(/*"mask.png"*/"mask1.png");
  cv::CommandLineParser parser(ac, av, "{help h||}{@input||}");
  if (parser.has("help"))
  {
      help(av);
      return 0;
  }
  std::string arg = parser.get<std::string>("@input");
  if (arg.empty()) 
  {
      help(av);
      return 1;
  }
  VideoCapture capture(arg); //try to open string, this will attempt to open it as a video file or image sequence
  if (!capture.isOpened()) //if this fails, try to open as a video camera, through the use of an integer param
      capture.open(atoi(arg.c_str()));
  if (!capture.isOpened()) 
  {
      cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
      help(av);
      return 1;
  }
  process(capture);
}