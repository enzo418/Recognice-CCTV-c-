#include "ConfigurationDAO.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace Web::DAL {

    ConfigurationDAO::ConfigurationDAO(nldb::DBSL3* pDb)
        : db(pDb),
          query(db),
          colConfiguration("configurations"),
          colCamera("cameras") {}

    /* ----------- HELPERS TO REPLACE _id WITH id ----------- */
    // could i just modify the internal id to "id"? Yes
    void inline __ReplaceIdKeyString(nldb::json& js) {
        js["id"] = js["_id"];

        js.erase("_id");
    }

    void inline ReplaceIdKeyString(nldb::json& js) {
        if (js.is_array()) {
            for (auto& n : js) {
                __ReplaceIdKeyString(n);
            }
        } else {
            __ReplaceIdKeyString(js);
        }
    }

    void ConfigurationDAO::AddCamerasToConfiguration(nldb::json& cfg) {
        if (cfg.contains("cameras") && cfg["cameras"].size() > 0) {
            // Convert the id array from configuration to an array of
            // cameras:
            auto& json_cameras = cfg["cameras"];

            auto condition = colCamera["_id"] == (std::string)json_cameras[0];

            for (int i = 1; i < json_cameras.size(); i++) {
                condition = condition ||
                            colCamera["_id"] == (std::string)json_cameras[i];
            }

            nldb::json cameras =
                query.from(colCamera).select().where(condition).execute();

            ReplaceIdKeyString(cameras);

            cfg["cameras"] = cameras;
        }
    }

    nldb::json ConfigurationDAO::GetConfiguration(const std::string& id) {
        nldb::json configurationsFound =
            query.from(colConfiguration)
                .select()
                .where(colConfiguration["_id"] == id)
                .execute();

        if (configurationsFound.size() == 1) {
            nldb::json& cfg = configurationsFound[0];
            AddCamerasToConfiguration(cfg);

            ReplaceIdKeyString(cfg);

            return cfg;
        }

        throw std::runtime_error("Configuration not found");
    }

    nldb::json ConfigurationDAO::GetCamera(const std::string& id) {
        nldb::json res = this->query.from(colCamera)
                             .select()
                             .where(colCamera["_id"] == id)
                             .execute();

        if (res.size() == 1) {
            nldb::json& camera = res[0];

            ReplaceIdKeyString(camera);

            return camera;
        }

        throw std::runtime_error("Camera not found");
    }

    std::string ConfigurationDAO::AddCameraToConfiguration(
        const std::string& configurationID,
        const nldb::json& cameraConfiguration) {
        // first check that the configuration exists
        nldb::json result =
            query.from(colConfiguration)
                .select(colConfiguration["cameras"])
                .where(colConfiguration["_id"] == configurationID)
                .execute();

        if (result.empty()) throw std::runtime_error("configuration not found");

        nldb::json& cfg = result[0];

        // insert the camera
        std::vector<std::string> ids =
            query.from(colCamera).insert(cameraConfiguration);

        std::string& cameraID = ids[0];

        // add the camera id to the configuration cameras
        cfg["cameras"].push_back(cameraID);

        // push the modified list to the database
        query.from(colConfiguration)
            .update(configurationID, {{"cameras", cfg["cameras"]}});

        return cameraID;
    }

    void ConfigurationDAO::DeleteCameraFromConfiguration(
        const std::string& configurationID, const std::string& cameraID) {
        // first check that the configuration exists
        // also, get its camera list
        nldb::json result =
            query.from(colConfiguration)
                .select(colConfiguration["cameras"])
                .where(colConfiguration["_id"] == configurationID)
                .execute();

        if (result.empty()) throw std::runtime_error("configuration not found");

        nldb::json& cfg = result[0];
        std::vector<std::string> cameras =
            cfg["cameras"].get<std::vector<std::string>>();

        // remove the camera id from the array
        const auto it = std::find(cameras.begin(), cameras.end(), cameraID);
        if (it != cameras.end()) {
            cameras.erase(it);

            cfg["cameras"] = cameras;

            // it will only update "cameras"
            query.from(colConfiguration).update(configurationID, cfg);
        }
    }

    nldb::json ConfigurationDAO::FindCamera(const std::string& configuration_id,
                                            const std::string& cameraName) {
        nldb::json configurationsFound =
            query.from(colConfiguration)
                .select(colConfiguration["cameras"])
                .where(colConfiguration["_id"] == configuration_id)
                .execute();

        if (configurationsFound.size() < 1) {
            throw std::runtime_error("configuration not found");
        }

        nldb::json& cfg = configurationsFound[0];

        if (!cfg.contains("cameras") || cfg["cameras"].size() == 0) {
            throw std::runtime_error("Configuration doesn't have any camera");
        }

        // we got the camera ids from this configuration, now for the camera
        // that has that name

        nldb::json& json_cameras = configurationsFound[0]["cameras"];

        auto condition = colCamera["_id"] == (std::string)json_cameras[0];

        for (int i = 1; i < json_cameras.size(); i++) {
            condition =
                condition || colCamera["_id"] == (std::string)json_cameras[i];
        }

        nldb::json cameras =
            query.from(colCamera)
                .select()
                .where(condition && colCamera["name"] == cameraName)
                .execute();

        if (cameras.empty()) {
            throw std::runtime_error("Camera not found");
        }

        nldb::json& camera = cameras[0];

        ReplaceIdKeyString(camera);

        return camera;
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

    void ConfigurationDAO::DeleteConfiguration(const std::string& id) {
        query.from(colConfiguration).remove(id);
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
        auto result =
            query.from(colConfiguration)
                .select(colConfiguration["_id"], colConfiguration["name"])
                .execute();

        ReplaceIdKeyString(result);

        return result;
    }
}  // namespace Web::DAL