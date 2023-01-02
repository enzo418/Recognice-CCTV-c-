#include "VideoBufferRepositoryNLDB.hpp"

#include <optional>

namespace Web::DAL {
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

    VideoBufferRepositoryNLDB::VideoBufferRepositoryNLDB(nldb::DBSL3* pDb)
        : db(pDb), query(db), colVideoBuffer("videoBuffer") {}

    bool VideoBufferRepositoryNLDB::Exists(const std::string& id) {
        return query.from(colVideoBuffer)
                   .select()
                   .where(colVideoBuffer["_id"] == id)
                   .execute()
                   .size() > 0;
    }

    nldb::json VideoBufferRepositoryNLDB::Get(const std::string& id) {
        auto result =
            query.from(colVideoBuffer)
                .select(colVideoBuffer["_id"], colVideoBuffer["camera_id"],
                        colVideoBuffer["duration"], colVideoBuffer["contours"],
                        colVideoBuffer["blobs"], colVideoBuffer["fps"],
                        colVideoBuffer["state"])
                .where(colVideoBuffer["_id"] == id)
                .execute();

        if (result.size() < 1) {
            throw std::runtime_error("video buffer not found");
        }

        ReplaceIdKeyString(result);

        return result[0];
    }

    nldb::json VideoBufferRepositoryNLDB::GetAll() {
        auto res =
            query.from(colVideoBuffer)
                .select(colVideoBuffer["_id"], colVideoBuffer["date_unix"],
                        colVideoBuffer["camera_id"], colVideoBuffer["duration"],
                        colVideoBuffer["contours"], colVideoBuffer["blobs"],
                        colVideoBuffer["fps"], colVideoBuffer["state"])
                .execute();

        ReplaceIdKeyString(res);

        return res;
    }

    nldb::json VideoBufferRepositoryNLDB::GetAll(const std::string& cameraID) {
        auto res =
            query.from(colVideoBuffer)
                .select(colVideoBuffer["_id"], colVideoBuffer["date_unix"],
                        colVideoBuffer["camera_id"], colVideoBuffer["duration"],
                        colVideoBuffer["contours"], colVideoBuffer["blobs"],
                        colVideoBuffer["fps"], colVideoBuffer["state"])
                .where(colVideoBuffer["camera_id"] == cameraID)
                .execute();

        ReplaceIdKeyString(res);

        return res;
    }

    std::string VideoBufferRepositoryNLDB::Add(const nldb::json& videoBuffer) {
        auto ids = query.from(colVideoBuffer).insert(videoBuffer);
        return ids[0];
    }

    void VideoBufferRepositoryNLDB::Update(
        const std::string& id, const nldb::json& updateVideoBuffer) {
        query.from(colVideoBuffer).update(id, updateVideoBuffer);
    }

    std::optional<std::string> VideoBufferRepositoryNLDB::GetRawBufferPath(
        const std::string& id) {
        auto result =
            query.from(colVideoBuffer)
                .select(colVideoBuffer["path"], colVideoBuffer["state"])
                .where(colVideoBuffer["_id"] == id)
                .execute();

        if (result.empty()) {
            throw std::runtime_error("video buffer not found");
        }

        auto& buffer = result[0];

        if (buffer.contains("path")) {
            return buffer["path"];
        }

        return std::nullopt;
    }

    std::optional<std::string> VideoBufferRepositoryNLDB::GetDiffBufferPath(
        const std::string& id) {
        auto result = query.from(colVideoBuffer)
                          .select(colVideoBuffer["diffFramesPath"],
                                  colVideoBuffer["state"])
                          .where(colVideoBuffer["_id"] == id)
                          .execute();

        if (result.empty()) {
            throw std::runtime_error("video buffer not found");
        }

        auto& buffer = result[0];

        if (buffer.contains("diffFramesPath")) {
            return buffer["diffFramesPath"];
        }

        return std::nullopt;
    }

    std::optional<nldb::json> VideoBufferRepositoryNLDB::GetInternal(
        const std::string& id) {
        auto result = query.from(colVideoBuffer)
                          .select(colVideoBuffer["path"],
                                  colVideoBuffer["diffFramesPath"],
                                  colVideoBuffer["state"])
                          .where(colVideoBuffer["_id"] == id)
                          .execute();

        if (!result.empty()) {
            return result[0];
        }

        return std::nullopt;
    }

    void VideoBufferRepositoryNLDB::Remove(const std::string& id) {
        query.from(colVideoBuffer).remove(id);
    }
}  // namespace Web::DAL