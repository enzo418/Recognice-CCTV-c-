#pragma once

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <exception>
#include <fstream>

#include "../../Blob/BlobDetector/BlobDetectorParams.hpp"
#include "../../Blob/BlobDetector/BlobFilters.hpp"
#include "../../Blob/Contours/ContoursFilters.hpp"
#include "../../Blob/FramesProcessor/ThresholdingParams.hpp"
#include "../../Log/log.hpp"
#include "../../Utils/SpecialEnums.hpp"
#include "../../Utils/SpecialStrings.hpp"
#include "Configuration.hpp"
#include "NotificationsServiceConfiguration.hpp"

#define ExistsInVector(svector, find_this)                          \
    std::find(std::begin(svector), std::end(svector), find_this) != \
        std::end(svector)

namespace YAML {
    template <>
    struct convert<Observer::Configuration::ResizeNotification> {
        static Node encode(
            const Observer::Configuration::ResizeNotification& rhs) {
            Node node;

            node["image"] = rhs.image;
            node["video"] = rhs.video;
            return node;
        }

        static bool decode(const Node& node,
                           Observer::Configuration::ResizeNotification& rhs) {
            rhs.image = node["image"].as<int>();
            rhs.video = node["video"].as<int>();
            return true;
        }
    };

    template <>
    struct convert<Observer::Configuration> {
        static Node encode(const Observer::Configuration& rhs) {
            Node node;

            node["name"] = rhs.name;
            node["mediaFolderPath"] = rhs.mediaFolderPath;
            node["notificationTextTemplate"] = rhs.notificationTextTemplate;
            node["telegramNotificationsConfiguration"] =
                rhs.telegramConfiguration;
            node["localWebNotificationsConfiguration"] =
                rhs.localWebConfiguration;
            node["outputPreviewConfiguration"] = rhs.outputConfiguration;
            node["cameraConfiguration"] = rhs.camerasConfiguration;
            node["resizeNotifications"] = rhs.resizeNotifications;
            return node;
        }

        static bool decode(const Node& node, Observer::Configuration& rhs) {
            //            auto cfgNode = node["configuration"];
            
            rhs.name = node["name"].as<std::string>();
            rhs.mediaFolderPath = node["mediaFolderPath"].as<std::string>();
            rhs.notificationTextTemplate =
                node["notificationTextTemplate"].as<std::string>();
            rhs.localWebConfiguration =
                node["localWebNotificationsConfiguration"]
                    .as<Observer::LocalWebNotificationsConfiguration>();
            rhs.telegramConfiguration =
                node["telegramNotificationsConfiguration"]
                    .as<Observer::TelegramNotificationsConfiguration>();
            rhs.outputConfiguration =
                node["outputPreviewConfiguration"]
                    .as<Observer::OutputPreviewConfiguration>();
            rhs.camerasConfiguration =
                node["cameraConfiguration"]
                    .as<std::vector<Observer::CameraConfiguration>>();
            rhs.resizeNotifications =
                node["resizeNotifications"]
                    .as<Observer::Configuration::ResizeNotification>();
            return true;
        }
    };

    void EncodeNotificationsServiceConfiguration(
        Node& node, const Observer::NotificationsServiceConfiguration& cfg);

    void DecodeNotificationsServiceConfiguration(
        const Node& node, Observer::NotificationsServiceConfiguration& cfg);
    template <>
    struct convert<Observer::NotificationsServiceConfiguration> {
        static Node encode(
            const Observer::NotificationsServiceConfiguration& rhs) {
            Node node;
            node["enabled"] = rhs.enabled;
            node["secondsBetweenTextNotification"] =
                rhs.secondsBetweenTextNotification;
            node["secondsBetweenImageNotification"] =
                rhs.secondsBetweenImageNotification;
            node["secondsBetweenVideoNotification"] =
                rhs.secondsBetweenVideoNotification;
            node["noticationsToSend"] = rhs.noticationsToSend;
            node["onNotifSendExtraImageNotfWithAllTheCameras"] =
                rhs.onNotifSendExtraImageNotfWithAllTheCameras;
            node["drawTraceOfChangeOn"] = rhs.drawTraceOfChangeOn;
            return node;
        }

        static bool decode(const Node& node,
                           Observer::NotificationsServiceConfiguration& rhs) {
            rhs.enabled = node["enabled"].as<bool>();
            rhs.secondsBetweenTextNotification =
                node["secondsBetweenTextNotification"].as<double>();
            rhs.secondsBetweenImageNotification =
                node["secondsBetweenImageNotification"].as<double>();
            rhs.secondsBetweenVideoNotification =
                node["secondsBetweenVideoNotification"].as<double>();
            rhs.noticationsToSend =
                node["noticationsToSend"].as<Observer::ENotificationType>();
            rhs.onNotifSendExtraImageNotfWithAllTheCameras =
                node["onNotifSendExtraImageNotfWithAllTheCameras"].as<bool>();
            rhs.drawTraceOfChangeOn =
                node["drawTraceOfChangeOn"].as<Observer::ETrazable>();
            return true;
        }
    };

    template <>
    struct convert<Observer::LocalWebNotificationsConfiguration> {
        static Node encode(
            const Observer::LocalWebNotificationsConfiguration& rhs) {
            // call encode of "superclass"
            Node node =
                convert<Observer::NotificationsServiceConfiguration>::encode(
                    rhs);
            node["webServerUrl"] = rhs.webServerUrl;

            return node;
        }

        static bool decode(const Node& node,
                           Observer::LocalWebNotificationsConfiguration& rhs) {
            convert<Observer::NotificationsServiceConfiguration>::decode(
                node,
                dynamic_cast<Observer::NotificationsServiceConfiguration&>(
                    rhs));
            rhs.webServerUrl = node["webServerUrl"].as<std::string>();
            return true;
        }
    };

    template <>
    struct convert<Observer::TelegramNotificationsConfiguration> {
        static Node encode(
            const Observer::TelegramNotificationsConfiguration& rhs) {
            Node node =
                convert<Observer::NotificationsServiceConfiguration>::encode(
                    rhs);

            node["apiKey"] = rhs.apiKey;
            node["chatID"] = rhs.chatID;
            return node;
        }

        static bool decode(const Node& node,
                           Observer::TelegramNotificationsConfiguration& rhs) {
            convert<Observer::NotificationsServiceConfiguration>::decode(
                node,
                dynamic_cast<Observer::NotificationsServiceConfiguration&>(
                    rhs));

            rhs.apiKey = node["apiKey"].as<std::string>();
            rhs.chatID = node["chatID"].as<std::string>();
            return true;
        }
    };

    template <>
    struct convert<Observer::OutputPreviewConfiguration> {
        static Node encode(const Observer::OutputPreviewConfiguration& rhs) {
            Node node;

            node["showOutput"] = rhs.showOutput;
            node["resolution"] = rhs.resolution;
            node["scaleFactor"] = rhs.scaleFactor;
            node["showIgnoredAreas"] = rhs.showIgnoredAreas;
            node["showProcessedFrames"] = rhs.showProcessedFrames;
            return node;
        }

        static bool decode(const Node& node,
                           Observer::OutputPreviewConfiguration& rhs) {
            rhs.showOutput = node["showOutput"].as<bool>();
            rhs.resolution = node["resolution"].as<Observer::Size>();
            rhs.scaleFactor = node["scaleFactor"].as<double>();
            rhs.showIgnoredAreas = node["showIgnoredAreas"].as<bool>();
            rhs.showProcessedFrames = node["showProcessedFrames"].as<bool>();

            return true;
        }
    };

    template <>
    struct convert<Observer::ProcessingConfiguration> {
        static Node encode(const Observer::ProcessingConfiguration& rhs) {
            Node node;

            node["roi"] = rhs.roi;
            node["noiseThreshold"] = rhs.noiseThreshold;
            node["resize"] = rhs.resize;
            return node;
        }

        static bool decode(const Node& node,
                           Observer::ProcessingConfiguration& rhs) {
            rhs.noiseThreshold = node["noiseThreshold"].as<double>();
            rhs.roi = node["roi"].as<Observer::Rect>();
            rhs.resize = node["resize"].as<Observer::Size>();

            return true;
        }
    };

    template <>
    struct convert<Observer::CameraConfiguration> {
        static Node encode(const Observer::CameraConfiguration& rhs) {
            Node node;

            node["name"] = rhs.name;
            node["url"] = rhs.url;
            node["fps"] = rhs.fps;
            node["positionOnOutput"] = rhs.positionOnOutput;
            node["rotation"] = rhs.rotation;
            node["type"] = rhs.type;
            node["minimumChangeThreshold"] = rhs.minimumChangeThreshold;
            node["increaseThresholdFactor"] = rhs.increaseThresholdFactor;
            node["secondsBetweenTresholdUpdate"] =
                rhs.secondsBetweenTresholdUpdate;
            node["saveDetectedChangeInVideo"] = rhs.saveDetectedChangeInVideo;
            node["ignoredAreas"] = rhs.ignoredAreas;
            node["videoValidatorBufferSize"] = rhs.videoValidatorBufferSize;
            node["restrictedAreas"] = rhs.restrictedAreas;
            node["objectDetectionMethod"] = rhs.objectDetectionMethod;
            node["BlobDetection"] = rhs.blobDetection;
            node["Processing"] = rhs.processingConfiguration;
            node["ResizeTo"] = rhs.resizeTo;

            return node;
        }

        static bool decode(const Node& node,
                           Observer::CameraConfiguration& rhs) {
            rhs.name = node["name"].as<std::string>();
            rhs.url = node["url"].as<std::string>();
            rhs.fps = node["fps"].as<double>();
            rhs.positionOnOutput = node["positionOnOutput"].as<int>();
            rhs.rotation = node["rotation"].as<double>();
            rhs.type = node["type"].as<Observer::ECameraType>();
            rhs.minimumChangeThreshold =
                node["minimumChangeThreshold"].as<int>();
            rhs.increaseThresholdFactor =
                node["increaseThresholdFactor"].as<double>();
            rhs.secondsBetweenTresholdUpdate =
                node["secondsBetweenTresholdUpdate"].as<int>();
            rhs.saveDetectedChangeInVideo =
                node["saveDetectedChangeInVideo"].as<bool>();
            rhs.ignoredAreas =
                node["ignoredAreas"].as<std::vector<Observer::Rect>>();
            rhs.videoValidatorBufferSize =
                node["videoValidatorBufferSize"].as<int>();
            rhs.restrictedAreas =
                node["restrictedAreas"]
                    .as<std::vector<Observer::RestrictedArea>>();
            rhs.objectDetectionMethod =
                node["objectDetectionMethod"]
                    .as<Observer::EObjectDetectionMethod>();
            rhs.blobDetection = node["BlobDetection"]
                                    .as<Observer::BlobDetectionConfiguration>();
            rhs.processingConfiguration =
                node["Processing"].as<Observer::ProcessingConfiguration>();
            rhs.resizeTo = node["ResizeTo"].as<Observer::Size>();
            return true;
        }
    };

    template <>
    struct convert<Observer::Rect> {
        static Node encode(const Observer::Rect& rhs) {
            Node node;

            node["x"] = rhs.x;
            node["y"] = rhs.y;
            node["width"] = rhs.width;
            node["height"] = rhs.height;
            return node;
        }

        static bool decode(const Node& node, Observer::Rect& rhs) {
            rhs.x = node["x"].as<int>();
            rhs.y = node["y"].as<int>();
            rhs.width = node["width"].as<int>();
            rhs.height = node["height"].as<int>();

            return true;
        }
    };

    template <>
    struct convert<Observer::Size> {
        static Node encode(const Observer::Size& rhs) {
            Node node;

            node["width"] = rhs.width;
            node["height"] = rhs.height;
            return node;
        }

        static bool decode(const Node& node, Observer::Size& rhs) {
            rhs.width = node["width"].as<int>();
            rhs.height = node["height"].as<int>();

            return true;
        }
    };

    template <>
    struct convert<Observer::Point> {
        static Node encode(const Observer::Point& rhs) {
            Node node;

            node["x"] = rhs.x;
            node["y"] = rhs.y;
            return node;
        }

        static bool decode(const Node& node, Observer::Point& rhs) {
            rhs.x = node["x"].as<int>();
            rhs.y = node["y"].as<int>();

            return true;
        }
    };

    template <>
    struct convert<Observer::RestrictedArea> {
        static Node encode(const Observer::RestrictedArea& rhs) {
            Node node;

            node["points"] = rhs.points;
            node["type"] = rhs.type;
            return node;
        }

        static bool decode(const Node& node, Observer::RestrictedArea& rhs) {
            rhs.points = node["points"].as<std::vector<Observer::Point>>();
            rhs.type = node["type"].as<Observer::ERestrictionType>();

            return true;
        }
    };

    template <>
    struct convert<Observer::EObjectDetectionMethod> {
       private:
        using RType = Observer::EObjectDetectionMethod;

       public:
        static Node encode(const Observer::EObjectDetectionMethod& rhs) {
            Node node;

            switch (rhs) {
                case RType::NONE:
                    node = "None";
                    break;
                case RType::HOG_DESCRIPTOR:
                    node = "Hog Descriptor";
                    break;
                case RType::YOLODNN_V4:
                    node = "Yolo DNN V4";
                    break;
            }

            return node;
        }

        static bool decode(const Node& node,
                           Observer::EObjectDetectionMethod& rhs) {
            std::string val;

            try {
                val = node.as<std::string>();
            } catch (const BadConversion& e) {
                return false;
            }

            Observer::StringUtility::StringToLower(val);

            if (val == "none") {
                rhs = RType::NONE;
            } else if (val == "hog descriptor") {
                rhs = RType::HOG_DESCRIPTOR;
            } else if (val == "yolo dnn v4") {
                rhs = RType::YOLODNN_V4;
            }

            return true;
        }
    };

    template <>
    struct convert<Observer::ERestrictionType> {
       private:
        using RType = Observer::ERestrictionType;

       public:
        static Node encode(const Observer::ERestrictionType& rhs) {
            Node node;

            switch (rhs) {
                case RType::ALLOW:
                    node = "Allow";
                    break;
                case RType::DENY:
                    node = "Deny";
                    break;
            }

            return node;
        }

        static bool decode(const Node& node, Observer::ERestrictionType& rhs) {
            std::string val;

            try {
                val = node.as<std::string>();
            } catch (const BadConversion& e) {
                return false;
            }

            Observer::StringUtility::StringToLower(val);

            if (val == "allow") {
                rhs = RType::ALLOW;
            } else if (val == "deny") {
                rhs = RType::DENY;
            }

            return true;
        }
    };

    template <>
    struct convert<Observer::ENotificationType> {
       private:
        using RType = Observer::ENotificationType;

       public:
        static Node encode(const Observer::ENotificationType& rhs) {
            Node node;
            std::vector<std::string> out;

            if (has_flag(rhs, RType::TEXT)) {
                out.emplace_back("Text");
            }

            if (has_flag(rhs, RType::IMAGE)) {
                out.emplace_back("Image");
            }

            if (has_flag(rhs, RType::VIDEO)) {
                out.emplace_back("Video");
            }

            node = out;

            return node;
        }

        static bool decode(const Node& node, Observer::ENotificationType& rhs) {
            if (!node.IsSequence()) {
                return false;
            }
            std::vector<std::string> types;

            rhs = Observer::ENotificationType::NONE;

            try {
                types = node.as<std::vector<std::string>>();
            } catch (const BadConversion& e) {
                return false;
            }

            for (auto& val : types) {
                Observer::StringUtility::StringToLower(val);
            }

            if (ExistsInVector(types, "text")) {
                rhs |= RType::TEXT;
            }

            if (ExistsInVector(types, "image")) {
                rhs |= RType::IMAGE;
            }

            if (ExistsInVector(types, "video")) {
                rhs |= RType::VIDEO;
            }

            return true;
        }
    };

    template <>
    struct convert<Observer::ECameraType> {
       private:
        using RType = Observer::ECameraType;

       public:
        static Node encode(const Observer::ECameraType& rhs) {
            Node node;

            switch (rhs) {
                case RType::DISABLED:
                    node = "Disabled";
                    break;
                case RType::NOTIFICATOR:
                    node = "Notificator";
                    break;
                case RType::OBJECT_DETECTOR:
                    node = "Object Detector";
                    break;
                case RType::VIEW:
                    node = "View";
                    break;
            }

            return node;
        }

        static bool decode(const Node& node, Observer::ECameraType& rhs) {
            std::string val;

            try {
                val = node.as<std::string>();
            } catch (const BadConversion& e) {
                return false;
            }

            Observer::StringUtility::StringToLower(val);

            if (val == "disabled") {
                rhs = RType::DISABLED;
            } else if (val == "notificator") {
                rhs = RType::NOTIFICATOR;
            } else if (val == "object detector") {
                rhs = RType::OBJECT_DETECTOR;
            } else if (val == "view") {
                rhs = RType::VIEW;
            }

            return true;
        }
    };

    template <>
    struct convert<Observer::ETrazable> {
       private:
        using RType = Observer::ETrazable;

       public:
        static Node encode(const Observer::ETrazable& rhs) {
            Node node;
            std::vector<std::string> out;

            if (has_flag(rhs, RType::IMAGE)) {
                out.emplace_back("Image");
            }

            if (has_flag(rhs, RType::VIDEO)) {
                out.emplace_back("Video");
            }

            node = out;

            return node;
        }

        static bool decode(const Node& node, Observer::ETrazable& rhs) {
            if (!node.IsSequence()) {
                return false;
            }

            rhs = Observer::ETrazable::NONE;

            std::vector<std::string> types;

            try {
                types = node.as<std::vector<std::string>>();
            } catch (const BadConversion& e) {
                return false;
            }

            for (auto& val : types) {
                Observer::StringUtility::StringToLower(val);
            }

            if (ExistsInVector(types, "image")) {
                set_flag(rhs, RType::IMAGE);
            }

            if (ExistsInVector(types, "video")) {
                set_flag(rhs, RType::VIDEO);
            }

            return true;
        }
    };

    template <>
    struct convert<Observer::BlobDetectionConfiguration> {
       private:
        using RType = Observer::BlobDetectionConfiguration;

       public:
        static Node encode(const RType& rhs) {
            Node node;

            node["BlobDetectionParameters"] = rhs.blobDetectorParams;
            node["BlobFilters"] = rhs.blobFilters;
            node["ContoursFilters"] = rhs.contoursFilters;
            node["ThresholdingParameters"] = rhs.thresholdingParams;
            return node;
        }

        static bool decode(const Node& node, RType& rhs) {
            rhs.blobDetectorParams = node["BlobDetectionParameters"]
                                         .as<Observer::BlobDetectorParams>();
            rhs.blobFilters = node["BlobFilters"].as<Observer::BlobFilters>();
            rhs.contoursFilters =
                node["ContoursFilters"].as<Observer::ContoursFilter>();
            rhs.thresholdingParams = node["ThresholdingParameters"]
                                         .as<Observer::ThresholdingParams>();
            return true;
        }
    };

    template <>
    struct convert<Observer::BlobFilters::BlobVelocityFilters> {
       private:
        using RType = Observer::BlobFilters::BlobVelocityFilters;

       public:
        static Node encode(const RType& rhs) {
            Node node;

            node["UseVelocityFilter"] = rhs.UseVelocityFilter;
            node["MinVelocity"] = rhs.MinVelocity;
            node["MaxVelocity"] = rhs.MaxVelocity;
            return node;
        }

        static bool decode(const Node& node, RType& rhs) {
            rhs.UseVelocityFilter = node["UseVelocityFilter"].as<bool>();
            rhs.MinVelocity = node["MinVelocity"].as<int>();
            rhs.MaxVelocity = node["MaxVelocity"].as<int>();

            return true;
        }
    };

    template <>
    struct convert<Observer::BlobFilters> {
       private:
        using RType = Observer::BlobFilters;

       public:
        static Node encode(const RType& rhs) {
            Node node;

            node["BlobVelocityFilter"] = rhs.VelocityFilter;
            node["MinimumOccurrences"] = rhs.MinimumOccurrences;
            node["MinimumUnitsTraveled"] = rhs.MinimumUnitsTraveled;
            return node;
        }

        static bool decode(const Node& node, RType& rhs) {
            rhs.MinimumOccurrences = node["MinimumOccurrences"].as<int>();
            rhs.MinimumUnitsTraveled = node["MinimumUnitsTraveled"].as<int>();
            rhs.VelocityFilter =
                node["BlobVelocityFilter"].as<RType::BlobVelocityFilters>();

            return true;
        }
    };

    template <>
    struct convert<Observer::BlobDetectorParams> {
       private:
        using RType = Observer::BlobDetectorParams;

       public:
        static Node encode(const RType& rhs) {
            Node node;

            node["DistanceThreshold"] = rhs.distance_thresh;
            node["SimilarityThreshold"] = rhs.similarity_threshold;
            node["BlobMaxLife"] = rhs.blob_max_life;
            return node;
        }

        static bool decode(const Node& node, RType& rhs) {
            rhs.distance_thresh = node["DistanceThreshold"].as<int>();
            rhs.similarity_threshold = node["SimilarityThreshold"].as<double>();
            rhs.blob_max_life = node["BlobMaxLife"].as<int>();
            return true;
        }
    };

    template <>
    struct convert<Observer::ContoursFilter::IgnoredAreas> {
       private:
        using RType = Observer::ContoursFilter::IgnoredAreas;

       public:
        static Node encode(const RType& rhs) {
            Node node;

            node["MinAreaPercentageToIgnore"] = rhs.minAreaPercentageToIgnore;
            node["Areas"] = rhs.areas;
            node["Reference"] = rhs.reference;
            return node;
        }

        static bool decode(const Node& node, RType& rhs) {
            rhs.minAreaPercentageToIgnore =
                node["MinAreaPercentageToIgnore"].as<double>();
            rhs.areas = node["Areas"].as<std::vector<Observer::Rect>>();
            rhs.reference = node["Reference"].as<Observer::Size>();
            return true;
        }
    };

    template <>
    struct convert<Observer::ContoursFilter::IgnoredSets> {
       private:
        using RType = Observer::ContoursFilter::IgnoredSets;

       public:
        static Node encode(const RType& rhs) {
            Node node;

            node["Sets"] = rhs.sets;
            node["Reference"] = rhs.reference;
            node["Reference"] = rhs.reference;
            node["MinPercentageToIgnore"] = rhs.minPercentageToIgnore;

            return node;
        }

        static bool decode(const Node& node, RType& rhs) {
            rhs.sets =
                node["Sets"].as<std::vector<std::vector<Observer::Point>>>();
            rhs.reference = node["Reference"].as<Observer::Size>();
            rhs.minPercentageToIgnore =
                node["MinPercentageToIgnore"].as<double>();

            return true;
        }
    };

    template <>
    struct convert<Observer::ContoursFilter> {
       private:
        using RType = Observer::ContoursFilter;

       public:
        static Node encode(const RType& rhs) {
            Node node;

            node["FilterByAverageArea"] = rhs.FilterByAverageArea;
            node["MinimumArea"] = rhs.MinimumArea;
            node["IgnoredAreas"] = rhs.ignoredAreas;
            node["IgnoredSets"] = rhs.ignoredSets;
            return node;
        }

        static bool decode(const Node& node, RType& rhs) {
            rhs.FilterByAverageArea = node["FilterByAverageArea"].as<bool>();
            rhs.MinimumArea = node["MinimumArea"].as<int>();
            rhs.ignoredAreas = node["IgnoredAreas"].as<RType::IgnoredAreas>();
            rhs.ignoredSets = node["IgnoredSets"].as<RType::IgnoredSets>();
            return true;
        }
    };

    template <>
    struct convert<Observer::ThresholdingParams::ResizeParam> {
       private:
        using RType = Observer::ThresholdingParams::ResizeParam;

       public:
        static Node encode(const RType& rhs) {
            Node node;

            node["Resize"] = rhs.resize;
            node["Size"] = rhs.size;
            return node;
        }

        static bool decode(const Node& node, RType& rhs) {
            rhs.resize = node["Resize"].as<bool>();
            rhs.size = node["Size"].as<Observer::Size>();

            return true;
        }
    };

    template <>
    struct convert<Observer::ThresholdingParams> {
       private:
        using RType = Observer::ThresholdingParams;

       public:
        static Node encode(const RType& rhs) {
            Node node;

            node["ResizeParameters"] = rhs.Resize;
            node["FramesBetweenDiffFrames"] = rhs.FramesBetweenDiffFrames;
            node["ContextFrames"] = rhs.ContextFrames;
            node["MedianBlurKernelSize"] = rhs.MedianBlurKernelSize;
            node["GaussianBlurKernelSize"] = rhs.GaussianBlurKernelSize;
            node["DilationSize"] = rhs.DilationSize;
            node["BrightnessAboveThreshold"] = rhs.BrightnessAboveThreshold;

            return node;
        }

        static bool decode(const Node& node, RType& rhs) {
            rhs.Resize = node["ResizeParameters"].as<RType::ResizeParam>();
            rhs.FramesBetweenDiffFrames =
                node["FramesBetweenDiffFrames"].as<int>();
            rhs.ContextFrames = node["ContextFrames"].as<int>();
            rhs.MedianBlurKernelSize = node["MedianBlurKernelSize"].as<int>();
            rhs.GaussianBlurKernelSize =
                node["GaussianBlurKernelSize"].as<int>();
            rhs.DilationSize = node["DilationSize"].as<int>();
            rhs.BrightnessAboveThreshold =
                node["BrightnessAboveThreshold"].as<int>();
            return true;
        }
    };
}  // namespace YAML
