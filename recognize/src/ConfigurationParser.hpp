#pragma once

#include "Configuration.hpp"
#include "LocalWebNotifications.hpp"
#include "utils/StringUtility.hpp"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <algorithm>
#include <exception>

#define ExistsInVector(svector, find_this) std::find(std::begin(svector), std::end(svector), find_this) != std::end(svector)

namespace cv {
    // vector<Rect>
    /**
     * Custom template to avoid using opencv write templates
     * since they are just weird.
     * @tparam T
     * @param fs
     * @param x
     */
    template<typename T>
    static void write_mine(FileStorage& fs, const std::string& n, const std::vector<T>& y){
        for(auto&& x : y) {
            write(fs, n, x);
        }
    }

    /**
     * Custom template to avoid using opencv read templates
     * since they are just weird.
     * @tparam T
     * @param node
     * @param x
     * @param default_value
     */
    template<typename T>
    static void read_mine(const FileNode& node,
                          std::vector<T>& x,
                          const std::vector<T>& default_value = std::vector<T>()){
        if(node.empty())
            x = default_value;
        else {
            FileNodeIterator it = node.begin(), it_end = node.end();
            auto size = it.remaining();
            x.reserve(size);
            for (; it != it_end; ++it)
            {
                T y;
                read(*it, y);
                x.push_back(std::move(y));
            }
        }
    }

    // Configuration
    static void write(FileStorage& fs, const std::string& n, const Observer::Configuration& x){
        fs << "mediaFolderPath" << x.mediaFolderPath << "notificationTextTemplate" << x.notificationTextTemplate

        << "telegramNotificationsConfiguration" << "{";
        write(fs, x.telegramConfiguration);
        fs << "}"
        << "localWebNotificationsConfiguration" << "{" ;
        write(fs, x.localWebConfiguration); fs << "}"
        << "outputPreviewConfiguration" << "{";
        write(fs, x.outputConfiguration); fs << "}"
        << "cameraConfiguration" << "{";
        write_mine(fs, String(), x.camerasConfiguration); fs << "}";
    }

    static void read(const FileNode& node, Observer::Configuration& x, const Observer::Configuration& default_value = Observer::Configuration()){
        if(node.empty())
            x = default_value;
        else {
            node["mediaFolderPath"] >> x.mediaFolderPath;
            node["notificationTextTemplate"] >> x.notificationTextTemplate;
            node["telegramNotificationsConfiguration"] >> x.telegramConfiguration;
            node["localWebNotificationsConfiguration"] >> x.localWebConfiguration;
            node["outputPreviewConfiguration"] >> x.outputConfiguration;
            node["cameraConfiguration"] >> x.camerasConfiguration;
        }
    }

    // OutputPreviewConfiguration
    static void write(FileStorage& fs, const std::string&, const Observer::OutputPreviewConfiguration& x){
        fs  << "showOutput" << x.showOutput
            << "resolution"
                    << "{"
                    << "width" << x.resolution.width
                    << "height" << x.resolution.height
                    << "}"
            << "scaleFactor" << x.scaleFactor
            << "showIgnoredAreas" << x.showIgnoredAreas
            << "showProcessedFrames" << x.showProcessedFrames;
    }

    static void read(const FileNode& node,
                     Observer::OutputPreviewConfiguration& x,
                     const Observer::OutputPreviewConfiguration& default_value = Observer::OutputPreviewConfiguration()){
        if(node.empty())
            x = default_value;
        else {
            node["showOutput"] >> x.showOutput;
            node["resolution"]["width"] >> x.resolution.width;
            node["resolution"]["height"] >> x.resolution.height;
            node["scaleFactor"] >> x.scaleFactor;
            node["showIgnoredAreas"] >> x.showIgnoredAreas;
            node["showProcessedFrames"] >> x.showProcessedFrames;
        }
    }

    // NotificationsServiceConfiguration
    static void write(FileStorage& fs, const std::string&, const Observer::NotificationsServiceConfiguration& x){
        fs << "enabled" << x.enabled
        << "secondsBetweenTextNotification" << x.secondsBetweenTextNotification
        << "secondsBetweenImageNotification" << x.secondsBetweenImageNotification
        << "secondsBetweenVideoNotification" << x.secondsBetweenVideoNotification
        << "noticationsToSend" << x.noticationsToSend
        << "onNotifSendExtraImageNotfWithAllTheCameras" << x.onNotifSendExtraImageNotfWithAllTheCameras
        << "drawTraceOfChangeOn" << x.drawTraceOfChangeOn;
    }

    static void read(const FileNode& node,
                     Observer::NotificationsServiceConfiguration& x,
                     const Observer::NotificationsServiceConfiguration& default_value = Observer::NotificationsServiceConfiguration()){
        if(node.empty())
            x = default_value;
        else {
            node["enabled"] >> x.enabled;
            node["secondsBetweenTextNotification"] >> x.secondsBetweenTextNotification;
            node["secondsBetweenImageNotification"] >> x.secondsBetweenImageNotification;
            node["secondsBetweenVideoNotification"] >> x.secondsBetweenVideoNotification;
            node["noticationsToSend"] >> x.noticationsToSend;
            node["onNotifSendExtraImageNotfWithAllTheCameras"] >> x.onNotifSendExtraImageNotfWithAllTheCameras;
            node["drawTraceOfChangeOn"] >> x.drawTraceOfChangeOn;
        }
    }

    // TelegramNotificationsConfiguration
    static void write(FileStorage& fs, const std::string&n, const Observer::TelegramNotificationsConfiguration& x){
        fs
        << "apiKey" << x.apiKey
        << "chatID" << x.chatID;
        write(fs, n, static_cast<Observer::NotificationsServiceConfiguration>(x));
    }

    static void read(const FileNode& node,
                     Observer::TelegramNotificationsConfiguration& x,
                     const Observer::TelegramNotificationsConfiguration& default_value = Observer::TelegramNotificationsConfiguration()){
        if(node.empty())
            x = default_value;
        else {
            node >> *dynamic_cast<Observer::NotificationsServiceConfiguration*>(&x);
            node["apiKey"] >> x.apiKey;
            node["chatID"] >> x.chatID;
        }
    }

    // LocalWebNotificationsConfiguration
    static void write(FileStorage& fs, const std::string& n, const Observer::LocalWebNotificationsConfiguration& x){
        fs  << "webServerUrl" << x.webServerUrl;
        write(fs, n, static_cast<Observer::NotificationsServiceConfiguration>(x));
    }

    static void read(const FileNode& node,
                     Observer::LocalWebNotificationsConfiguration& x,
                     const Observer::LocalWebNotificationsConfiguration& default_value = Observer::LocalWebNotificationsConfiguration()){
        if(node.empty())
            x = default_value;
        else {
            node >> *dynamic_cast<Observer::NotificationsServiceConfiguration*>(&x);
            node["webServerUrl"] >> x.webServerUrl;
        }
    }



    // Rect
    static void write(FileStorage& fs, const std::string&, const cv::Rect& x){
        fs  << "{"
            << "x" << x.x
            << "y" << x.y
            << "width" << x.width
            << "height" << x.height
            << "}";
    }

    static void read(const FileNode& node,
                     cv::Rect& x,
                     const cv::Rect& default_value = cv::Rect()){
        if(node.empty())
            x = default_value;
        else {
            node["x"] >> x.x;
            node["y"] >> x.y;
            node["width"] >> x.width;
            node["height"] >> x.height;
        }
    }

    // RestrictedArea
    static void write(FileStorage& fs, const std::string&, const Observer::RestrictedArea& x){
        fs  << "{"
            << "points" << x.points
            << "type" << x.type
            << "}";
    }

    static void read(const FileNode& node,
                     Observer::RestrictedArea& x,
                     const Observer::RestrictedArea& default_value = Observer::RestrictedArea()){
        if(node.empty())
            x = default_value;
        else {
            read_mine(node["points"], x.points);
            node["type"] >> x.type;
        }
    }

    // CameraConfiguration
    static void write(FileStorage& fs, const std::string&, const Observer::CameraConfiguration& x){
        fs
        << "name" << x.name
        << "url" << x.url
        << "fps" << x.fps
        << "roi" << x.roi
        << "positionOnOutput" << x.positionOnOutput
        << "rotation" << x.rotation
        << "type" << x.type
        << "noiseThreshold" << x.noiseThreshold
        << "minimumChangeThreshold" << x.minimumChangeThreshold
        << "increaseThresholdFactor" << x.increaseThresholdFactor
        << "secondsBetweenTresholdUpdate" << x.secondsBetweenTresholdUpdate
        << "saveDetectedChangeInVideo" << x.saveDetectedChangeInVideo
        << "ignoredAreas" << "[";
        write_mine(fs, String(), x.ignoredAreas);
        fs << "]"
        << "videoValidatorBufferSize" << x.videoValidatorBufferSize
        << "restrictedAreas" << "[";
        write_mine(fs, String(), x.restrictedAreas);
        fs
        << "]"
        << "objectDetectionMethod" << x.objectDetectionMethod;
    }

    static void read(const FileNode& node,
                     Observer::CameraConfiguration& x,
                     const Observer::CameraConfiguration& default_value = Observer::CameraConfiguration()){
        if(node.empty())
            x = default_value;
        else {
            node["name"] >> x.name;
            node["url"] >> x.url;
            node["fps"] >> x.fps;
            node["roi"] >> x.roi;
            node["positionOnOutput"] >> x.positionOnOutput;
            node["rotation"] >> x.rotation;
            node["type"] >> x.type;
            node["noiseThreshold"] >> x.noiseThreshold;
            node["minimumChangeThreshold"] >> x.minimumChangeThreshold;
            node["increaseThresholdFactor"] >> x.increaseThresholdFactor;
            node["secondsBetweenTresholdUpdate"] >> x.secondsBetweenTresholdUpdate;
            node["saveDetectedChangeInVideo"] >> x.saveDetectedChangeInVideo;

            // write/read methods doesn't work for vector since
            // internally it just don't care and use some default
            // implementation that is not suitable for our data
            // types, so we need to explicitly call read on it.
            //            node["ignoredAreas"] >> x.ignoredAreas;
            // oh, wait, we can't use "read" since it does the same
            // thing but in a different order......... opencv.
            read_mine(node["ignoredAreas"], x.ignoredAreas);
            node["videoValidatorBufferSize"] >> x.videoValidatorBufferSize;
//            node["restrictedAreas"] >> x.restrictedAreas;
            read_mine(node["restrictedAreas"], x.restrictedAreas);
            node["objectDetectionMethod"] >> x.objectDetectionMethod;
        }
    }

    // Point
    static void write(FileStorage& fs, const std::string&, const cv::Point& x){
        fs  << "{"
            << "x" << x.x
            << "y" << x.y
            << "}";
    }

    static void read(const FileNode& node,
                     cv::Point& x,
                     const cv::Point& default_value = cv::Point()){
        if(node.empty())
            x = default_value;
        else {
            node["x"] >> x.x;
            node["y"] >> x.y;
        }
    }
}

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

    // opencv - TODO: Delete opencv parser!
    Configuration ParseYAML(cv::FileStorage& fs);
    void EmmitYAML(cv::FileStorage& fs, const Configuration& cfg);

    // yamlcpp
    Configuration ParseYAML(YAML::Node& node);
    void EmmitYAML(std::ofstream& fs, const Configuration& cfg);
    
    // TODO: example on main
    std::string ConfigurationToJson(const Configuration& cfg);
    
    // TODO: Just read the json file, yamlcpp can parse it
    Configuration JsonToConfiguration(YAML::Node& node);
}
