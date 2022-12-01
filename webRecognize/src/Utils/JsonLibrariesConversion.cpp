#include "JsonLibrariesConversion.hpp"

#include "nlohmann/json_fwd.hpp"
#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "yaml-cpp/emitter.h"

namespace Web {
    nldb::json FromYAML(YAML::Node& node) {
        // node -> json -> nlohmann
        return nldb::json::parse(
            Observer::ConfigurationParser::NodeAsJson(node));
    }

    YAML::Node FromNlohmann(nldb::json& node) {
        // node -> json -> yamlcpp
        return YAML::Load(node.dump());
    }
}  // namespace Web