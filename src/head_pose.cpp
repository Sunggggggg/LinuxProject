#include <vector>
#include <iostream>
#include <fstream>

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"

// ***
#include <stdio.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/photo.hpp>

#include "ldmarkmodel.h"

using namespace std;
using namespace cv;


int main(int argc, char** argv) // ***
{
    ldmarkmodel modelt;
    std::string modelFilePath = "roboman-landmark-model.bin";

    while (!load_ldmarkmodel(modelFilePath, modelt)) {
        std::cout << "�ļ��򿪴��������������ļ�·��." << std::endl;
        std::cin >> modelFilePath;
    }

    cv::VideoCapture mCamera(0);

    mCamera.open("/edv/video0", CAP_V4L2); // ***
    if (!mCamera.isOpened()) {
        printf("Can`t open Camera\n");
        return -1;
    }

    cv::Mat Image;
    cv::Mat current_shape;

    for (;;) {
        mCamera.read(Image);
        if (Image.empty()) break;

        modelt.track(Image, current_shape);
        cv::Vec3d eav;
        modelt.EstimateHeadPose(current_shape, eav); // 이 함수를 실행하면, 변수 eav가 eav[0]에 Pitch, eav[1]에 Yaw, eav[2]에 Roll 값을 전달받음

        // *** 여기서 반환받은 eav[1] 값을 이용해서 고개가 어느쪽으로 돌아갔는지 판단하면 됨.
        printf("\n %f", eav[1]);

        // imshow 화면에 pose 정보 출력하는 함수임
        // modelt.drawPose(Image, current_shape, 50);

        // imshow 화면에 landmark point 표시하는 부분임
        //int numLandmarks = current_shape.cols / 2;
        //for (int j = 0; j < numLandmarks; j++) {
        //    int x = current_shape.at<float>(j);
        //    int y = current_shape.at<float>(j + numLandmarks);
        //    std::stringstream ss;
        //    ss << j;
        //    //            cv::putText(Image, ss.str(), cv::Point(x, y), 0.5, 0.5, cv::Scalar(0, 0, 255));
        //    cv::circle(Image, cv::Point(x, y), 2, cv::Scalar(0, 0, 255), -1);
        //}
        // cv::imshow("Camera", Image);

        if (27 == cv::waitKey(5)) {
            mCamera.release();
            // cv::destroyAllWindows();
            break;
        }
    }
    return 0;
}