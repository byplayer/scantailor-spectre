// Copyright (C) 2026 ScanTailor Spectre contributors
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#include <QTemporaryDir>

#include <boost/test/unit_test.hpp>

#include <filters/output/Settings.h>

#include "DefaultParams.h"
#include "DefaultParamsProfileManager.h"
#include "DefaultParamsProvider.h"
#include "ImageId.h"
#include "PageId.h"

namespace {
DefaultParams buildNonDefaultParams() {
  DefaultParams::PageBoxParams pageBoxParams(QSizeF(148, 210), MODE_MANUAL, true);

  DefaultParams::SelectContentParams selectContentParams;
  selectContentParams.setContentDetectEnabled(false);
  selectContentParams.setContentFillFactor(0.42);
  selectContentParams.setBorderTolerance(7);

  DefaultParams::FinalizeParams finalizeParams;
  finalizeParams.setAutoColorModePolicy(finalize::AutoColorModePolicy::NeverColor);
  finalizeParams.setAutoWhiteBalanceEnabled(false);
  finalizeParams.setMidtoneThreshold(23);
  finalizeParams.setOutputFormat(finalize::OutputFormat::JPEG);
  finalizeParams.setTiffCompression(finalize::TiffCompression::Deflate);
  finalizeParams.setJpegQuality(72);

  DefaultParams::OutputParams outputParams;
  outputParams.setPassThroughEnabled(true);
  output::ColorParams colorParams;
  output::ColorCommonOptions colorCommonOptions;
  colorCommonOptions.setWienerCoef(0.35);
  colorCommonOptions.setWienerWindowSize(9);
  colorParams.setColorCommonOptions(colorCommonOptions);
  weasel::PhotoAdjustments photoAdjustments;
  photoAdjustments.setTemp(-30);
  photoAdjustments.setExposure(0.25);
  photoAdjustments.setBlacks(15);
  colorParams.setPhotoAdjustments(photoAdjustments);
  outputParams.setColorParams(colorParams);

  DefaultParams::OcrParams ocrParams;
  ocrParams.setOcrEnabled(false);
  ocrParams.setLanguage("ja-JP");
  ocrParams.setAccurateRecognitionEnabled(false);
  ocrParams.setLanguageCorrectionEnabled(false);

  DefaultParams::ExportParams exportParams;
  exportParams.setDpiLimitDisabled(false);
  exportParams.setMaxDpi(250);
  exportParams.setGrayscaleCompressionEnabled(true);
  exportParams.setQuality(PdfExporter::Quality::High);

  return DefaultParams(DefaultParams::FixOrientationParams(), DefaultParams::DeskewParams(),
                       DefaultParams::PageSplitParams(), pageBoxParams, selectContentParams,
                       DefaultParams::PageLayoutParams(), finalizeParams, outputParams, ocrParams, exportParams);
}
}  // namespace

BOOST_AUTO_TEST_SUITE(DefaultParamsProfileTestSuite)

// The "Custom" profile used to live only in memory, so the parameters the user
// confirmed were lost on the next launch. It is now written out like any other
// profile and has to survive the round trip.
BOOST_AUTO_TEST_CASE(every_section_survives_a_profile_round_trip) {
  QTemporaryDir profileDir;
  BOOST_REQUIRE(profileDir.isValid());

  const DefaultParamsProfileManager profileManager(profileDir.path());
  BOOST_REQUIRE(profileManager.writeProfile(buildNonDefaultParams(), "Custom"));

  DefaultParamsProfileManager::LoadStatus status;
  const std::unique_ptr<DefaultParams> loaded = profileManager.readProfile("Custom", &status);
  BOOST_REQUIRE_EQUAL(status, DefaultParamsProfileManager::SUCCESS);
  BOOST_REQUIRE(loaded != nullptr);

  const DefaultParams::PageBoxParams& pageBox = loaded->getPageBoxParams();
  BOOST_CHECK_EQUAL(pageBox.getPageRectSize().width(), 148);
  BOOST_CHECK_EQUAL(pageBox.getPageRectSize().height(), 210);
  BOOST_CHECK_EQUAL(pageBox.getPageDetectMode(), MODE_MANUAL);
  BOOST_CHECK(pageBox.isFineTuneCorners());

  const DefaultParams::SelectContentParams& selectContent = loaded->getSelectContentParams();
  BOOST_CHECK(!selectContent.isContentDetectEnabled());
  BOOST_CHECK_CLOSE(selectContent.getContentFillFactor(), 0.42, 0.0001);
  BOOST_CHECK_EQUAL(selectContent.getBorderTolerance(), 7);

  const DefaultParams::FinalizeParams& finalizeParams = loaded->getFinalizeParams();
  BOOST_CHECK(finalizeParams.getAutoColorModePolicy() == finalize::AutoColorModePolicy::NeverColor);
  BOOST_CHECK(!finalizeParams.isAutoWhiteBalanceEnabled());
  BOOST_CHECK_EQUAL(finalizeParams.getMidtoneThreshold(), 23);
  BOOST_CHECK(finalizeParams.getOutputFormat() == finalize::OutputFormat::JPEG);
  BOOST_CHECK(finalizeParams.getTiffCompression() == finalize::TiffCompression::Deflate);
  BOOST_CHECK_EQUAL(finalizeParams.getJpegQuality(), 72);

  const DefaultParams::OutputParams& outputParams = loaded->getOutputParams();
  BOOST_CHECK(outputParams.isPassThroughEnabled());
  BOOST_CHECK_CLOSE(outputParams.getColorParams().colorCommonOptions().wienerCoef(), 0.35, 0.0001);
  BOOST_CHECK_EQUAL(outputParams.getColorParams().colorCommonOptions().wienerWindowSize(), 9);
  const weasel::PhotoAdjustments& photoAdjustments = outputParams.getColorParams().photoAdjustments();
  BOOST_CHECK_CLOSE(photoAdjustments.temp(), -30.0, 0.0001);
  BOOST_CHECK_CLOSE(photoAdjustments.exposure(), 0.25, 0.0001);
  BOOST_CHECK_CLOSE(photoAdjustments.blacks(), 15.0, 0.0001);

  const DefaultParams::OcrParams& ocrParams = loaded->getOcrParams();
  BOOST_CHECK(!ocrParams.isOcrEnabled());
  BOOST_CHECK_EQUAL(ocrParams.getLanguage().toStdString(), "ja-JP");
  BOOST_CHECK(!ocrParams.isAccurateRecognitionEnabled());
  BOOST_CHECK(!ocrParams.isLanguageCorrectionEnabled());

  const DefaultParams::ExportParams& exportParams = loaded->getExportParams();
  BOOST_CHECK(!exportParams.isDpiLimitDisabled());
  BOOST_CHECK_EQUAL(exportParams.getMaxDpi(), 250);
  BOOST_CHECK(exportParams.isGrayscaleCompressionEnabled());
  BOOST_CHECK(exportParams.getQuality() == PdfExporter::Quality::High);
}

// Every per-property setter in output::Settings used to build a page's first
// Params from scratch, throwing away every default except the one being
// written - so a color mode change wiped the default photo adjustments.
BOOST_AUTO_TEST_CASE(output_settings_seed_pages_from_the_default_parameters) {
  DefaultParamsProvider& provider = DefaultParamsProvider::getInstance();
  const DefaultParams savedParams = provider.getParams();
  const QString savedProfileName = provider.getProfileName();

  {
    weasel::PhotoAdjustments adjustments;
    adjustments.setExposure(0.4);
    adjustments.setContrast(20);
    output::ColorParams colorParams;
    colorParams.setPhotoAdjustments(adjustments);

    DefaultParams::OutputParams outputParams;
    outputParams.setColorParams(colorParams);
    outputParams.setDespeckleLevel(2.5);

    auto defaults = std::make_unique<DefaultParams>();
    defaults->setOutputParams(outputParams);
    provider.setParams(std::move(defaults), "Custom");
  }

  const PageId pageId(ImageId(QStringLiteral("/tmp/seed-page.tif"), 1));
  output::Settings settings;

  // A page nothing has written yet reports the defaults.
  BOOST_CHECK_CLOSE(settings.getParams(pageId).colorParams().photoAdjustments().exposure(), 0.4, 0.0001);

  // Storing the first property must keep the rest of the defaults.
  output::ColorParams changed = settings.getParams(pageId).colorParams();
  changed.setColorMode(output::GRAYSCALE);
  settings.setColorParams(pageId, changed);

  const output::Params stored = settings.getParams(pageId);
  BOOST_CHECK(stored.colorParams().colorMode() == output::GRAYSCALE);
  BOOST_CHECK_CLOSE(stored.colorParams().photoAdjustments().exposure(), 0.4, 0.0001);
  BOOST_CHECK_CLOSE(stored.colorParams().photoAdjustments().contrast(), 20.0, 0.0001);
  BOOST_CHECK_CLOSE(stored.despeckleLevel(), 2.5, 0.0001);

  provider.setParams(std::make_unique<DefaultParams>(savedParams), savedProfileName);
}

BOOST_AUTO_TEST_SUITE_END()
