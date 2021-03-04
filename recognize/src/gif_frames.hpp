#pragma once
#include <iostream>
#include <vector>
#include <queue>

#include <opencv2/core.hpp>

#include "types_configuration.hpp"
#include "image_manipulation.hpp"
#include "change_descriptor.hpp"

enum State  { 	
				Initial = 0, /** Initial state: updating "before" frames **/
				Collecting, /** collecting frames */
				Ready, /** filled "after" frames, ready to send **/
				Wait, /** Do not send... yet **/
				Send, /** green flag to continue **/
				Cancelled /** red flag. Delete the notification **/
			};

struct FrameDescriptor {
	cv::Mat frame;
	FindingInfo finding;
};

class GifFrames {
	private:
		ProgramConfiguration* program;
		CameraConfiguration* camera;

		// numbers of frames before
		size_t framesBefore;

		// numbers of frames after
		size_t framesAfter;

		size_t totalFramesBefore = 0;

		// if should update the frames before the change
		bool updateBefore = true;

		// if should update the frames after the change
		bool updateAfter = false;

		size_t indexBefore = 0;
		// vector of frames "before"
		std::queue<cv::Mat> before;


		size_t indexAfter = 0;
		// vector of frames "after"
		std::queue<cv::Mat> after;

		// current state of this gif
		State state = State::Initial;

		double avrgDistanceFrames = 0;

		double avrgAreaDifference = 0;

		std::string debugMessage;

		// original frames
		std::vector<cv::Mat> frames;

		// frames used to get the diff between the findings
		std::vector<FrameDescriptor> framesTransformed;
		
		// first frame of "after" in which a difference was found
		cv::Mat firstFrameWithDescription;

		void framesToSingleVectors();

		std::vector<cv::Point> findingTrace;

	public:
		GifFrames(ProgramConfiguration* programConfig, CameraConfiguration* cameraConfig);

		void addFrame(cv::Mat& frame);

		std::vector<cv::Mat> getGifFrames(bool applyTransformations = false);

		State getState();

		std::string getText();

		void detectedChange();

		bool isValid();

		size_t indexMiddleFrame();

		cv::Mat& firstFrameWithChangeDetected();

		std::vector<cv::Point> getFindingTrace();
};