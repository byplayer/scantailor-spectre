// Copyright (C) 2019  Joseph Artsimovich <joseph.artsimovich@gmail.com>, 4lex4 <4lex49@zoho.com>
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#include "DefaultParams.h"

#include <foundation/Utils.h>

#include "XmlMarshaller.h"
#include "XmlUnmarshaller.h"

using namespace foundation;
using namespace page_split;
using namespace output;

namespace {
// Keep in sync with select_content::Settings.
constexpr double DEFAULT_CONTENT_FILL_FACTOR = 0.65;
constexpr int DEFAULT_BORDER_TOLERANCE = 2;
}  // namespace

DefaultParams::DefaultParams(const DefaultParams::FixOrientationParams& fixOrientationParams,
                             const DefaultParams::DeskewParams& deskewParams,
                             const DefaultParams::PageSplitParams& pageSplitParams,
                             const DefaultParams::PageBoxParams& pageBoxParams,
                             const DefaultParams::SelectContentParams& selectContentParams,
                             const DefaultParams::PageLayoutParams& pageLayoutParams,
                             const DefaultParams::FinalizeParams& finalizeParams,
                             const DefaultParams::OutputParams& outputParams,
                             const DefaultParams::OcrParams& ocrParams,
                             const DefaultParams::ExportParams& exportParams)
    : m_fixOrientationParams(fixOrientationParams),
      m_deskewParams(deskewParams),
      m_pageSplitParams(pageSplitParams),
      m_pageBoxParams(pageBoxParams),
      m_selectContentParams(selectContentParams),
      m_pageLayoutParams(pageLayoutParams),
      m_finalizeParams(finalizeParams),
      m_outputParams(outputParams),
      m_ocrParams(ocrParams),
      m_exportParams(exportParams),
      m_units(MILLIMETRES) {}

DefaultParams::DefaultParams(const QDomElement& el)
    : m_fixOrientationParams(el.namedItem("fix-orientation-params").toElement()),
      m_deskewParams(el.namedItem("deskew-params").toElement()),
      m_pageSplitParams(el.namedItem("page-split-params").toElement()),
      m_pageBoxParams(el.namedItem("page-box-params").toElement()),
      m_selectContentParams(el.namedItem("select-content-params").toElement()),
      m_pageLayoutParams(el.namedItem("page-layout-params").toElement()),
      m_finalizeParams(el.namedItem("finalize-params").toElement()),
      m_outputParams(el.namedItem("output-params").toElement()),
      m_ocrParams(el.namedItem("ocr-params").toElement()),
      m_exportParams(el.namedItem("export-params").toElement()),
      m_units(unitsFromString(el.attribute("units"))) {}

QDomElement DefaultParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.appendChild(m_fixOrientationParams.toXml(doc, "fix-orientation-params"));
  el.appendChild(m_deskewParams.toXml(doc, "deskew-params"));
  el.appendChild(m_pageSplitParams.toXml(doc, "page-split-params"));
  el.appendChild(m_pageBoxParams.toXml(doc, "page-box-params"));
  el.appendChild(m_selectContentParams.toXml(doc, "select-content-params"));
  el.appendChild(m_pageLayoutParams.toXml(doc, "page-layout-params"));
  el.appendChild(m_finalizeParams.toXml(doc, "finalize-params"));
  el.appendChild(m_outputParams.toXml(doc, "output-params"));
  el.appendChild(m_ocrParams.toXml(doc, "ocr-params"));
  el.appendChild(m_exportParams.toXml(doc, "export-params"));
  el.setAttribute("units", unitsToString(m_units));
  return el;
}

DefaultParams::DefaultParams() : m_units(MILLIMETRES) {}

DefaultParams::FixOrientationParams::FixOrientationParams(const OrthogonalRotation& imageRotation)
    : m_imageRotation(imageRotation) {}

DefaultParams::FixOrientationParams::FixOrientationParams(const QDomElement& el)
    : m_imageRotation(el.namedItem("imageRotation").toElement()) {}

QDomElement DefaultParams::FixOrientationParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.appendChild(m_imageRotation.toXml(doc, "imageRotation"));
  return el;
}

DefaultParams::DeskewParams::DeskewParams(double deskewAngleDeg, AutoManualMode mode)
    : m_deskewAngleDeg(deskewAngleDeg), m_mode(mode) {}

DefaultParams::DeskewParams::DeskewParams() : m_deskewAngleDeg(0.0), m_mode(MODE_AUTO) {}

DefaultParams::DeskewParams::DeskewParams(const QDomElement& el)
    : m_deskewAngleDeg(el.attribute("deskewAngleDeg").toDouble()),
      m_mode((el.attribute("mode") == "manual") ? MODE_MANUAL : MODE_AUTO) {}

QDomElement DefaultParams::DeskewParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.setAttribute("deskewAngleDeg", Utils::doubleToString(m_deskewAngleDeg));
  el.setAttribute("mode", (m_mode == MODE_AUTO) ? "auto" : "manual");
  return el;
}

DefaultParams::PageSplitParams::PageSplitParams(page_split::LayoutType layoutType) : m_layoutType(layoutType) {}

DefaultParams::PageSplitParams::PageSplitParams() : m_layoutType(AUTO_LAYOUT_TYPE) {}

DefaultParams::PageSplitParams::PageSplitParams(const QDomElement& el)
    : m_layoutType(layoutTypeFromString(el.attribute("layoutType"))) {}

QDomElement DefaultParams::PageSplitParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.setAttribute("layoutType", layoutTypeToString(m_layoutType));
  return el;
}

DefaultParams::SelectContentParams::SelectContentParams(const QSizeF& pageRectSize,
                                                        bool contentDetectEnabled,
                                                        AutoManualMode pageDetectMode,
                                                        bool fineTuneCorners)
    : m_pageRectSize(pageRectSize),
      m_contentDetectEnabled(contentDetectEnabled),
      m_pageDetectMode(pageDetectMode),
      m_fineTuneCorners(fineTuneCorners),
      m_contentFillFactor(DEFAULT_CONTENT_FILL_FACTOR),
      m_borderTolerance(DEFAULT_BORDER_TOLERANCE) {}

DefaultParams::SelectContentParams::SelectContentParams()
    : m_pageRectSize(QSizeF(210, 297)),
      m_contentDetectEnabled(true),
      m_pageDetectMode(MODE_DISABLED),
      m_fineTuneCorners(false),
      m_contentFillFactor(DEFAULT_CONTENT_FILL_FACTOR),
      m_borderTolerance(DEFAULT_BORDER_TOLERANCE) {}

DefaultParams::SelectContentParams::SelectContentParams(const QDomElement& el)
    : m_pageRectSize(XmlUnmarshaller::sizeF(el.namedItem("pageRectSize").toElement())),
      m_contentDetectEnabled(el.attribute("contentDetectEnabled") == "1"),
      m_pageDetectMode(stringToAutoManualMode(el.attribute("pageDetectMode"))),
      m_fineTuneCorners(el.attribute("fineTuneCorners") == "1"),
      m_contentFillFactor(
          el.attribute("contentFillFactor", QString::number(DEFAULT_CONTENT_FILL_FACTOR)).toDouble()),
      m_borderTolerance(el.attribute("borderTolerance", QString::number(DEFAULT_BORDER_TOLERANCE)).toInt()) {}

QDomElement DefaultParams::SelectContentParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.appendChild(XmlMarshaller(doc).sizeF(m_pageRectSize, "pageRectSize"));
  el.setAttribute("contentDetectEnabled", m_contentDetectEnabled ? "1" : "0");
  el.setAttribute("pageDetectMode", autoManualModeToString(m_pageDetectMode));
  el.setAttribute("fineTuneCorners", m_fineTuneCorners ? "1" : "0");
  el.setAttribute("contentFillFactor", Utils::doubleToString(m_contentFillFactor));
  el.setAttribute("borderTolerance", m_borderTolerance);
  return el;
}

DefaultParams::PageLayoutParams::PageLayoutParams(const Margins& hardMargins,
                                                  const page_layout::Alignment& alignment,
                                                  bool autoMargins)
    : m_hardMargins(hardMargins), m_alignment(alignment), m_autoMargins(autoMargins) {}

DefaultParams::PageLayoutParams::PageLayoutParams() : m_hardMargins(10, 5, 10, 5), m_autoMargins(true) {}

DefaultParams::PageLayoutParams::PageLayoutParams(const QDomElement& el)
    : m_hardMargins(el.namedItem("hardMargins").toElement()),
      m_alignment(el.namedItem("alignment").toElement()),
      m_autoMargins(el.attribute("autoMargins") == "1") {}

QDomElement DefaultParams::PageLayoutParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.appendChild(m_hardMargins.toXml(doc, "hardMargins"));
  el.appendChild(m_alignment.toXml(doc, "alignment"));
  el.setAttribute("autoMargins", m_autoMargins ? "1" : "0");
  return el;
}

DefaultParams::OutputParams::OutputParams(const Dpi& dpi,
                                          const output::ColorParams& colorParams,
                                          const output::SplittingOptions& splittingOptions,
                                          const output::PictureShapeOptions& pictureShapeOptions,
                                          const output::DepthPerception& depthPerception,
                                          const output::DewarpingOptions& dewarpingOptions,
                                          const double despeckleLevel)
    : m_dpi(dpi),
      m_colorParams(colorParams),
      m_splittingOptions(splittingOptions),
      m_pictureShapeOptions(pictureShapeOptions),
      m_depthPerception(depthPerception),
      m_dewarpingOptions(dewarpingOptions),
      m_despeckleLevel(despeckleLevel),
      m_passThrough(false) {}

DefaultParams::OutputParams::OutputParams() : m_dpi(300, 300), m_despeckleLevel(1.0), m_passThrough(false) {}

DefaultParams::OutputParams::OutputParams(const QDomElement& el)
    : m_dpi(el.namedItem("dpi").toElement()),
      m_colorParams(el.namedItem("colorParams").toElement()),
      m_splittingOptions(el.namedItem("splittingOptions").toElement()),
      m_pictureShapeOptions(el.namedItem("pictureShapeOptions").toElement()),
      m_depthPerception(el.attribute("depthPerception").toDouble()),
      m_dewarpingOptions(el.namedItem("dewarpingOptions").toElement()),
      m_despeckleLevel(el.attribute("despeckleLevel").toDouble()),
      m_passThrough(el.attribute("passThrough", "0") == "1") {}

QDomElement DefaultParams::OutputParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.appendChild(m_dpi.toXml(doc, "dpi"));
  el.appendChild(m_colorParams.toXml(doc, "colorParams"));
  el.appendChild(m_splittingOptions.toXml(doc, "splittingOptions"));
  el.appendChild(m_pictureShapeOptions.toXml(doc, "pictureShapeOptions"));
  el.setAttribute("depthPerception", Utils::doubleToString(m_depthPerception.value()));
  el.appendChild(m_dewarpingOptions.toXml(doc, "dewarpingOptions"));
  el.setAttribute("despeckleLevel", Utils::doubleToString(m_despeckleLevel));
  el.setAttribute("passThrough", m_passThrough ? "1" : "0");
  return el;
}

/*=============================== PageBoxParams ==============================*/

DefaultParams::PageBoxParams::PageBoxParams(const QSizeF& pageRectSize,
                                            AutoManualMode pageDetectMode,
                                            bool fineTuneCorners)
    : m_pageRectSize(pageRectSize), m_pageDetectMode(pageDetectMode), m_fineTuneCorners(fineTuneCorners) {}

DefaultParams::PageBoxParams::PageBoxParams()
    : m_pageRectSize(QSizeF(210, 297)), m_pageDetectMode(MODE_DISABLED), m_fineTuneCorners(false) {}

DefaultParams::PageBoxParams::PageBoxParams(const QDomElement& el)
    : m_pageRectSize(XmlUnmarshaller::sizeF(el.namedItem("pageRectSize").toElement())),
      m_pageDetectMode(stringToAutoManualMode(el.attribute("pageDetectMode"))),
      m_fineTuneCorners(el.attribute("fineTuneCorners") == "1") {
  if (m_pageRectSize.isEmpty()) {
    m_pageRectSize = QSizeF(210, 297);
  }
}

QDomElement DefaultParams::PageBoxParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.appendChild(XmlMarshaller(doc).sizeF(m_pageRectSize, "pageRectSize"));
  el.setAttribute("pageDetectMode", autoManualModeToString(m_pageDetectMode));
  el.setAttribute("fineTuneCorners", m_fineTuneCorners ? "1" : "0");
  return el;
}

/*============================== FinalizeParams ==============================*/

namespace {
QString autoColorModePolicyToString(finalize::AutoColorModePolicy policy) {
  switch (policy) {
    case finalize::AutoColorModePolicy::NeverColor:
      return "neverColor";
    case finalize::AutoColorModePolicy::ForceBlackAndWhite:
      return "forceBlackAndWhite";
    default:
      return "bestGuess";
  }
}

finalize::AutoColorModePolicy stringToAutoColorModePolicy(const QString& str) {
  if (str == "neverColor") {
    return finalize::AutoColorModePolicy::NeverColor;
  } else if (str == "forceBlackAndWhite") {
    return finalize::AutoColorModePolicy::ForceBlackAndWhite;
  }
  return finalize::AutoColorModePolicy::BestGuess;
}

QString outputFormatToString(finalize::OutputFormat format) {
  switch (format) {
    case finalize::OutputFormat::PNG:
      return "png";
    case finalize::OutputFormat::JPEG:
      return "jpeg";
    default:
      return "tiff";
  }
}

finalize::OutputFormat stringToOutputFormat(const QString& str) {
  if (str == "png") {
    return finalize::OutputFormat::PNG;
  } else if (str == "jpeg") {
    return finalize::OutputFormat::JPEG;
  }
  return finalize::OutputFormat::TIFF;
}

QString tiffCompressionToString(finalize::TiffCompression compression) {
  return (compression == finalize::TiffCompression::Deflate) ? "deflate" : "lzw";
}

finalize::TiffCompression stringToTiffCompression(const QString& str) {
  return (str == "deflate") ? finalize::TiffCompression::Deflate : finalize::TiffCompression::LZW;
}

QString pdfQualityToString(PdfExporter::Quality quality) {
  switch (quality) {
    case PdfExporter::Quality::High:
      return "high";
    case PdfExporter::Quality::Low:
      return "low";
    default:
      return "medium";
  }
}

PdfExporter::Quality stringToPdfQuality(const QString& str) {
  if (str == "high") {
    return PdfExporter::Quality::High;
  } else if (str == "low") {
    return PdfExporter::Quality::Low;
  }
  return PdfExporter::Quality::Medium;
}
}  // namespace

DefaultParams::FinalizeParams::FinalizeParams()
    : m_autoColorModePolicy(finalize::AutoColorModePolicy::BestGuess),
      m_autoWhiteBalance(true),
      m_midtoneThreshold(8),
      m_outputFormat(finalize::OutputFormat::TIFF),
      m_tiffCompression(finalize::TiffCompression::LZW),
      m_jpegQuality(90) {}

DefaultParams::FinalizeParams::FinalizeParams(const QDomElement& el)
    : m_autoColorModePolicy(stringToAutoColorModePolicy(el.attribute("autoColorModePolicy"))),
      m_autoWhiteBalance(el.attribute("autoWhiteBalance", "1") == "1"),
      m_midtoneThreshold(el.attribute("midtoneThreshold", "8").toInt()),
      m_outputFormat(stringToOutputFormat(el.attribute("outputFormat"))),
      m_tiffCompression(stringToTiffCompression(el.attribute("tiffCompression"))),
      m_jpegQuality(el.attribute("jpegQuality", "90").toInt()) {}

QDomElement DefaultParams::FinalizeParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.setAttribute("autoColorModePolicy", autoColorModePolicyToString(m_autoColorModePolicy));
  el.setAttribute("autoWhiteBalance", m_autoWhiteBalance ? "1" : "0");
  el.setAttribute("midtoneThreshold", m_midtoneThreshold);
  el.setAttribute("outputFormat", outputFormatToString(m_outputFormat));
  el.setAttribute("tiffCompression", tiffCompressionToString(m_tiffCompression));
  el.setAttribute("jpegQuality", m_jpegQuality);
  return el;
}

/*================================= OcrParams ================================*/

DefaultParams::OcrParams::OcrParams()
    : m_ocrEnabled(true), m_language(), m_accurateRecognition(true), m_languageCorrection(true) {}

DefaultParams::OcrParams::OcrParams(const QDomElement& el)
    : m_ocrEnabled(el.attribute("enabled", "1") == "1"),
      m_language(el.attribute("language")),
      m_accurateRecognition(el.attribute("accurate", "1") == "1"),
      m_languageCorrection(el.attribute("languageCorrection", "1") == "1") {}

QDomElement DefaultParams::OcrParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.setAttribute("enabled", m_ocrEnabled ? "1" : "0");
  el.setAttribute("language", m_language);
  el.setAttribute("accurate", m_accurateRecognition ? "1" : "0");
  el.setAttribute("languageCorrection", m_languageCorrection ? "1" : "0");
  return el;
}

/*=============================== ExportParams ===============================*/

DefaultParams::ExportParams::ExportParams()
    : m_noDpiLimit(true), m_maxDpi(400), m_compressGrayscale(false), m_quality(PdfExporter::Quality::Medium) {}

DefaultParams::ExportParams::ExportParams(const QDomElement& el)
    : m_noDpiLimit(el.attribute("noDpiLimit", "1") == "1"),
      m_maxDpi(el.attribute("maxDpi", "400").toInt()),
      m_compressGrayscale(el.attribute("compressGrayscale", "0") == "1"),
      m_quality(stringToPdfQuality(el.attribute("quality"))) {}

QDomElement DefaultParams::ExportParams::toXml(QDomDocument& doc, const QString& name) const {
  QDomElement el(doc.createElement(name));
  el.setAttribute("noDpiLimit", m_noDpiLimit ? "1" : "0");
  el.setAttribute("maxDpi", m_maxDpi);
  el.setAttribute("compressGrayscale", m_compressGrayscale ? "1" : "0");
  el.setAttribute("quality", pdfQualityToString(m_quality));
  return el;
}
