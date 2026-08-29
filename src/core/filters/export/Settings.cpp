// Copyright (C) 2024  ScanTailor Spectre contributors
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#include "Settings.h"

#include <QMutexLocker>

#include "DefaultParams.h"
#include "DefaultParamsProvider.h"

namespace export_ {

namespace {
const DefaultParams::ExportParams& exportDefaults() {
  return DefaultParamsProvider::getInstance().getParams().getExportParams();
}
}  // namespace

Settings::Settings()
    : m_noDpiLimit(exportDefaults().isDpiLimitDisabled()),
      m_maxDpi(exportDefaults().getMaxDpi()),
      m_compressGrayscale(exportDefaults().isGrayscaleCompressionEnabled()),
      m_quality(exportDefaults().getQuality()) {}

bool Settings::noDpiLimit() const {
  QMutexLocker locker(&m_mutex);
  return m_noDpiLimit;
}

void Settings::setNoDpiLimit(bool value) {
  QMutexLocker locker(&m_mutex);
  m_noDpiLimit = value;
}

int Settings::maxDpi() const {
  QMutexLocker locker(&m_mutex);
  return m_maxDpi;
}

void Settings::setMaxDpi(int dpi) {
  QMutexLocker locker(&m_mutex);
  m_maxDpi = dpi;
}

bool Settings::compressGrayscale() const {
  QMutexLocker locker(&m_mutex);
  return m_compressGrayscale;
}

void Settings::setCompressGrayscale(bool value) {
  QMutexLocker locker(&m_mutex);
  m_compressGrayscale = value;
}

PdfExporter::Quality Settings::quality() const {
  QMutexLocker locker(&m_mutex);
  return m_quality;
}

void Settings::setQuality(PdfExporter::Quality quality) {
  QMutexLocker locker(&m_mutex);
  m_quality = quality;
}

BookMetadata Settings::bookMetadata() const {
  QMutexLocker locker(&m_mutex);
  return m_bookMetadata;
}

void Settings::setBookMetadata(const BookMetadata& metadata) {
  QMutexLocker locker(&m_mutex);
  m_bookMetadata = metadata;
}

bool Settings::sendToZotero() const {
  QMutexLocker locker(&m_mutex);
  return m_sendToZotero;
}

bool Settings::hasExplicitSendToZoteroChoice() const {
  QMutexLocker locker(&m_mutex);
  return m_hasExplicitSendToZoteroChoice;
}

void Settings::setSendToZotero(bool value, bool explicitChoice) {
  QMutexLocker locker(&m_mutex);
  m_sendToZotero = value;
  m_hasExplicitSendToZoteroChoice = explicitChoice;
}

void Settings::armSendToZoteroForLoopProject() {
  QMutexLocker locker(&m_mutex);
  if (!m_hasExplicitSendToZoteroChoice) {
    m_sendToZotero = true;
  }
}

}  // namespace export_
