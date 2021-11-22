#include "../src/Configuration.hpp"
#include "../src/ConfigurationParser.hpp"

#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <opencv4/opencv2/core/types.hpp>

using namespace Observer;

class ConfigurationTest : public ::testing::Test {
 protected:
  void SetUp() override {
     OutputPreviewConfiguration outputPreview = {
      .showOutput = true,
      .resolution = cv::Size(205, 418),
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

  CameraConfiguration camera1 = {
      .name = "TestCamera1",
      .url = "rtsp://example.com/media.mp4",
      .fps = 42,
      .roi = cv::Rect(10, 10, 640, 360),
      .positionOnOutput = 0,
      .rotation = -5,
      .type = ECameraType::NOTIFICATOR,
      .noiseThreshold = 35,
      .minimumChangeThreshold = 101,
      .increaseThresholdFactor = 1.03,
      .secondsBetweenTresholdUpdate = 5,
      .saveDetectedChangeInVideo = false,
      .ignoredAreas = {cv::Rect(15, 15, 640, 360), cv::Rect(5, 15, 123, 435),
                       cv::Rect(7, 7, 112, 444)},
      .videoValidatorBufferSize = 60,
      .restrictedAreas = {{{cv::Point(10, 10), cv::Point(5, 5)},
                           ERestrictionType::ALLOW}},
      .objectDetectionMethod = EObjectDetectionMethod::HOG_DESCRIPTOR,
  };

  CameraConfiguration camera2 = {
      .name = "TestCamera2",
      .url = "rtsp://example.com/media.mp4/streamid=1",
      .fps = 10,
      .roi = cv::Rect(1, 1, 233, 233),
      .positionOnOutput = 1,
      .rotation = 99,
      .type = ECameraType::OBJECT_DETECTOR,
      .noiseThreshold = 15,
      .minimumChangeThreshold = 12,
      .increaseThresholdFactor = 5.002,
      .secondsBetweenTresholdUpdate = 7,
      .saveDetectedChangeInVideo = true,
      .ignoredAreas = {cv::Rect(2, 2, 622, 117)},
      .videoValidatorBufferSize = 10,
      .restrictedAreas = {{{cv::Point(10, 10), cv::Point(5, 5)},
                           ERestrictionType::DENY}},
      .objectDetectionMethod = EObjectDetectionMethod::NONE,
  };

  this->config = {.mediaFolderPath = "../web/media",
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

// TODO: Add throw tests