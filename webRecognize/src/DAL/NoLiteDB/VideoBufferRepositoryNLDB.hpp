#pragma once

#include <stdexcept>

#include "nldb/Collection.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Query/Query.hpp"
#include "nldb/SQL3Implementation.hpp"
#include "nldb/backends/sqlite3/DB/DB.hpp"
#include "nldb/nldb_json.hpp"

namespace Web::DAL {
    class VideoBufferRepositoryNLDB {
       public:
        VideoBufferRepositoryNLDB(nldb::DBSL3* pDb);

       public:
        bool Exists(const std::string& id);

        /**
         * @brief Return the video buffer stored with
         * "id", "duration", "camera_id" "state", "fps", "blobs", "contours"
         * depending on its state
         *
         * @param id
         * @return nldb::json
         */
        nldb::json Get(const std::string& id);

        /**
         * @brief Get path of the camera frames buffer
         *
         * @param id buffer id
         * @return std::optional<std::string> nullopt if the current buffer
         * still has finished processing or the path otherwise.
         * @throws std::runtime on not found
         */
        std::optional<std::string> GetRawBufferPath(const std::string& id);

        /**
         * @brief Get path of the diff frames buffer
         *
         * @param id
         * @return std::optional<std::string> std::optional<std::string> nullopt
         * if the current buffer still has finished processing or the path
         * otherwise.
         * @throws std::runtime on not found
         */
        std::optional<std::string> GetDiffBufferPath(const std::string& id);

        /**
         * @brief Get the all the buffer of a camera
         *
         * @param cameraID
         * @return nldb::json
         */
        nldb::json GetAll(const std::string& cameraID);

        nldb::json GetAll();

        std::string Add(const nldb::json& videoBuffer);

        void Update(const std::string& id, const nldb::json& updateVideoBuffer);

        /**
         * @brief Get the a buffer with all its members, should not be sent to
         * the client because it exposes the server paths
         *
         * @param id
         * @return std::optional<nldb::json>
         */
        std::optional<nldb::json> GetInternal(const std::string& id);

        void Remove(const std::string& id);

       private:
        nldb::DBSL3* db;
        nldb::Query<nldb::DBSL3> query;
        nldb::Collection colVideoBuffer;
    };
}  // namespace Web::DAL