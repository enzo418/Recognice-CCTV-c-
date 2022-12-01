#pragma once

#include "nldb/nldb_json.hpp"
#include "yaml-cpp/node/node.h"

namespace Web {
    nldb::json FromYAML(YAML::Node& node);
    YAML::Node FromNlohmann(nldb::json& node);
}  // namespace Web