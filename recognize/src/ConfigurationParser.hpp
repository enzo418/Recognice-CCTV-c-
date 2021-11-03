#pragma once

#include "Configuration.hpp"
#include "LocalWebNotifications.hpp"
#include <yaml-cpp/yaml.h>

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
            rhs.mediaFolderPath = node["mediaFolderPath"].as<std::string>();
            rhs.notificationTextTemplate = node["notificationTextTemplate"].as<std::string>();
            rhs.localWebConfiguration = node["localWebConfiguration"].as<Observer::LocalWebNotificationsConfiguration>();
            rhs.telegramConfiguration = node["telegramConfiguration"].as<Observer::TelegramNotificationsConfiguration>();
            rhs.outputConfiguration = node["outputConfiguration"].as<Observer::OutputPreviewConfiguration>();
            rhs.camerasConfiguration = node["camerasConfiguration"].as<std::vector<Observer::CameraConfiguration>>();
            return true;
        }
    };

    void EncodeNotificationsServiceConfiguration(Node& node,
                                                 Observer::NotificationsServiceConfiguration& cfg) {
        node["enabled"] = cfg.enabled;
        node["secondsBetweenTextNotification"] = cfg.secondsBetweenTextNotification;
        node["secondsBetweenImageNotification"] = cfg.secondsBetweenImageNotification;
        node["secondsBetweenVideoNotification"] = cfg.secondsBetweenVideoNotification;
        node["noticationsToSend"] = cfg.noticationsToSend;
        node["onNotifSendExtraImageNotfWithAllTheCameras"] = cfg.onNotifSendExtraImageNotfWithAllTheCameras;
        node["drawTraceOfChangeOn"] = cfg.drawTraceOfChangeOn;
    }

    template<>
    struct convert<Observer::LocalWebNotificationsConfiguration> {
        static Node encode(Observer::LocalWebNotificationsConfiguration& rhs) {
            Node node;

            node["webServerUrl"] = rhs.webServerUrl;
            EncodeNotificationsServiceConfiguration(node,
                                                    dynamic_cast<Observer::NotificationsServiceConfiguration &>(rhs));
            return node;
        }

        static bool decode(const Node& node, Observer::LocalWebNotificationsConfiguration& rhs) {
            rhs.webServerUrl = node["webServerUrl"].as<std::string>();
            // DecodeNot...
            return true;
        }
    };
}

namespace Observer::ConfigurationParser {
    // opencv
    Configuration ParseYAML(cv::FileStorage& fs);
    void EmmitYAML(cv::FileStorage& fs, const Configuration& cfg);

    // yamlcpp
    Configuration ParseYAML(YAML::Node& node);
    void EmmitYAML(std::ofstream& fs, const Configuration& cfg);
}
