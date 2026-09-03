// Copyright (C) 2026 ScanTailor Spectre contributors
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#ifndef SCANTAILOR_APP_AUTOPROCESSOPTIONS_H_
#define SCANTAILOR_APP_AUTOPROCESSOPTIONS_H_

#include <QString>
#include <optional>

/**
 * Everything an Auto Process run needs to know before it starts.
 *
 * AutoProcessDialog fills this in for interactive runs; the command line front
 * end fills the same struct without asking anyone anything.
 */
struct AutoProcessOptions {
  enum class ColorHandling {
    ForceBlackAndWhite = 0,
    BlackAndWhiteAndGrayscale = 1,
    BestGuess = 2,
    ForceGrayscale = 3
  };

  /** Run the OCR stage and keep its results for the exported text layer. */
  bool includeOcr = false;

  /** Throw away color decisions made by earlier runs and detect them again. */
  bool redetectColor = true;

  ColorHandling colorHandling = ColorHandling::BestGuess;

  /** Apple Vision accurate recognition, as opposed to the fast pass. */
  bool accurateOcr = true;

  /** Unset leaves whatever the OCR stage is already configured with. */
  std::optional<bool> ocrLanguageCorrection;

  /**
   * Unset leaves the OCR stage's own language alone. "auto" means multilingual
   * recognition; an empty string falls back to the system language.
   */
  std::optional<QString> ocrLanguage;
};

#endif  // SCANTAILOR_APP_AUTOPROCESSOPTIONS_H_
