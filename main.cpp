#include <opencv2/opencv.hpp>
#include <iostream>
#include <unistd.h>
// https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html
using namespace std;

int main()
{

    cv::VideoCapture cap;
    int deviceID = 0;        // 0 = open default camera
    int apiID = cv::CAP_ANY; // 0 = autodetect default API

    cv::Mat firstFrame;
    cv::Mat frame;
    cv::Mat grayFrame;
    cv::Mat imgBlur;
    cv::Mat frameDelta;
    cv::Mat thresh;
    int x, y, w, h;

    vector<vector<cv::Point>> contours;
    int MIN_AREA = 500;

    cv::Mat currentFrame;
    // open selected camera using selected API
    cap.open(deviceID, apiID);
    // check if we succeeded
    if (!cap.isOpened())
    {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    //--- GRAB AND WRITE LOOP
    cout << "Start grabbing" << endl
         << "Press any key to terminate" << endl;
    while (true)
    {
        cap.read(frame);
        // if the first stream is not initialized, store it for reference	cap.read(firstFrame);
        if (frame.empty())
        {
            cout << "Could not open or find the image!\n"
                 << endl;
            return 1;
        }
        // color has no bearing on motion detection algorithm
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // to smooth the image and remove noise(if not then could throw algorithm off)
        // smothing avarage pixel intensities across an 21x21 region
        cv::GaussianBlur(grayFrame, imgBlur, cv::Size(21, 21), 0, 0);

        // if the first stream is not initialized, store it for reference
        if (firstFrame.empty())
        {
            firstFrame = grayFrame.clone(); // Store the first frame in grayscale
            continue;
        }
        // compute the absolute difference between the current frame and firstFrame
        cv::absdiff(firstFrame, imgBlur, frameDelta);
        cv::threshold(frameDelta, thresh, 10.0, 255.0, cv::THRESH_BINARY);

        // // dilate the threshold image to fill in holes, then find contours on the thresholded image
        // // apply background substraction
        cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);

        cv::findContours(thresh.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        vector<cv::Rect> boundRect(contours.size());
        if (contours.empty())
            continue;

        for (size_t i = 0; i < contours.size(); i++)
        {
            if (cv::contourArea(contours[i]) < MIN_AREA)
                continue;
            // get bounding box from countour
            boundRect[i] = cv::boundingRect(contours[i]);
            // draw bounding box
            cv::rectangle(frame, boundRect[i], cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("firstFrame", frame);

        if (cv::waitKey(1) >= 0)
        {
            break;
        }
    }
    cap.release();
    return 0;
}