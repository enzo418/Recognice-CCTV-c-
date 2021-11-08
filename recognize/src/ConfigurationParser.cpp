#include "ConfigurationParser.hpp"
#include <opencv2/opencv.hpp>

namespace Observer::ConfigurationParser {
    Configuration ParseYAML(const std::string& filePath) {
        YAML::Node node = YAML::LoadFile(filePath);
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

    void EmmitYAML(const std::string& filePath, const Configuration& cfg) {
        std::ofstream fs(filePath);

        YAML::Node out;
        out["configuration"] = cfg;
        fs << out;
        
        fs.close();
    }

    std::string GetConfigurationAsJSON(const Configuration &cfg) {
		YAML::Node node;
		node["configuration"] = cfg;
		YAML::Emitter emitter;
        emitter << YAML::DoubleQuoted << YAML::Flow << YAML::BeginSeq << node;
        return std::string(emitter.c_str() + 1);  // Strip leading [ character      
    }

    void EmmitJSON(const std::string &filePath, const Configuration &cfg) {
        std::ofstream jsonout(filePath);
		auto jsonstr = GetConfigurationAsJSON(cfg);
		jsonout << jsonstr;
		jsonout.close();
	}
	
    Configuration ParseJSON(const std::string& filePath) {
		return ParseYAML(filePath);
	}
}
