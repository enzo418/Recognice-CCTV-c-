#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <vector>
#include "types.hpp"

namespace ImageManipulation {
	void RotateImage(cv::Mat& image, double angle);

	/// <summary>Draw a circle into the given image </summary>
	/// <param name='image'>Image to draw the circle</param>
	/// <param name='coords'>array of 3 doubles that have the coords in this position: 0=xA, 1=yA, 2=xB, 3=yB </param>
	void DrawCircle(cv::Mat& image, double* coords);

	/// <summary>Add pad to iamge</summary>
	/// <param name='bottom'>Pad to add below the image</param>
	/// <param name='right'>Pad to add in the right side of the image</param>
	void AddPad(cv::Mat& image, ulong bottom = 0, ulong right = 0);

	/// <summary>Stack images Horizontally</summary>
	/// <param name='images'>Array of images to stack</param>
	/// <param name='arraySize'>Amount of images to concat</param>
	/// <param name='height'>Height of each image in the array. Pass 0 to automatically calculate it.</param>
	cv::Mat HStackWithPad(cv::Mat* images, ushort arraySize, ushort height = 0);

	/// <summary>Stack images Vertically</summary>
	/// <param name='images'>Array of images to stack</param>
	/// <param name='arraySize'>Amount of images to concat</param>
	/// <param name='width'>Width of each image in the array. Pass 0 to automatically calculate it.</param>
	cv::Mat VStackWithPad(cv::Mat* images, uint8_t arraySize, ushort width = 0);

	/// <summary>Stack images Horizontally with limit per row, then stack the hstacked rows vertically</summary>
	/// <param name='images'>Array of images to stack</param>
	/// <param name='arraySize'>Amount of images to stack</param>
	/// <param name='maxHStack'>Amount of images to stack horizontally on each row</param>
	cv::Mat StackImages(cv::Mat* images, uint8_t arraySize, uint8_t maxHStack = 2);
};

