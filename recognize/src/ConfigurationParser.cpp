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

    }

    void EmmitYAML(std::ofstream& fs, const Configuration& cfg) {
//        fs << "configuration" << cfg;
    }
}