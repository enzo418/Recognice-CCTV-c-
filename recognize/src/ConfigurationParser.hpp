#pragma once

#include "Configuration.hpp"
#include "LocalWebNotifications.hpp"
#include "utils/StringUtility.hpp"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <algorithm>
#include <exception>

#define ExistsInVector(svector, find_this) std::find(std::begin(svector), std::end(svector), find_this) != std::end(svector)

namespace YAML {
    template<>
    struct convert<Observer::Configuration> {
        static Node encode(const Observer::Configuration& rhs) {
            Node node;

            node["mediaFolderPath"] = rhs.mediaFolderPath;
            node["notificationTextTemplate"] = rhs.notificationTextTemplate;
            node["telegramNotificationsConfiguration"] = rhs.telegramConfiguration;
            node["localWebNotificationsConfiguration"] = rhs.localWebConfiguration;
            node["outputPreviewConfiguration"] = rhs.outputConfiguration;
            node["cameraConfiguration"] = rhs.camerasConfiguration;
            return node;
        }

        static bool decode(const Node& node, Observer::Configuration& rhs) {
//            auto cfgNode = node["configuration"];
            rhs.mediaFolderPath = node["mediaFolderPath"].as<std::string>();
            rhs.notificationTextTemplate = node["notificationTextTemplate"].as<std::string>();
            rhs.localWebConfiguration = node["localWebNotificationsConfiguration"].as<Observer::LocalWebNotificationsConfiguration>();
            rhs.telegramConfiguration = node["telegramNotificationsConfiguration"].as<Observer::TelegramNotificationsConfiguration>();
            rhs.outputConfiguration = node["outputPreviewConfiguration"].as<Observer::OutputPreviewConfiguration>();
            rhs.camerasConfiguration = node["cameraConfiguration"].as<std::vector<Observer::CameraConfiguration>>();
            return true;
        }
    };

    void EncodeNotificationsServiceConfiguration(Node& node,
                                                 const Observer::NotificationsServiceConfiguration& cfg);

    void DecodeNotificationsServiceConfiguration(const Node& node,
                                                 Observer::NotificationsServiceConfiguration& cfg);
    template<>
    struct convert<Observer::NotificationsServiceConfiguration> {
        static Node encode(const Observer::NotificationsServiceConfiguration& rhs) {
            Node node;
			node["enabled"] = rhs.enabled;
			node["secondsBetweenTextNotification"] = rhs.secondsBetweenTextNotification;
			node["secondsBetweenImageNotification"] = rhs.secondsBetweenImageNotification;
			node["secondsBetweenVideoNotification"] = rhs.secondsBetweenVideoNotification;
			node["noticationsToSend"] = rhs.noticationsToSend;
			node["onNotifSendExtraImageNotfWithAllTheCameras"] = rhs.onNotifSendExtraImageNotfWithAllTheCameras;
			node["drawTraceOfChangeOn"] = rhs.drawTraceOfChangeOn;
            return node;
        }

        static bool decode(const Node& node, Observer::NotificationsServiceConfiguration& rhs) {
			rhs.enabled = node["enabled"].as<bool>();
			rhs.secondsBetweenTextNotification = node["secondsBetweenTextNotification"].as<double>();
			rhs.secondsBetweenImageNotification = node["secondsBetweenImageNotification"].as<double>();
			rhs.secondsBetweenVideoNotification = node["secondsBetweenVideoNotification"].as<double>();
			rhs.noticationsToSend = node["noticationsToSend"].as<Observer::ENotificationType>();
			rhs.onNotifSendExtraImageNotfWithAllTheCameras = node["onNotifSendExtraImageNotfWithAllTheCameras"].as<bool>();
			rhs.drawTraceOfChangeOn = node["drawTraceOfChangeOn"].as<Observer::ETrazable>();
            return true;
        }
    };

    template<>
    struct convert<Observer::LocalWebNotificationsConfiguration> {
        static Node encode(const Observer::LocalWebNotificationsConfiguration& rhs) {
            // call encode of "superclass"
			Node node = convert<Observer::NotificationsServiceConfiguration>::encode(rhs);
            node["webServerUrl"] = rhs.webServerUrl;
			
            return node;
        }

        static bool decode(const Node& node, Observer::LocalWebNotificationsConfiguration& rhs) {
			convert<Observer::NotificationsServiceConfiguration>::decode(
				node, 
				dynamic_cast<Observer::NotificationsServiceConfiguration&>(rhs)
			);
            rhs.webServerUrl = node["webServerUrl"].as<std::string>();
            return true;
        }
    };

    template<>
    struct convert<Observer::TelegramNotificationsConfiguration> {
        static Node encode(const Observer::TelegramNotificationsConfiguration& rhs) {
            Node node = convert<Observer::NotificationsServiceConfiguration>::encode(rhs);
			
            node["apiKey"] = rhs.apiKey;
            node["chatID"] = rhs.chatID;
            return node;
        }

        static bool decode(const Node& node, Observer::TelegramNotificationsConfiguration& rhs) {
			convert<Observer::NotificationsServiceConfiguration>::decode(
				node, 
				dynamic_cast<Observer::NotificationsServiceConfiguration&>(rhs)
			);
			
			rhs.apiKey = node["apiKey"].as<std::string>();
			rhs.chatID = node["chatID"].as<std::string>();
            return true;
        }
    };

    template<>
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

        static bool decode(const Node& node, Observer::OutputPreviewConfiguration& rhs) {
            rhs.showOutput = node["showOutput"].as<bool>();
            rhs.resolution = node["resolution"].as<cv::Size>();
            rhs.scaleFactor = node["scaleFactor"].as<double>();
            rhs.showIgnoredAreas = node["showIgnoredAreas"].as<bool>();
            rhs.showProcessedFrames = node["showProcessedFrames"].as<bool>();

            return true;
        }
    };

    template<>
    struct convert<Observer::CameraConfiguration> {
        static Node encode(const Observer::CameraConfiguration& rhs) {
            Node node;

            node["name"] = rhs.name;
            node["url"] = rhs.url;
            node["fps"] = rhs.fps;
            node["roi"] = rhs.roi;
            node["positionOnOutput"] = rhs.positionOnOutput;
            node["rotation"] = rhs.rotation;
            node["type"] = rhs.type;
            node["noiseThreshold"] = rhs.noiseThreshold;
            node["minimumChangeThreshold"] = rhs.minimumChangeThreshold;
            node["increaseThresholdFactor"] = rhs.increaseThresholdFactor;
            node["secondsBetweenTresholdUpdate"] = rhs.secondsBetweenTresholdUpdate;
            node["saveDetectedChangeInVideo"] = rhs.saveDetectedChangeInVideo;
            node["ignoredAreas"] = rhs.ignoredAreas;
            node["videoValidatorBufferSize"] = rhs.videoValidatorBufferSize;
            node["restrictedAreas"] = rhs.restrictedAreas;
            node["objectDetectionMethod"] = rhs.objectDetectionMethod;
            return node;
        }

        static bool decode(const Node& node, Observer::CameraConfiguration& rhs) {
            rhs.name = node["name"].as<std::string>();
            rhs.url = node["url"].as<std::string>();
            rhs.fps = node["fps"].as<double>();
            rhs.roi = node["roi"].as<cv::Rect>();
            rhs.positionOnOutput = node["positionOnOutput"].as<int>();
            rhs.rotation = node["rotation"].as<double>();
            rhs.type = node["type"].as<Observer::ECameraType>();
            rhs.noiseThreshold = node["noiseThreshold"].as<double>();
            rhs.minimumChangeThreshold = node["minimumChangeThreshold"].as<int>();
            rhs.increaseThresholdFactor = node["increaseThresholdFactor"].as<double>();
            rhs.secondsBetweenTresholdUpdate = node["secondsBetweenTresholdUpdate"].as<int>();
            rhs.saveDetectedChangeInVideo = node["saveDetectedChangeInVideo"].as<bool>();
            rhs.ignoredAreas = node["ignoredAreas"].as<std::vector<cv::Rect>>();
            rhs.videoValidatorBufferSize = node["videoValidatorBufferSize"].as<int>();
            rhs.restrictedAreas = node["restrictedAreas"].as<std::vector<Observer::RestrictedArea>>();
            rhs.objectDetectionMethod = node["objectDetectionMethod"].as<Observer::EObjectDetectionMethod>();

            return true;
        }
    };

    template<>
    struct convert<cv::Rect> {
        static Node encode(const cv::Rect& rhs) {
            Node node;

            node["x"] = rhs.x;
            node["y"] = rhs.y;
            node["width"] = rhs.width;
            node["height"] = rhs.height;
            return node;
        }

        static bool decode(const Node& node, cv::Rect& rhs) {
            rhs.x = node["x"].as<int>();
            rhs.y = node["y"].as<int>();
            rhs.width = node["width"].as<int>();
            rhs.height = node["height"].as<int>();

            return true;
        }
    };

    template<>
    struct convert<cv::Size> {
        static Node encode(const cv::Size& rhs) {
            Node node;

            node["width"] = rhs.width;
            node["height"] = rhs.height;
            return node;
        }

        static bool decode(const Node& node, cv::Size& rhs) {
            rhs.width = node["width"].as<int>();
            rhs.height = node["height"].as<int>();

            return true;
        }
    };

    template<>
    struct convert<cv::Point> {
        static Node encode(const cv::Point& rhs) {
            Node node;

            node["x"] = rhs.x;
            node["y"] = rhs.y;
            return node;
        }

        static bool decode(const Node& node, cv::Point& rhs) {
            rhs.x = node["x"].as<int>();
            rhs.y = node["y"].as<int>();

            return true;
        }
    };

    template<>
    struct convert<Observer::RestrictedArea> {
        static Node encode(const Observer::RestrictedArea& rhs) {
            Node node;

            node["points"] = rhs.points;
            node["type"] = rhs.type;
            return node;
        }

        static bool decode(const Node& node, Observer::RestrictedArea& rhs) {
            rhs.points = node["points"].as<std::vector<cv::Point>>();
            rhs.type = node["type"].as<Observer::ERestrictionType>();

            return true;
        }
    };

    template<>
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

        static bool decode(const Node& node, Observer::EObjectDetectionMethod& rhs) {
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
    
    template<>
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
    
    template<>
    struct convert<Observer::ENotificationType> {
    private:
        using RType = Observer::ENotificationType;
    public:
        static Node encode(const Observer::ENotificationType& rhs) {
            Node node;
            std::vector<std::string> out;
            
            if ((rhs & RType::TEXT) == RType::TEXT) {
                out.emplace_back("Text");
            }
            
            if ((rhs & RType::IMAGE) == RType::IMAGE) {
                out.emplace_back("Image");
            }
            
            if ((rhs & RType::VIDEO) == RType::VIDEO) {
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

            try {
                types = node.as<std::vector<std::string>>();
            } catch (const BadConversion& e) {
                return false;
            }

            for(auto& val : types) {
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
    
    template<>
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
            }

            return true;
        }
    };
    
    template<>
    struct convert<Observer::ETrazable> {
    private:
        using RType = Observer::ETrazable;
    public:
        static Node encode(const Observer::ETrazable& rhs) {
            Node node;
            std::vector<std::string> out;
            
            if ((rhs & RType::IMAGE) == RType::IMAGE) {
                out.emplace_back("Image");
            }
            
            if ((rhs & RType::VIDEO) == RType::VIDEO) {
                out.emplace_back("Video");
            }
            
            node = out;
            
            return node;
        }

        static bool decode(const Node& node, Observer::ETrazable& rhs) {
            if (!node.IsSequence()) {
                return false;
            }
            std::vector<std::string> types;

            try {            
                types = node.as<std::vector<std::string>>();
            } catch (const BadConversion& e) {
                return false;
            }

            for(auto& val : types) {
                Observer::StringUtility::StringToLower(val);            
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
}

namespace Observer::ConfigurationParser {
    struct MissingKey : public std::exception {
        public:
            MissingKey(const std::string& pKeymissing) : keymissing(std::move(pKeymissing)) {

            }

            const char* what () const throw () {
                return ("Missing Key '" + this->keymissing + "'").c_str();
            }
            
            std::string keyMissing() const {
                return this->keymissing;
            }

        private:
            std::string keymissing;
    };
    
    struct WrongType : public std::exception {
        public:
            WrongType(int pLine, int pColumn, int pPosition) 
            : mLine(pLine), mCol(pColumn), mPos(pPosition) {

            }

            const char* what () const throw () {
                return "Bad conversion.";
            }
            
            int line () const {
                return this->mLine;
            }
            
            int column () const {
                return this->mCol;
            }
            
            int position () const {
                return this->mPos;
            }

        private:
            int mLine;
            int mCol;
            int mPos;
    };
	
    // yamlcpp
    Configuration ParseYAML(YAML::Node& node);
    void EmmitYAML(std::ofstream& fs, const Configuration& cfg);
    
    std::string ConfigurationToJson(const Configuration& cfg);
    Configuration JsonToConfiguration(YAML::Node& node);
}
