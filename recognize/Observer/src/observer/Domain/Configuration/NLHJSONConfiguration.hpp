#pragma once

#include <exception>

#include "Configuration.hpp"
#include "ParsingExceptions.hpp"
#include "magic_enum.hpp"
#include "nlohmann/json.hpp"
#include "observer/Domain/Configuration/CameraConfiguration.hpp"
#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/Configuration/NotificationsServiceConfiguration.hpp"
#include "observer/Domain/Configuration/OutputPreviewConfiguration.hpp"
#include "observer/RecoJson.hpp"
#include "observer/Utils/SpecialEnums.hpp"
#include "observer/Utils/SpecialStrings.hpp"

#define ExistsInVector(svector, find_this)                          \
    std::find(std::begin(svector), std::end(svector), find_this) != \
        std::end(svector)

namespace Observer {

    /* ------------------------ SIZE ------------------------ */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Size, width, height);

    /* ------------------------ RECT ------------------------ */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Rect, x, y, width, height);

    /* ------------------------ POINT ----------------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Point, x, y);

    /* --------------- EObjectDetectionMethod --------------- */
    NLOHMANN_JSON_SERIALIZE_ENUM(EObjectDetectionMethod,
                                 {
                                     {NONE, "None"},
                                     {HOG_DESCRIPTOR, "Hog Descriptor"},
                                     {YOLODNN_V4, "Yolo DNN V4"},
                                 });

    /* ------------------ ERestrictionType ------------------ */
    NLOHMANN_JSON_SERIALIZE_ENUM(ERestrictionType,
                                 {{ALLOW, "Allow"}, {DENY, "Deny"}});

    /* --------------------- ECameraType -------------------- */
    NLOHMANN_JSON_SERIALIZE_ENUM(ECameraType,
                                 {{DISABLED, "Disabled"},
                                  {NOTIFICATOR, "Notificator"},
                                  {OBJECT_DETECTOR, "Object Detector"},
                                  {VIEW, "View"}});

    /* ------------------- RestrictedArea ------------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RestrictedArea, points, type);

    /* ----------------- BlobDetectorParams ----------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BlobDetectorParams, distance_thresh,
                                       similarity_threshold, blob_max_life);

    /* ---------- BlobFilters::BlobVelocityFilters ---------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BlobFilters::BlobVelocityFilters,
                                       UseVelocityFilter, MinVelocity,
                                       MaxVelocity);

    /* --------------------- BlobFilters -------------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BlobFilters, MinimumOccurrences,
                                       MinimumUnitsTraveled, VelocityFilter);

    /* ------------ ContoursFilter::IgnoredAreas ------------ */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ContoursFilter::IgnoredAreas, areas,
                                       minAreaPercentageToIgnore, reference);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ContoursFilter::IgnoredSets, sets,
                                       reference, minPercentageToIgnore);

    /* --------------------- ContoursFilter -------------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ContoursFilter, FilterByAverageArea,
                                       MinimumArea, ignoredAreas, ignoredSets);

    /* ----------- ThresholdingParams::ResizeParam ---------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ThresholdingParams::ResizeParam, size,
                                       resize);

    /* ----------------- ThresholdingParams ----------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ThresholdingParams,
                                       FramesBetweenDiffFrames, ContextFrames,
                                       MedianBlurKernelSize,
                                       GaussianBlurKernelSize, DilationSize,
                                       BrightnessAboveThreshold, Resize);

    /* ------------- BlobDetectionConfiguration ------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BlobDetectionConfiguration,
                                       blobDetectorParams, blobFilters,
                                       contoursFilters, thresholdingParams);

    /* --------------- ProcessingConfiguration -------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProcessingConfiguration, resize,
                                       noiseThreshold, roi);

    /* ----------------- CameraConfiguration ---------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        CameraConfiguration, name, url, resizeTo, fps, positionOnOutput,
        rotation, type, minimumChangeThreshold, increaseThresholdFactor,
        secondsBetweenThresholdUpdate, saveDetectedChangeInVideo, ignoredAreas,
        videoValidatorBufferSize, restrictedAreas, objectDetectionMethod,
        processingConfiguration, blobDetection);

    /* ------------- OutputPreviewConfiguration ------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OutputPreviewConfiguration, showOutput,
                                       resolution, scaleFactor,
                                       showIgnoredAreas, showProcessedFrames);

    /* ------------------ ENotificationType ----------------- */
    inline void to_json(json& j, const ENotificationType& p) {
        std::vector<std::string> out;

        if (has_flag(p, ENotificationType::TEXT)) {
            out.emplace_back("Text");
        }

        if (has_flag(p, ENotificationType::IMAGE)) {
            out.emplace_back("Image");
        }

        if (has_flag(p, ENotificationType::VIDEO)) {
            out.emplace_back("Video");
        }

        j = out;
    }

    inline void from_json(const json& j, ENotificationType& p) {
        if (!j.is_array()) {
            throw ConfigurationParser::WrongType(j.type_name(), "array",
                                                 j.dump());
        }

        p = Observer::ENotificationType::NONE;

        std::vector<std::string> types;

        try {
            types = j.get<std::vector<std::string>>();
        } catch (const std::exception& e) {
            throw ConfigurationParser::WrongType("N/A", "string", j.dump());
        }

        for (auto& val : types) {
            Observer::StringUtility::StringToLower(val);
        }

        if (ExistsInVector(types, "text")) {
            p |= ENotificationType::TEXT;
        }

        if (ExistsInVector(types, "image")) {
            p |= ENotificationType::IMAGE;
        }

        if (ExistsInVector(types, "video")) {
            p |= ENotificationType::VIDEO;
        }
    }

    /* ---------------------- ETrazable --------------------- */
    inline void to_json(json& j, const ETrazable& p) {
        std::vector<std::string> out;

        if (has_flag(p, ETrazable::IMAGE)) {
            out.emplace_back("Image");
        }

        if (has_flag(p, ETrazable::VIDEO)) {
            out.emplace_back("Video");
        }

        j = out;
    }

    inline void from_json(const json& j, ETrazable& p) {
        if (!j.is_array()) {
            throw ConfigurationParser::WrongType(j.type_name(), "array",
                                                 j.dump());
        }

        p = Observer::ETrazable::NONE;

        std::vector<std::string> types;

        try {
            types = j.get<std::vector<std::string>>();
        } catch (const std::exception& e) {
            throw ConfigurationParser::WrongType("N/A", "string", j.dump());
        }

        for (auto& val : types) {
            Observer::StringUtility::StringToLower(val);
        }

        if (ExistsInVector(types, "image")) {
            set_flag(p, ETrazable::IMAGE);
        }

        if (ExistsInVector(types, "video")) {
            set_flag(p, ETrazable::VIDEO);
        }
    }

    /* ---------- NotificationsServiceConfiguration --------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        NotificationsServiceConfiguration, enabled,
        secondsBetweenTextNotification, secondsBetweenImageNotification,
        secondsBetweenVideoNotification, notificationsToSend,
        onNotifSendExtraImageNotfWithAllTheCameras, drawTraceOfChangeOn);

    /* ---------- LocalWebNotificationsConfiguration --------- */
    inline void to_json(json& j, const LocalWebNotificationsConfiguration& p) {
        to_json(j,
                reinterpret_cast<const NotificationsServiceConfiguration&>(p));
        j["webServerUrl"] = p.webServerUrl;
    }

    inline void from_json(const json& j,
                          LocalWebNotificationsConfiguration& p) {
        j.at("webServerUrl").get_to(p.webServerUrl);
        from_json(j, dynamic_cast<NotificationsServiceConfiguration&>(p));
    }

    /* ---------- TelegramNotificationsConfiguration --------- */
    inline void to_json(json& j, const TelegramNotificationsConfiguration& p) {
        to_json(j,
                reinterpret_cast<const NotificationsServiceConfiguration&>(p));
        j["apiKey"] = p.apiKey;
        j["chatID"] = p.chatID;
    }

    inline void from_json(const json& j,
                          TelegramNotificationsConfiguration& p) {
        j.at("apiKey").get_to(p.apiKey);
        j.at("chatID").get_to(p.chatID);
        from_json(j, reinterpret_cast<NotificationsServiceConfiguration&>(p));
    }

    /* ---------- Configuration::ResizeNotification --------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Configuration::ResizeNotification, image,
                                       video);

    /* -------------------- CONFIGURATION ------------------- */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Configuration, name, mediaFolderPath,
                                       notificationTextTemplate,
                                       resizeNotifications,
                                       telegramConfiguration,
                                       localWebConfiguration,
                                       outputConfiguration, cameras);
}  // namespace Observer