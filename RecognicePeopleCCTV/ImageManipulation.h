#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <vector>
#include "types.h"
//using namespace cv;

class ImageManipulation {
	public: 
		static void RotateImage(cv::Mat& image, double angle) {
            cv::Point2f pc(image.cols / 2., image.rows / 2.);
            cv::Mat r = cv::getRotationMatrix2D(pc, angle, 1.0);
            cv::warpAffine(image, image, r, image.size());
        }

        /// <summary>Draw a circle into the given image </summary>
        /// <param name='image'>Image to draw the circle</param>
        /// <param name='coords'>array of 3 doubles that have the coords in this position: 0=xA, 1=yA, 2=xB, 3=yB </param>
        static void DrawCircle(cv::Mat& image, double* coords) {
            cv::Point center ((coords[0] + coords[1]) / 2, (coords[2] + coords[3]) / 2);
            cv::circle(image, center, 2, cv::Scalar(255, 255, 255), 3);
        }

        /// <summary>Add pad to iamge</summary>
        /// <param name='bottom'>Pad to add below the image</param>
        /// <param name='right'>Pad to add in the right side of the image</param>
        static void AddPad(cv::Mat& image, ulong bottom = 0, ulong right = 0) {
            cv::copyMakeBorder(image, image, 0, bottom, 0, right, cv::BORDER_CONSTANT);
        }

        /// <summary>Stack images Horizontally</summary>
        /// <param name='images'>Array of images to stack</param>
        /// <param name='arraySize'>Amount of images to concat</param>
        /// <param name='height'>Height of each image in the array. Pass 0 to automatically calculate it.</param>
        static cv::Mat HStackWithPad(cv::Mat* images, ushort arraySize, ushort height = 0) {
            int maxHeight = 0;
            cv::Mat res;

            assert(arraySize > 0);

            if (height == 0) {
                // calculate the ideal height (bigger)
                for (ushort i = 0; i < arraySize; i++) {
                    if (images[i].rows > maxHeight)
                        maxHeight = images[i].rows;
                }
            } else maxHeight = height;
                        
            for (ushort i = 0; i < arraySize; i++) {
                assert(images[i].dims <= 2);
                assert(images[i].type() == images[0].type());
                if (images[i].rows != maxHeight)
                    AddPad(images[i], (maxHeight - images[i].rows));
            }

            try {
                cv::hconcat(images, arraySize, res);
            } catch (const std::exception& e) {
                std::cout << "Exception: " << e.what() << std::endl;
                res = cv::Mat(RES_WIDTH * arraySize, RES_HEIGHT, CV_8UC3, cv::Scalar(0, 0, 0)); // black image
            }            

            return res;
        }

        /// <summary>Stack images Vertically</summary>
        /// <param name='images'>Array of images to stack</param>
        /// <param name='arraySize'>Amount of images to concat</param>
        /// <param name='width'>Width of each image in the array. Pass 0 to automatically calculate it.</param>
        static cv::Mat VStackWithPad(cv::Mat* images, uint8_t arraySize, ushort width = 0) {
            int maxWidth = 0;
            cv::Mat res;

            assert(arraySize > 0);

            if (width == 0) {
                // calculate the ideal width (bigger)
                for (uint8_t i = 0; i < arraySize; i++) {
                    if (images[i].cols > maxWidth)
                        maxWidth = images[i].cols;
                }
            } else maxWidth = width;

            for (uint8_t i = 0; i < arraySize; i++) {
                if (images[i].cols != maxWidth)
                    AddPad(images[i], 0, (maxWidth - images[i].cols));                
            }

            cv::vconcat(images, arraySize, res);
            return res;
        }

        /// <summary>Stack images Horizontally with limit per row, then stack the hstacked rows vertically</summary>
        /// <param name='images'>Array of images to stack</param>
        /// <param name='arraySize'>Amount of images to stack</param>
        /// <param name='maxHStack'>Amount of images to stack horizontally on each row</param>
        static cv::Mat StackImages(cv::Mat* images, uint8_t arraySize, uint8_t maxHStack = 2) {
            std::vector<cv::Mat> hstacked;
            uint8_t count = 0;

            assert(maxHStack <= arraySize);

            if (arraySize == maxHStack) {
                return HStackWithPad(&images[0], maxHStack, RES_HEIGHT);
            }

            while (count <= (arraySize - maxHStack)) {
                hstacked.push_back(HStackWithPad(&images[count], maxHStack, RES_HEIGHT));
                count += maxHStack;
            }

            // (arraySize - count) = images left
            if ((arraySize - count) == 1) {
                hstacked.push_back(images[count]);
            } else if ((arraySize - count) > 1) {
                hstacked.push_back(StackImages(&images[count], (arraySize - count), (arraySize - count)));
            }

            // pass 0 since its the width of two images Hstacked.
            return VStackWithPad(&hstacked[0], hstacked.size(), 0);
        }
};

