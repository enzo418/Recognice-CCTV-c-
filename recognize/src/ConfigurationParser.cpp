#include "ConfigurationParser.hpp"
#include <opencv2/opencv.hpp>

namespace Observer::ConfigurationParser {
    Configuration ParseYAML(cv::FileStorage& fs) {
        Configuration cfg;
        fs["configuration"] >> cfg;
        return cfg;
    }

    void EmmitYAML(cv::FileStorage& fs, const Configuration& cfg) {
        fs << "configuration" << "{";
        write(fs, cfg);
        fs << "}";
    }

    Configuration ParseYAML(YAML::Node& node) {
        Configuration cfg;
        
        try {
            cfg = node["configuration"].as<Observer::Configuration>();
        } catch (const YAML::InvalidNode& ex) {
            // Log
            std::cout << "Couldn't parse the configuration file: " << std::endl;

            if (!ex.mark.is_null()) {
                std::cout
                        << "\n\t Line: " << ex.mark.line << "\n\t Pos: " << ex.mark.pos
                        << std::endl;
            } 
            
                std::cout << "\tError: " << ex.msg << std::endl;
            

            auto key = Observer::StringUtility::GetStringBetweenDelimiter(ex.msg, "\"", "\"");

            throw MissingKey(key.c_str());
        } 
/*        catch (const YAML::TypedBadConversion &ex) {
            std::cout << "Couldn't parse the configuration file: " << std::endl;
            
            if (!ex.mark.is_null()) {
                std::cout
                        << "\n\t Line: " << ex.mark.line << "\n\t Pos: " << ex.mark.pos
                        << std::endl;
            }
            
            std::cout << "\tError: " << ex.msg << std::endl;
            
            throw WrongType(ex.mark.line, ex.mark.column, ex.mark.pos);
        } */
        catch (const YAML::ParserException& ex) {
            std::cout << "Couldn't parse the configuration file: " << std::endl;
            
            if (!ex.mark.is_null()) {
                std::cout
                        << "\n\t Line: " << ex.mark.line << "\n\t Pos: " << ex.mark.pos
                        << std::endl;
            }
            
            std::cout << "\tError: " << ex.msg << std::endl;
        } catch (const std::exception& ex) {
            std::cout << "Couldn't parse the configuration file: " << std::endl;
        }
        
        return cfg;
    }

    void EmmitYAML(std::ofstream& fs, const Configuration& cfg) {
        YAML::Node out;
        out["configuration"] = cfg;
        fs << out;
    }
}

namespace YAML {
    void EncodeNotificationsServiceConfiguration(Node& node,
                                                 const Observer::NotificationsServiceConfiguration& cfg) {
        node["enabled"] = cfg.enabled;
        node["secondsBetweenTextNotification"] = cfg.secondsBetweenTextNotification;
        node["secondsBetweenImageNotification"] = cfg.secondsBetweenImageNotification;
        node["secondsBetweenVideoNotification"] = cfg.secondsBetweenVideoNotification;
        node["noticationsToSend"] = cfg.noticationsToSend;
        node["onNotifSendExtraImageNotfWithAllTheCameras"] = cfg.onNotifSendExtraImageNotfWithAllTheCameras;
        node["drawTraceOfChangeOn"] = cfg.drawTraceOfChangeOn;
    }

    void DecodeNotificationsServiceConfiguration(const Node& node,
                                                 Observer::NotificationsServiceConfiguration& cfg) {
        cfg.enabled = node["enabled"].as<bool>();
        cfg.secondsBetweenTextNotification = node["secondsBetweenTextNotification"].as<double>();
        cfg.secondsBetweenImageNotification = node["secondsBetweenImageNotification"].as<double>();
        cfg.secondsBetweenVideoNotification = node["secondsBetweenVideoNotification"].as<double>();
        cfg.noticationsToSend = node["noticationsToSend"].as<Observer::ENotificationType>();
        cfg.onNotifSendExtraImageNotfWithAllTheCameras = node["onNotifSendExtraImageNotfWithAllTheCameras"].as<bool>();
        cfg.drawTraceOfChangeOn = node["drawTraceOfChangeOn"].as<Observer::ETrazable>();
    }
}