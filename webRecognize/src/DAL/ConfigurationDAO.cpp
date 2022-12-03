#include "ConfigurationDAO.hpp"

namespace Web::DAL {

    ConfigurationDAO::ConfigurationDAO(nldb::DBSL3* pDb) : db(pDb), query(db) {}

    void ConfigurationDAO::AddCamerasToConfiguration(nldb::json& cfg) {
        if (cfg.contains("cameras") && cfg["cameras"].size() > 0) {
            // Convert the id array from configuration to an array of
            // cameras:
            auto& json_cameras = cfg["cameras"];

            auto colCameras = query.collection("cameras");

            auto condition = colCameras["_id"] == (std::string)json_cameras[0];

            for (int i = 1; i < json_cameras.size(); i++) {
                condition = condition ||
                            colCameras["_id"] == (std::string)json_cameras[i];
            }

            nldb::json cameras =
                query.from(colCameras).select().where(condition).execute();

            cfg["cameras"] = cameras;
        }
    }

    nldb::json ConfigurationDAO::Get(const std::string& id) {
        auto colCfgs = query.collection("configurations");
        nldb::json configurationsFound =
            query.from(colCfgs).select().where(colCfgs["_id"] == id).execute();

        if (configurationsFound.size() == 1) {
            nldb::json& cfg = configurationsFound[0];
            AddCamerasToConfiguration(cfg);
            return cfg;
        }

        throw std::runtime_error("Configuration not found");
    }

    std::string ConfigurationDAO::InsertConfiguration(
        const nldb::json& pConfig) {
        nldb::json config = pConfig;  // we might modify it

        if (config.contains("cameras") && config["cameras"].size() > 0) {
            std::vector<std::string> newIds =
                query.from("cameras").insert(config["cameras"]);

            config["cameras"] = newIds;
        }

        return query.from("configurations").insert(config)[0];
    }

    void ConfigurationDAO::UpdateConfiguration(const std::string& id,
                                               const nldb::json& data) {
        query.from("configurations").update(id, data);
    }

    void ConfigurationDAO::UpdateCamera(const std::string& id,
                                        const nldb::json& data) {
        query.from("cameras").update(id, data);
    }
}  // namespace Web::DAL