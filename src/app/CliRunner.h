// Copyright (C) 2026 ScanTailor Spectre contributors
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#ifndef SCANTAILOR_APP_CLIRUNNER_H_
#define SCANTAILOR_APP_CLIRUNNER_H_

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <optional>

#include "AutoProcessOptions.h"
#include "PdfExporter.h"

class MainWindow;

/**
 * \brief Non-interactive front end: one input, one profile, one PDF.
 *
 * The regular pipeline does the work - a MainWindow is built and driven through
 * Auto Process exactly as it would be by hand, it is just never shown. Loading
 * a project and processing it are asynchronous, so this runner is a small state
 * machine that waits for the window's signals and exits when the PDF is written.
 */
class CliRunner : public QObject {
  Q_OBJECT

 public:
  struct Options {
    /** PDF file, folder of images, or .ScanTailor project. */
    QString input;
    /** Where the converted PDF goes. */
    QString output;
    /** Profile file path, or the name of an installed profile. */
    QString profile;
    /** Where intermediate page images are written. Empty means a temp folder. */
    QString workDir;
    /** Import resolution for PDF input, and the fallback for DPI-less images. */
    int importDpi = 0;
    /** Right-to-left page order for folder input. */
    bool rtl = false;

    // Anything left unset below is taken from the profile.
    std::optional<bool> ocr;
    std::optional<QString> ocrLanguage;
    std::optional<bool> accurateOcr;
    std::optional<AutoProcessOptions::ColorHandling> colorHandling;
    std::optional<PdfExporter::Quality> quality;
    std::optional<int> maxDpi;
    std::optional<bool> compressGrayscale;
  };

  /**
   * \brief Whether these arguments ask for a conversion rather than the GUI.
   *
   * Checked before QApplication exists, so it works on the raw argv.
   */
  static bool isCliInvocation(int argc, char** argv);

  /**
   * \brief Ask for the offscreen platform plugin, when there is one to ask for.
   *
   * A conversion never shows a window, so the offscreen plugin keeps the run out
   * of the Dock and lets it work without a window server. Not every install ships
   * that plugin though - if it is missing, saying nothing and letting Qt pick its
   * default platform is far better than refusing to start. Must be called before
   * QApplication is constructed, so it works on the raw argv.
   */
  static void preferOffscreenPlatform(const char* argv0);

  /**
   * \brief Turns the command line into Options.
   *
   * Handles --help and --version itself (both exit the process). Returns false
   * with \p errorMessage filled in when the arguments do not make sense.
   */
  static bool parse(const QStringList& args, Options* options, QString* errorMessage);

  explicit CliRunner(Options options, QObject* parent = nullptr);

  ~CliRunner() override;

  /** Kicks off the conversion. Call it from the event loop, not before it. */
  void start();

 private:
  void startProcessing();
  void onAutoProcessFinished(bool completed);
  bool applyProfile(QString* errorMessage);
  AutoProcessOptions buildAutoProcessOptions() const;
  bool loadInput(QString* errorMessage);
  bool loadImageFolder(const QString& folder, QString* errorMessage);
  [[noreturn]] void finish(int exitCode, const QString& message);

  Options m_options;
  QPointer<MainWindow> m_window;
  bool m_autoProcessHandled = false;
};

#endif  // SCANTAILOR_APP_CLIRUNNER_H_
