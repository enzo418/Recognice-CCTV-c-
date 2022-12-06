#include "ConfigurationDAO.hpp"

namespace Web::DAL {

    ConfigurationDAO::ConfigurationDAO(nldb::DBSL3* pDb)
        : db(pDb),
          query(db),
          colConfiguration("configurations"),
          colCamera("cameras") {}

    void ConfigurationDAO::AddCamerasToConfiguration(nldb::json& cfg) {
        if (cfg.contains("cameras") && cfg["cameras"].size() > 0) {
            // Convert the id array from configuration to an array of
            // cameras:
            auto& json_cameras = cfg["cameras"];

            auto condition = colCamera["id"] == (std::string)json_cameras[0];

            for (int i = 1; i < json_cameras.size(); i++) {
                condition = condition ||
                            colCamera["id"] == (std::string)json_cameras[i];
            }

            nldb::json cameras =
                query.from(colCamera).select().where(condition).execute();

            cfg["cameras"] = cameras;
        }
    }

    nldb::json ConfigurationDAO::Get(const std::string& id) {
        nldb::json configurationsFound =
            query.from(colConfiguration)
                .select()
                .where(colConfiguration["id"] == id)
                .execute();

        if (configurationsFound.size() == 1) {
            nldb::json& cfg = configurationsFound[0];
            AddCamerasToConfiguration(cfg);
            return cfg;
        }

        throw std::runtime_error("Configuration not found");
    }

    nldb::json ConfigurationDAO::GetCamera(const std::string& id) {
        nldb::json res = this->query.from(colCamera)
                             .select()
                             .where(colCamera["id"] == id)
                             .execute();

        if (res.size() == 1) {
            return res[0];
        }

        throw std::runtime_error("Camera not found");
    }

    std::string ConfigurationDAO::InsertConfiguration(
        const nldb::json& pConfig) {
        nldb::json config = pConfig;  // we might modify it

        if (config.contains("cameras") && config["cameras"].size() > 0) {
            std::vector<std::string> newIds =
                query.from("cameras").insert(config["cameras"]);

            config["cameras"] = newIds;
        }

        return query.from(colConfiguration).insert(config)[0];
    }

    void ConfigurationDAO::UpdateConfiguration(const std::string& id,
                                               const nldb::json& data) {
        query.from(colConfiguration).update(id, data);
    }

    void ConfigurationDAO::UpdateCamera(const std::string& id,
                                        const nldb::json& data) {
        query.from(colCamera).update(id, data);
    }

    nldb::json ConfigurationDAO::GetAllNamesAndId() {
        return query.from(colConfiguration)
            .select(colConfiguration["id"], colConfiguration["name"])
            .execute();
    }
}  // namespace Web::DAL