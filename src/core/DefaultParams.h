// Copyright (C) 2019  Joseph Artsimovich <joseph.artsimovich@gmail.com>, 4lex4 <4lex49@zoho.com>
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#ifndef SCANTAILOR_CORE_DEFAULTPARAMS_H_
#define SCANTAILOR_CORE_DEFAULTPARAMS_H_


#include <filters/output/ColorParams.h>
#include <filters/output/DepthPerception.h>
#include <filters/output/DespeckleLevel.h>
#include <filters/output/DewarpingOptions.h>
#include <filters/output/PictureShapeOptions.h>
#include <filters/page_layout/Alignment.h>
#include <filters/page_split/LayoutType.h>
#include <filters/finalize/Settings.h>

#include <QtCore/QRectF>

#include "AutoManualMode.h"
#include "Dpi.h"
#include "PdfExporter.h"
#include "Margins.h"
#include "OrthogonalRotation.h"
#include "Units.h"

class DefaultParams {
 public:
  class FixOrientationParams {
   public:
    FixOrientationParams() = default;

    explicit FixOrientationParams(const OrthogonalRotation& imageRotation);

    explicit FixOrientationParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    const OrthogonalRotation& getImageRotation() const;

    void setImageRotation(const OrthogonalRotation& imageRotation);

   private:
    OrthogonalRotation m_imageRotation;
  };

  class DeskewParams {
   public:
    DeskewParams();

    explicit DeskewParams(double deskewAngleDeg, AutoManualMode mode);

    explicit DeskewParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    double getDeskewAngleDeg() const;

    void setDeskewAngleDeg(double deskewAngleDeg);

    AutoManualMode getMode() const;

    void setMode(AutoManualMode mode);

   private:
    double m_deskewAngleDeg;
    AutoManualMode m_mode;
  };

  class PageSplitParams {
   public:
    PageSplitParams();

    explicit PageSplitParams(page_split::LayoutType layoutType);

    explicit PageSplitParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    page_split::LayoutType getLayoutType() const;

    void setLayoutType(page_split::LayoutType layoutType);

   private:
    page_split::LayoutType m_layoutType;
  };

  class SelectContentParams {
   public:
    SelectContentParams();

    SelectContentParams(const QSizeF& pageRectSize,
                        bool contentDetectEnabled,
                        AutoManualMode pageDetectMode,
                        bool fineTuneCorners);

    explicit SelectContentParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    const QSizeF& getPageRectSize() const;

    void setPageRectSize(const QSizeF& pageRectSize);

    bool isContentDetectEnabled() const;

    void setContentDetectEnabled(bool contentDetectEnabled);

    bool isFineTuneCorners() const;

    void setFineTuneCorners(bool fineTuneCorners);

    AutoManualMode getPageDetectMode() const;

    void setPageDetectMode(AutoManualMode pageDetectMode);

    double getContentFillFactor() const;

    void setContentFillFactor(double contentFillFactor);

    int getBorderTolerance() const;

    void setBorderTolerance(int borderTolerance);

   private:
    QSizeF m_pageRectSize;
    bool m_contentDetectEnabled;
    AutoManualMode m_pageDetectMode;
    bool m_fineTuneCorners;
    double m_contentFillFactor;
    int m_borderTolerance;
  };

  class PageBoxParams {
   public:
    PageBoxParams();

    PageBoxParams(const QSizeF& pageRectSize, AutoManualMode pageDetectMode, bool fineTuneCorners);

    explicit PageBoxParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    const QSizeF& getPageRectSize() const;

    void setPageRectSize(const QSizeF& pageRectSize);

    AutoManualMode getPageDetectMode() const;

    void setPageDetectMode(AutoManualMode pageDetectMode);

    bool isFineTuneCorners() const;

    void setFineTuneCorners(bool fineTuneCorners);

   private:
    QSizeF m_pageRectSize;
    AutoManualMode m_pageDetectMode;
    bool m_fineTuneCorners;
  };

  class PageLayoutParams {
   public:
    PageLayoutParams();

    PageLayoutParams(const Margins& hardMargins, const page_layout::Alignment& alignment, bool autoMargins);

    explicit PageLayoutParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    const Margins& getHardMargins() const;

    void setHardMargins(const Margins& hardMargins);

    const page_layout::Alignment& getAlignment() const;

    void setAlignment(const page_layout::Alignment& alignment);

    bool isAutoMargins() const;

    void setAutoMargins(bool autoMargins);

   private:
    Margins m_hardMargins;
    page_layout::Alignment m_alignment;
    bool m_autoMargins;
  };

  class OutputParams {
   public:
    OutputParams();

    OutputParams(const Dpi& dpi,
                 const output::ColorParams& colorParams,
                 const output::SplittingOptions& splittingOptions,
                 const output::PictureShapeOptions& pictureShapeOptions,
                 const output::DepthPerception& depthPerception,
                 const output::DewarpingOptions& dewarpingOptions,
                 double despeckleLevel);

    explicit OutputParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    const Dpi& getDpi() const;

    void setDpi(const Dpi& dpi);

    const output::ColorParams& getColorParams() const;

    void setColorParams(const output::ColorParams& colorParams);

    const output::SplittingOptions& getSplittingOptions() const;

    void setSplittingOptions(const output::SplittingOptions& splittingOptions);

    const output::PictureShapeOptions& getPictureShapeOptions() const;

    void setPictureShapeOptions(const output::PictureShapeOptions& pictureShapeOptions);

    const output::DepthPerception& getDepthPerception() const;

    void setDepthPerception(const output::DepthPerception& depthPerception);

    const output::DewarpingOptions& getDewarpingOptions() const;

    void setDewarpingOptions(const output::DewarpingOptions& dewarpingOptions);

    double getDespeckleLevel() const;

    void setDespeckleLevel(double despeckleLevel);

    bool isPassThroughEnabled() const;

    void setPassThroughEnabled(bool enabled);

   private:
    Dpi m_dpi;
    output::ColorParams m_colorParams;
    output::SplittingOptions m_splittingOptions;
    output::PictureShapeOptions m_pictureShapeOptions;
    output::DepthPerception m_depthPerception;
    output::DewarpingOptions m_dewarpingOptions;
    double m_despeckleLevel;
    bool m_passThrough;
  };

  class FinalizeParams {
   public:
    FinalizeParams();

    explicit FinalizeParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    finalize::AutoColorModePolicy getAutoColorModePolicy() const;

    void setAutoColorModePolicy(finalize::AutoColorModePolicy policy);

    bool isAutoWhiteBalanceEnabled() const;

    void setAutoWhiteBalanceEnabled(bool enabled);

    int getMidtoneThreshold() const;

    void setMidtoneThreshold(int threshold);

    finalize::OutputFormat getOutputFormat() const;

    void setOutputFormat(finalize::OutputFormat format);

    finalize::TiffCompression getTiffCompression() const;

    void setTiffCompression(finalize::TiffCompression compression);

    int getJpegQuality() const;

    void setJpegQuality(int quality);

   private:
    finalize::AutoColorModePolicy m_autoColorModePolicy;
    bool m_autoWhiteBalance;
    int m_midtoneThreshold;
    finalize::OutputFormat m_outputFormat;
    finalize::TiffCompression m_tiffCompression;
    int m_jpegQuality;
  };

  class OcrParams {
   public:
    OcrParams();

    explicit OcrParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    bool isOcrEnabled() const;

    void setOcrEnabled(bool enabled);

    const QString& getLanguage() const;

    void setLanguage(const QString& language);

    bool isAccurateRecognitionEnabled() const;

    void setAccurateRecognitionEnabled(bool enabled);

    bool isLanguageCorrectionEnabled() const;

    void setLanguageCorrectionEnabled(bool enabled);

   private:
    bool m_ocrEnabled;
    QString m_language;
    bool m_accurateRecognition;
    bool m_languageCorrection;
  };

  class ExportParams {
   public:
    ExportParams();

    explicit ExportParams(const QDomElement& el);

    QDomElement toXml(QDomDocument& doc, const QString& name) const;

    bool isDpiLimitDisabled() const;

    void setDpiLimitDisabled(bool disabled);

    int getMaxDpi() const;

    void setMaxDpi(int dpi);

    bool isGrayscaleCompressionEnabled() const;

    void setGrayscaleCompressionEnabled(bool enabled);

    PdfExporter::Quality getQuality() const;

    void setQuality(PdfExporter::Quality quality);

   private:
    bool m_noDpiLimit;
    int m_maxDpi;
    bool m_compressGrayscale;
    PdfExporter::Quality m_quality;
  };

 public:
  DefaultParams();

  DefaultParams(const FixOrientationParams& fixOrientationParams,
                const DeskewParams& deskewParams,
                const PageSplitParams& pageSplitParams,
                const PageBoxParams& pageBoxParams,
                const SelectContentParams& selectContentParams,
                const PageLayoutParams& pageLayoutParams,
                const FinalizeParams& finalizeParams,
                const OutputParams& outputParams,
                const OcrParams& ocrParams,
                const ExportParams& exportParams);

  explicit DefaultParams(const QDomElement& el);

  QDomElement toXml(QDomDocument& doc, const QString& name) const;

  const FixOrientationParams& getFixOrientationParams() const;

  void setFixOrientationParams(const FixOrientationParams& fixOrientationParams);

  const DeskewParams& getDeskewParams() const;

  void setDeskewParams(const DeskewParams& deskewParams);

  const PageSplitParams& getPageSplitParams() const;

  void setPageSplitParams(const PageSplitParams& pageSplitParams);

  const SelectContentParams& getSelectContentParams() const;

  void setSelectContentParams(const SelectContentParams& selectContentParams);

  const PageLayoutParams& getPageLayoutParams() const;

  void setPageLayoutParams(const PageLayoutParams& pageLayoutParams);

  const OutputParams& getOutputParams() const;

  void setOutputParams(const OutputParams& outputParams);

  const PageBoxParams& getPageBoxParams() const;

  void setPageBoxParams(const PageBoxParams& pageBoxParams);

  const FinalizeParams& getFinalizeParams() const;

  void setFinalizeParams(const FinalizeParams& finalizeParams);

  const OcrParams& getOcrParams() const;

  void setOcrParams(const OcrParams& ocrParams);

  const ExportParams& getExportParams() const;

  void setExportParams(const ExportParams& exportParams);

  Units getUnits() const;

  void setUnits(Units units);

 private:
  FixOrientationParams m_fixOrientationParams;
  DeskewParams m_deskewParams;
  PageSplitParams m_pageSplitParams;
  PageBoxParams m_pageBoxParams;
  SelectContentParams m_selectContentParams;
  PageLayoutParams m_pageLayoutParams;
  FinalizeParams m_finalizeParams;
  OutputParams m_outputParams;
  OcrParams m_ocrParams;
  ExportParams m_exportParams;
  Units m_units;
};


inline Units DefaultParams::getUnits() const {
  return m_units;
}

inline void DefaultParams::setUnits(Units units) {
  DefaultParams::m_units = units;
}

inline const DefaultParams::FixOrientationParams& DefaultParams::getFixOrientationParams() const {
  return m_fixOrientationParams;
}

inline void DefaultParams::setFixOrientationParams(const DefaultParams::FixOrientationParams& fixOrientationParams) {
  DefaultParams::m_fixOrientationParams = fixOrientationParams;
}

inline const DefaultParams::DeskewParams& DefaultParams::getDeskewParams() const {
  return m_deskewParams;
}

inline void DefaultParams::setDeskewParams(const DefaultParams::DeskewParams& deskewParams) {
  DefaultParams::m_deskewParams = deskewParams;
}

inline const DefaultParams::PageSplitParams& DefaultParams::getPageSplitParams() const {
  return m_pageSplitParams;
}

inline void DefaultParams::setPageSplitParams(const DefaultParams::PageSplitParams& pageSplitParams) {
  DefaultParams::m_pageSplitParams = pageSplitParams;
}

inline const DefaultParams::SelectContentParams& DefaultParams::getSelectContentParams() const {
  return m_selectContentParams;
}

inline void DefaultParams::setSelectContentParams(const DefaultParams::SelectContentParams& selectContentParams) {
  DefaultParams::m_selectContentParams = selectContentParams;
}

inline const DefaultParams::PageLayoutParams& DefaultParams::getPageLayoutParams() const {
  return m_pageLayoutParams;
}

inline void DefaultParams::setPageLayoutParams(const DefaultParams::PageLayoutParams& pageLayoutParams) {
  DefaultParams::m_pageLayoutParams = pageLayoutParams;
}

inline const DefaultParams::OutputParams& DefaultParams::getOutputParams() const {
  return m_outputParams;
}

inline void DefaultParams::setOutputParams(const DefaultParams::OutputParams& outputParams) {
  DefaultParams::m_outputParams = outputParams;
}

inline const OrthogonalRotation& DefaultParams::FixOrientationParams::getImageRotation() const {
  return m_imageRotation;
}

inline void DefaultParams::FixOrientationParams::setImageRotation(const OrthogonalRotation& imageRotation) {
  FixOrientationParams::m_imageRotation = imageRotation;
}

inline double DefaultParams::DeskewParams::getDeskewAngleDeg() const {
  return m_deskewAngleDeg;
}

inline void DefaultParams::DeskewParams::setDeskewAngleDeg(double deskewAngleDeg) {
  DeskewParams::m_deskewAngleDeg = deskewAngleDeg;
}

inline AutoManualMode DefaultParams::DeskewParams::getMode() const {
  return m_mode;
}

inline void DefaultParams::DeskewParams::setMode(AutoManualMode mode) {
  DeskewParams::m_mode = mode;
}

inline page_split::LayoutType DefaultParams::PageSplitParams::getLayoutType() const {
  return m_layoutType;
}

inline void DefaultParams::PageSplitParams::setLayoutType(page_split::LayoutType layoutType) {
  PageSplitParams::m_layoutType = layoutType;
}

inline const QSizeF& DefaultParams::SelectContentParams::getPageRectSize() const {
  return m_pageRectSize;
}

inline void DefaultParams::SelectContentParams::setPageRectSize(const QSizeF& pageRectSize) {
  SelectContentParams::m_pageRectSize = pageRectSize;
}

inline bool DefaultParams::SelectContentParams::isContentDetectEnabled() const {
  return m_contentDetectEnabled;
}

inline void DefaultParams::SelectContentParams::setContentDetectEnabled(bool contentDetectEnabled) {
  SelectContentParams::m_contentDetectEnabled = contentDetectEnabled;
}

inline bool DefaultParams::SelectContentParams::isFineTuneCorners() const {
  return m_fineTuneCorners;
}

inline void DefaultParams::SelectContentParams::setFineTuneCorners(bool fineTuneCorners) {
  SelectContentParams::m_fineTuneCorners = fineTuneCorners;
}

inline AutoManualMode DefaultParams::SelectContentParams::getPageDetectMode() const {
  return m_pageDetectMode;
}

inline void DefaultParams::SelectContentParams::setPageDetectMode(AutoManualMode pageDetectMode) {
  SelectContentParams::m_pageDetectMode = pageDetectMode;
}

inline const Margins& DefaultParams::PageLayoutParams::getHardMargins() const {
  return m_hardMargins;
}

inline void DefaultParams::PageLayoutParams::setHardMargins(const Margins& hardMargins) {
  PageLayoutParams::m_hardMargins = hardMargins;
}

inline const page_layout::Alignment& DefaultParams::PageLayoutParams::getAlignment() const {
  return m_alignment;
}

inline void DefaultParams::PageLayoutParams::setAlignment(const page_layout::Alignment& alignment) {
  PageLayoutParams::m_alignment = alignment;
}

inline bool DefaultParams::PageLayoutParams::isAutoMargins() const {
  return m_autoMargins;
}

inline void DefaultParams::PageLayoutParams::setAutoMargins(bool autoMargins) {
  PageLayoutParams::m_autoMargins = autoMargins;
}

inline const Dpi& DefaultParams::OutputParams::getDpi() const {
  return m_dpi;
}

inline void DefaultParams::OutputParams::setDpi(const Dpi& dpi) {
  OutputParams::m_dpi = dpi;
}

inline const output::ColorParams& DefaultParams::OutputParams::getColorParams() const {
  return m_colorParams;
}

inline void DefaultParams::OutputParams::setColorParams(const output::ColorParams& colorParams) {
  OutputParams::m_colorParams = colorParams;
}

inline const output::SplittingOptions& DefaultParams::OutputParams::getSplittingOptions() const {
  return m_splittingOptions;
}

inline void DefaultParams::OutputParams::setSplittingOptions(const output::SplittingOptions& splittingOptions) {
  OutputParams::m_splittingOptions = splittingOptions;
}

inline const output::PictureShapeOptions& DefaultParams::OutputParams::getPictureShapeOptions() const {
  return m_pictureShapeOptions;
}

inline void DefaultParams::OutputParams::setPictureShapeOptions(
    const output::PictureShapeOptions& pictureShapeOptions) {
  OutputParams::m_pictureShapeOptions = pictureShapeOptions;
}

inline const output::DepthPerception& DefaultParams::OutputParams::getDepthPerception() const {
  return m_depthPerception;
}

inline void DefaultParams::OutputParams::setDepthPerception(const output::DepthPerception& depthPerception) {
  OutputParams::m_depthPerception = depthPerception;
}

inline const output::DewarpingOptions& DefaultParams::OutputParams::getDewarpingOptions() const {
  return m_dewarpingOptions;
}

inline void DefaultParams::OutputParams::setDewarpingOptions(const output::DewarpingOptions& dewarpingOptions) {
  OutputParams::m_dewarpingOptions = dewarpingOptions;
}

inline double DefaultParams::OutputParams::getDespeckleLevel() const {
  return m_despeckleLevel;
}

inline void DefaultParams::OutputParams::setDespeckleLevel(double despeckleLevel) {
  OutputParams::m_despeckleLevel = despeckleLevel;
}

inline bool DefaultParams::OutputParams::isPassThroughEnabled() const {
  return m_passThrough;
}

inline void DefaultParams::OutputParams::setPassThroughEnabled(bool enabled) {
  OutputParams::m_passThrough = enabled;
}

inline double DefaultParams::SelectContentParams::getContentFillFactor() const {
  return m_contentFillFactor;
}

inline void DefaultParams::SelectContentParams::setContentFillFactor(double contentFillFactor) {
  SelectContentParams::m_contentFillFactor = contentFillFactor;
}

inline int DefaultParams::SelectContentParams::getBorderTolerance() const {
  return m_borderTolerance;
}

inline void DefaultParams::SelectContentParams::setBorderTolerance(int borderTolerance) {
  SelectContentParams::m_borderTolerance = borderTolerance;
}

inline const DefaultParams::PageBoxParams& DefaultParams::getPageBoxParams() const {
  return m_pageBoxParams;
}

inline void DefaultParams::setPageBoxParams(const DefaultParams::PageBoxParams& pageBoxParams) {
  DefaultParams::m_pageBoxParams = pageBoxParams;
}

inline const DefaultParams::FinalizeParams& DefaultParams::getFinalizeParams() const {
  return m_finalizeParams;
}

inline void DefaultParams::setFinalizeParams(const DefaultParams::FinalizeParams& finalizeParams) {
  DefaultParams::m_finalizeParams = finalizeParams;
}

inline const DefaultParams::OcrParams& DefaultParams::getOcrParams() const {
  return m_ocrParams;
}

inline void DefaultParams::setOcrParams(const DefaultParams::OcrParams& ocrParams) {
  DefaultParams::m_ocrParams = ocrParams;
}

inline const DefaultParams::ExportParams& DefaultParams::getExportParams() const {
  return m_exportParams;
}

inline void DefaultParams::setExportParams(const DefaultParams::ExportParams& exportParams) {
  DefaultParams::m_exportParams = exportParams;
}

inline const QSizeF& DefaultParams::PageBoxParams::getPageRectSize() const {
  return m_pageRectSize;
}

inline void DefaultParams::PageBoxParams::setPageRectSize(const QSizeF& pageRectSize) {
  PageBoxParams::m_pageRectSize = pageRectSize;
}

inline AutoManualMode DefaultParams::PageBoxParams::getPageDetectMode() const {
  return m_pageDetectMode;
}

inline void DefaultParams::PageBoxParams::setPageDetectMode(AutoManualMode pageDetectMode) {
  PageBoxParams::m_pageDetectMode = pageDetectMode;
}

inline bool DefaultParams::PageBoxParams::isFineTuneCorners() const {
  return m_fineTuneCorners;
}

inline void DefaultParams::PageBoxParams::setFineTuneCorners(bool fineTuneCorners) {
  PageBoxParams::m_fineTuneCorners = fineTuneCorners;
}

inline finalize::AutoColorModePolicy DefaultParams::FinalizeParams::getAutoColorModePolicy() const {
  return m_autoColorModePolicy;
}

inline void DefaultParams::FinalizeParams::setAutoColorModePolicy(finalize::AutoColorModePolicy policy) {
  FinalizeParams::m_autoColorModePolicy = policy;
}

inline bool DefaultParams::FinalizeParams::isAutoWhiteBalanceEnabled() const {
  return m_autoWhiteBalance;
}

inline void DefaultParams::FinalizeParams::setAutoWhiteBalanceEnabled(bool enabled) {
  FinalizeParams::m_autoWhiteBalance = enabled;
}

inline int DefaultParams::FinalizeParams::getMidtoneThreshold() const {
  return m_midtoneThreshold;
}

inline void DefaultParams::FinalizeParams::setMidtoneThreshold(int threshold) {
  FinalizeParams::m_midtoneThreshold = threshold;
}

inline finalize::OutputFormat DefaultParams::FinalizeParams::getOutputFormat() const {
  return m_outputFormat;
}

inline void DefaultParams::FinalizeParams::setOutputFormat(finalize::OutputFormat format) {
  FinalizeParams::m_outputFormat = format;
}

inline finalize::TiffCompression DefaultParams::FinalizeParams::getTiffCompression() const {
  return m_tiffCompression;
}

inline void DefaultParams::FinalizeParams::setTiffCompression(finalize::TiffCompression compression) {
  FinalizeParams::m_tiffCompression = compression;
}

inline int DefaultParams::FinalizeParams::getJpegQuality() const {
  return m_jpegQuality;
}

inline void DefaultParams::FinalizeParams::setJpegQuality(int quality) {
  FinalizeParams::m_jpegQuality = quality;
}

inline bool DefaultParams::OcrParams::isOcrEnabled() const {
  return m_ocrEnabled;
}

inline void DefaultParams::OcrParams::setOcrEnabled(bool enabled) {
  OcrParams::m_ocrEnabled = enabled;
}

inline const QString& DefaultParams::OcrParams::getLanguage() const {
  return m_language;
}

inline void DefaultParams::OcrParams::setLanguage(const QString& language) {
  OcrParams::m_language = language;
}

inline bool DefaultParams::OcrParams::isAccurateRecognitionEnabled() const {
  return m_accurateRecognition;
}

inline void DefaultParams::OcrParams::setAccurateRecognitionEnabled(bool enabled) {
  OcrParams::m_accurateRecognition = enabled;
}

inline bool DefaultParams::OcrParams::isLanguageCorrectionEnabled() const {
  return m_languageCorrection;
}

inline void DefaultParams::OcrParams::setLanguageCorrectionEnabled(bool enabled) {
  OcrParams::m_languageCorrection = enabled;
}

inline bool DefaultParams::ExportParams::isDpiLimitDisabled() const {
  return m_noDpiLimit;
}

inline void DefaultParams::ExportParams::setDpiLimitDisabled(bool disabled) {
  ExportParams::m_noDpiLimit = disabled;
}

inline int DefaultParams::ExportParams::getMaxDpi() const {
  return m_maxDpi;
}

inline void DefaultParams::ExportParams::setMaxDpi(int dpi) {
  ExportParams::m_maxDpi = dpi;
}

inline bool DefaultParams::ExportParams::isGrayscaleCompressionEnabled() const {
  return m_compressGrayscale;
}

inline void DefaultParams::ExportParams::setGrayscaleCompressionEnabled(bool enabled) {
  ExportParams::m_compressGrayscale = enabled;
}

inline PdfExporter::Quality DefaultParams::ExportParams::getQuality() const {
  return m_quality;
}

inline void DefaultParams::ExportParams::setQuality(PdfExporter::Quality quality) {
  ExportParams::m_quality = quality;
}


#endif  // SCANTAILOR_CORE_DEFAULTPARAMS_H_
