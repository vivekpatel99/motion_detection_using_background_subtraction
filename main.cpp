#include <opencv2/opencv.hpp>
#include <iostream>
#include <unistd.h>

using namespace std;

int main()
{
    int CAM_NUM = 0; // Replace with your camera number
    cv::VideoCapture cap(CAM_NUM);
    if (!cap.isOpened())
    {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cv::Mat firstFrame;
    cv::Mat frame;
    cv::Mat gray;
    cv::Mat frameDelta;
    cv::Mat thresh;
    int MIN_AREA = 500;

    usleep(2000000); // Sleep for 2 seconds

    while (true)
    {
        cap.read(frame);
        if (frame.empty())
        {
            cout << "ERROR: No frame available !!\n";
            break;
        }

        // Resize the image to have less processing time
        cv::resize(frame, frame, cv::Size(500, frame.rows * 500 / frame.cols));
        // Convert to grayscale
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        // Smooth the image to remove noise
        cv::GaussianBlur(gray, gray, cv::Size(21, 21), 0);

        // If the first frame is not initialized, store it for reference
        if (firstFrame.empty())
        {
            firstFrame = gray.clone();
            continue;
        }

        // Compute the absolute difference between the current frame and firstFrame
        cv::absdiff(firstFrame, gray, frameDelta);
        cv::threshold(frameDelta, thresh, 25, 255, cv::THRESH_BINARY);

        // Dilate the threshold image to fill in holes, then find contours on the thresholded image
        cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);

        vector<vector<cv::Point>> contours;
        cv::findContours(thresh.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Looping for contours
        for (size_t i = 0; i < contours.size(); i++)
        {
            if (cv::contourArea(contours[i]) < MIN_AREA)
                continue;

            cv::Moments M = cv::moments(contours[i]);
            int cX = int(M.m10 / M.m00);
            int cY = int(M.m01 / M.m00);

            // Get bounding box from contour
            cv::Rect boundingBox = cv::boundingRect(contours[i]);

            // Draw bounding box
            cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);
            cv::circle(frame, cv::Point(cX, cY), 7, cv::Scalar(0, 0, 255), -1);
        }

        cv::imshow("Original", frame);
        cv::imshow("Threshold", thresh);
        cv::imshow("FrameDelta", frameDelta);

        if (cv::waitKey(1) == 'q')
        {
            break;
        }
        firstFrame.empty();
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
