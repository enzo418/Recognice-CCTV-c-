#pragma once

#include <string>
#include <opencv2/opencv.hpp>
#include "external/bitmask_operators.hpp"

namespace Observer
{
    enum ERestrictionType
    {
        ALLOW = 1,
        DENY = 2
    };

    struct RestrictedArea
    {
        std::vector<cv::Point> points;
        ERestrictionType type;

        bool operator==(const RestrictedArea&) const = default;
    };

    enum ECameraType
    {
        DISABLED = 1,
        NOTIFICATOR = 2,
        OBJECT_DETECTOR = 4
    };

    enum EObjectDetectionMethod
    {
        NONE = 1,
        HOG_DESCRIPTOR = 2,
        YOLODNN_V4 = 4
    };

    struct CameraConfiguration
    {
        std::string name;

        std::string url;
        
        // camera max fps to use. Lower fps lowers the CPU usage
        double fps;

        // region of interest
        cv::Rect roi;

        // position of the camera in the preview. 0 = top left
        int positionOnOutput;
        
        // rotation of the camera in degrees
        double rotation;

        ECameraType type;

        // value used to remove noise in the image
        double noiseThreshold = 45;

        // minimum ammount of pixel that changed to trigger a validator
        int minimumChangeThreshold = 5;

        // factor to increse the threshold when updating it
        double increaseThresholdFactor = 1.2;

        // seconds to wait before updating the camera threshold again
        int secondsBetweenTresholdUpdate = 15;

        // TODO: what is it
        bool saveDetectedChangeInVideo = true;

        // list of rectangles to ignore if a change happens inside it
        std::vector<cv::Rect> ignoredAreas;

        // How much frames to use to validate the change
        int videoValidatorBufferSize = 30;

        // Restricted areas to check when validating a change
        std::vector<RestrictedArea> restrictedAreas;

        // Method to dected the objects
        EObjectDetectionMethod objectDetectionMethod;

        bool operator==(const CameraConfiguration&) const = default;
    };

} // namespace Observer
