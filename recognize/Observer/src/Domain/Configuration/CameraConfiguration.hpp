#pragma once

#include <opencv2/opencv.hpp>
#include <string>

#include "../../../vendor/bitmask_operators.hpp"
#include "../../Blob/BlobDetectionConfiguration.hpp"
#include "../../Point.hpp"
#include "../../Rect.hpp"

namespace Observer {
    enum ERestrictionType { ALLOW = 1, DENY = 2 };

    struct RestrictedArea {
        std::vector<Point> points;
        ERestrictionType type;

        bool operator==(const RestrictedArea&) const = default;
    };

    enum ECameraType { DISABLED = 1, NOTIFICATOR = 2, OBJECT_DETECTOR = 4 };

    enum EObjectDetectionMethod {
        NONE = 1,
        HOG_DESCRIPTOR = 2,
        YOLODNN_V4 = 4
    };

    struct ProcessingConfiguration {
        // resize all the frames before processing to save cpu/memory
        Size resize;

        // value used to remove noise in the image
        double noiseThreshold = 45;

        // region of interest, taken from resize. That means that the relation
        // is roi.x + roi.width <= resize.width &
        // roi.y + roi.height <= resize.height
        Rect roi;

        bool operator==(const ProcessingConfiguration&) const = default;
    };

    struct CameraConfiguration {
        std::string name;

        std::string url;

        // resize each frame received to this size.
        // If the camera send 1280x760 you can resize it to 640x360 or
        // 1920x1080.
        Size resizeTo;

        // camera max fps to use. Lower fps lowers the CPU usage
        double fps;

        // position of the camera in the preview. 0 = top left
        int positionOnOutput;

        // rotation of the camera in degrees
        double rotation;

        ECameraType type;

        // minimum ammount of pixel that changed to trigger a validator
        int minimumChangeThreshold = 5;

        // factor to increse the threshold when updating it
        double increaseThresholdFactor = 1.2;

        // seconds to wait before updating the camera threshold again
        int secondsBetweenTresholdUpdate = 15;

        // TODO: what is it
        bool saveDetectedChangeInVideo = true;

        // list of rectangles to ignore if a change happens inside it
        std::vector<Rect> ignoredAreas;

        // How much frames to use to validate the change
        int videoValidatorBufferSize = 30;

        // Restricted areas to check when validating a change
        std::vector<RestrictedArea> restrictedAreas;

        // Method to dected the objects
        EObjectDetectionMethod objectDetectionMethod;

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
