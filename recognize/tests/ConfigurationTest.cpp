#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <opencv4/opencv2/core/types.hpp>

#include "../Observer/src/observer/Domain/Configuration/Configuration.hpp"
#include "../Observer/src/observer/Domain/Configuration/ConfigurationParser.hpp"
#include "../Observer/src/observer/Point.hpp"
#include "../Observer/src/observer/Rect.hpp"
#include "../Observer/src/observer/Size.hpp"

using namespace Observer;

class ConfigurationTest : public ::testing::Test {
   protected:
    void SetUp() override {
        OutputPreviewConfiguration outputPreview = {
            .showOutput = true,
            .resolution = Size(205, 418),
            .scaleFactor = 1.02,
            .showIgnoredAreas = true,
            .showProcessedFrames = false,
        };

        TelegramNotificationsConfiguration telegramConfiguration = {
            .apiKey = "test_api_key",
            .chatID = "123456789",
        };

        telegramConfiguration.enabled = true;
        telegramConfiguration.drawTraceOfChangeOn =
            ETrazable::IMAGE | ETrazable::VIDEO;
        telegramConfiguration.onNotifSendExtraImageNotfWithAllTheCameras = true;
        telegramConfiguration.secondsBetweenImageNotification = 5.1;
        telegramConfiguration.secondsBetweenTextNotification = 5.1;
        telegramConfiguration.secondsBetweenVideoNotification = 5.1;
        telegramConfiguration.noticationsToSend =
            ENotificationType::VIDEO | ENotificationType::TEXT;

        LocalWebNotificationsConfiguration localWebConfiguration = {
            .webServerUrl = "localhost:9000/test"};

        localWebConfiguration.enabled = true;
        localWebConfiguration.drawTraceOfChangeOn = ETrazable::NONE;
        localWebConfiguration.onNotifSendExtraImageNotfWithAllTheCameras = true;
        localWebConfiguration.secondsBetweenImageNotification = 5.1;
        localWebConfiguration.secondsBetweenTextNotification = 5.1;
        localWebConfiguration.secondsBetweenVideoNotification = 5.1;
        localWebConfiguration.noticationsToSend = ENotificationType::VIDEO |
                                                  ENotificationType::TEXT |
                                                  ENotificationType::IMAGE;

        ThresholdingParams thresholdingParams = {
            .FramesBetweenDiffFrames = 6,
            .ContextFrames = 6,
            .MedianBlurKernelSize = 3,
            .GaussianBlurKernelSize = 7,
            .DilationSize = 2,
            .BrightnessAboveThreshold = 4,
            .Resize = {.size = Size(640, 360), .resize = false}};

        ContoursFilter contoursFilters = {
            .FilterByAverageArea = true,
            .MinimumArea = 15,
        };

        BlobDetectorParams detectorParams = {
            .distance_thresh = 15 * 2,
            .similarity_threshold = 0.5,
            .blob_max_life = 3 * 14,
        };

        BlobFilters filtersBlob = {.MinimumOccurrences =
                                       std::numeric_limits<int>::min()};

        BlobDetectionConfiguration blobConfiguration = {
            .blobDetectorParams = detectorParams,
            .contoursFilters = contoursFilters,
            .thresholdingParams = thresholdingParams};

        CameraConfiguration camera1 = {
            .name = "TestCamera1",
            .url = "rtsp://example.com/media.mp4",
            .resizeTo = Size(1920, 1080),
            .fps = 42,
            .positionOnOutput = 0,
            .rotation = -5,
            .type = ECameraType::NOTIFICATOR,
            .minimumChangeThreshold = 101,
            .increaseThresholdFactor = 1.03,
            .secondsBetweenTresholdUpdate = 5,
            .saveDetectedChangeInVideo = false,
            .ignoredAreas = {Rect(15, 15, 640, 360), Rect(5, 15, 123, 435),
                             Rect(7, 7, 112, 444)},
            .videoValidatorBufferSize = 60,
            .restrictedAreas = {{{Point(10, 10), Point(5, 5)},
                                 ERestrictionType::ALLOW}},
            .objectDetectionMethod = EObjectDetectionMethod::HOG_DESCRIPTOR,
            .blobDetection = blobConfiguration,
            .processingConfiguration = {Size(640, 360), 35,
                                        Rect(10, 10, 640, 360)},
        };

        CameraConfiguration camera2 = {
            .name = "TestCamera2",
            .url = "rtsp://example.com/media.mp4/streamid=1",
            .resizeTo = Size(640, 360),
            .fps = 10,
            .positionOnOutput = 1,
            .rotation = 99,
            .type = ECameraType::OBJECT_DETECTOR,
            .minimumChangeThreshold = 12,
            .increaseThresholdFactor = 5.002,
            .secondsBetweenTresholdUpdate = 7,
            .saveDetectedChangeInVideo = true,
            .ignoredAreas = {Rect(2, 2, 622, 117)},
            .videoValidatorBufferSize = 10,
            .restrictedAreas = {{{Point(10, 10), Point(5, 5)},
                                 ERestrictionType::DENY}},
            .objectDetectionMethod = EObjectDetectionMethod::NONE,
            .processingConfiguration = {Size(640, 360), 15,
                                        Rect(1, 1, 233, 233)}};

        this->config = {.name = "configuration test",
                        .mediaFolderPath = "../web/media",
                        .notificationTextTemplate = "This is a template {N}",
                        .telegramConfiguration = telegramConfiguration,
                        .localWebConfiguration = localWebConfiguration,
                        .outputConfiguration = outputPreview,
                        .camerasConfiguration = {camera1, camera2}};
    }

    // void TearDown() override {}
    Configuration config;
};

void checkConfiguration(Configuration& cfg1, Configuration& cfg2) {
    /**
     * Do not use Assert_EQ since google test implements its
     * own overloads for the == operator. To ensure that it's
     * using our == operator we need to use assert/expect true.
     *
     * All the EXPECT are done like this to know wich is
     * component failing to validate.
     */
    EXPECT_TRUE(cfg1.name == cfg2.name);
    EXPECT_TRUE(cfg1.mediaFolderPath == cfg2.mediaFolderPath);
    EXPECT_TRUE(cfg1.notificationTextTemplate == cfg2.notificationTextTemplate);
    EXPECT_TRUE(cfg1.outputConfiguration == cfg2.outputConfiguration);
    EXPECT_TRUE(cfg1.telegramConfiguration == cfg2.telegramConfiguration);
    EXPECT_TRUE(cfg1.localWebConfiguration == cfg2.localWebConfiguration);
    EXPECT_TRUE(cfg1.camerasConfiguration == cfg2.camerasConfiguration);
    ASSERT_TRUE(cfg1 == cfg2);
}

TEST_F(ConfigurationTest, ShouldEmmitAndParseFromYAML) {
    const std::string file = "test1.yaml";
    ConfigurationParser::EmmitYAML(file, config);

    Configuration readedCfg = ConfigurationParser::ParseYAML(file);

    checkConfiguration(readedCfg, config);
}

TEST_F(ConfigurationTest, ShouldEmmitAndParseFromJson) {
    const std::string file = "test1.json";
    ConfigurationParser::EmmitJSON(file, config);

    Configuration readedCfg = ConfigurationParser::ParseJSON(file);

    checkConfiguration(readedCfg, config);
}

TEST(ConfigurationObjectTest, ShouldSetValueOnNode) {
    const std::string mockCfg =
        "{configuration: {mediaFolderPath: 'test', off: 123}}";

    Observer::ConfigurationParser::Object Obj;
    ASSERT_TRUE(
        Observer::ConfigurationParser::ReadConfigurationObject(mockCfg, Obj));

    // field: configuration -> mediaFolderPath = ../web2/media2
    std::string_view path = "configuration/mediaFolderPath/?to=../web2/media2";

    ASSERT_TRUE(Observer::ConfigurationParser::TrySetConfigurationFieldValue(
        Obj, path));

    auto s = Obj["configuration"]["mediaFolderPath"].as<std::string>();

    ASSERT_TRUE(s == "../web2/media2");
}

TEST(ConfigurationObjectTest, ShouldSetArrayValueOnNode) {
    // same test as above with a complex data type
    const std::string mockCfg =
        "{configuration: {cameras: [{ignoredAreas: []}], off: "
        "123}}";

    Observer::ConfigurationParser::Object Obj;
    ASSERT_TRUE(
        Observer::ConfigurationParser::ReadConfigurationObject(mockCfg, Obj));

    // this time as an example we are refering to a camera field so
    // Object must be a camera configuration object or it will fail
    // field: ignoredAreas = <value>
    std::string_view path =
        "configuration/cameras/0/ignoredAreas/"
        "?to=[{\"x\":23,\"y\":15,\"width\":640,\"height\":360},{\"x\":5,\"y\":"
        "15,\"width\":123,\"height\":435},{\"x\":7,\"y\":7,\"width\":112,"
        "\"height\":2222}]";

    // auto cam1 = Obj["configuration"]["cameras"][0];
    ASSERT_TRUE(Observer::ConfigurationParser::TrySetConfigurationFieldValue(
        Obj, path));

    auto data = Obj["configuration"]["cameras"][0]["ignoredAreas"];

    ASSERT_TRUE(data.IsSequence());
    ASSERT_TRUE(data[0]["x"].as<int>() == 23);
}

// TEST(ConfigurationObjectTest, ShouldGetValueOnNode) {
//     const std::string mockCfg = "{configuration: {cameras: [name: 'test']}}";

//     Observer::ConfigurationParser::Object Obj;
//     ASSERT_TRUE(
//         Observer::ConfigurationParser::ReadConfigurationObject(mockCfg,
//         Obj));

//     std::string_view path = "configuration/cameras/0/name";

//     YAML::Node result;
//     // auto cam1 = Obj["configuration"]["cameras"][0];
//     ASSERT_TRUE(Observer::ConfigurationParser::TryGetConfigurationFieldValue(
//         Obj, path, result));

//     ASSERT_TRUE("test" == result.as<std::string>());
// }

// TODO: Add throw tests