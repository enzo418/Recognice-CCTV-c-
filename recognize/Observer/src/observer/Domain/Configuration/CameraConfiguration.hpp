#pragma once

#include <opencv2/opencv.hpp>
#include <string>

#include "../../../../vendor/bitmask_operators.hpp"
#include "observer/Blob/BlobDetectionConfiguration.hpp"
#include "observer/Point.hpp"
#include "observer/Rect.hpp"

namespace Observer {
    enum ERestrictionType { ALLOW = 1, DENY = 2 };

    struct RestrictedArea {
        std::vector<Point> points;
        ERestrictionType type;

        bool operator==(const RestrictedArea&) const = default;
    };

    enum ECameraType {
        DISABLED = 1,
        NOTIFICATOR = 2,
        OBJECT_DETECTOR = 4,
        VIEW  // only show images
    };

    enum EObjectDetectionMethod {
        NONE = 1,
        HOG_DESCRIPTOR = 2,
        YOLODNN_V4 = 4
    };

    typedef std::vector<Observer::Point> Mask;

    struct ProcessingConfiguration {
        // resize all the frames before processing to save cpu/memory
        Size resize {640, 360};

        // value used to remove noise in the image
        double noiseThreshold = 45;

        // region of interest, relative to the camera resolution.
        // roi.x + roi.width <= resize.width & roi.y +
        // roi.height <= resize.height
        Rect roi;

        // Mask parts of the image out at the processing/detection stage
        std::vector<Mask> masks;

        bool operator==(const ProcessingConfiguration&) const = default;
    };

    struct CameraConfiguration {
        std::string name {"camera_name"};

        std::string url;

        // resize each frame received to this size.
        // If the camera send 1280x760 you can resize it to 640x360 or
        // 1920x1080.
        Size resizeTo {1280, 360};

        // camera max fps to use. Lower fps lowers the CPU usage
        double fps {20};

        // position of the camera in the preview. 0 = top left
        int positionOnOutput {0};

        // rotation of the camera in degrees
        double rotation {0};

        ECameraType type {ECameraType::VIEW};

        // minimum amount of pixel that changed to trigger a validator
        int minimumChangeThreshold = 100;

        // factor to increase the threshold when updating it
        double increaseThresholdFactor = 1.2;

        // seconds to wait before updating the camera threshold again
        int secondsBetweenThresholdUpdate = 15;

        // How much frames to use to validate the change
        int videoValidatorBufferSize = 30;

        // Method to detect the objects
        EObjectDetectionMethod objectDetectionMethod {
            EObjectDetectionMethod::NONE};

        BlobDetectionConfiguration blobDetection;

        ProcessingConfiguration processingConfiguration;

        bool operator==(const CameraConfiguration&) const = default;
    };

    struct {
        bool operator()(const CameraConfiguration& struct1,
                        const CameraConfiguration& struct2) {
            return (struct1.positionOnOutput < struct2.positionOnOutput);
        }
    } CompareCameraConfigurationsByPreviewOrder;

}  // namespace Observer
