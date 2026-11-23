// Copyright (C) 2026 ScanTailor Spectre contributors
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#include "CliRunner.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QTimer>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "DefaultParams.h"
#include "DefaultParamsProfileManager.h"
#include "DefaultParamsProvider.h"
#include "ImageFileInfo.h"
#include "ImageMetadataLoader.h"
#include "MainWindow.h"
#include "PageSequence.h"
#include "PdfReadError.h"
#include "PdfReader.h"
#include "ProjectFolder.h"
#include "SmartFilenameOrdering.h"
#include "filters/finalize/Settings.h"

namespace {

const char* const kSupportedImageExtensions[] = {"png", "jpg", "jpeg", "tif", "tiff"};

QTextStream& out() {
  static QTextStream stream(stdout);
  return stream;
}

QTextStream& err() {
  static QTextStream stream(stderr);
  return stream;
}

void report(const QString& message) {
  out() << message << Qt::endl;
}

bool isProjectFile(const QString& path) {
  return path.endsWith(QStringLiteral(".ScanTailor"), Qt::CaseInsensitive);
}

bool isPdfFile(const QString& path) {
  return path.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive);
}

bool isSupportedImage(const QFileInfo& fileInfo) {
  const QString suffix = fileInfo.suffix().toLower();
  for (const char* extension : kSupportedImageExtensions) {
    if (suffix == QLatin1String(extension)) {
      return true;
    }
  }
  return false;
}

std::optional<PdfExporter::Quality> parseQuality(const QString& value) {
  if (value.compare(QStringLiteral("high"), Qt::CaseInsensitive) == 0) {
    return PdfExporter::Quality::High;
  }
  if (value.compare(QStringLiteral("medium"), Qt::CaseInsensitive) == 0) {
    return PdfExporter::Quality::Medium;
  }
  if (value.compare(QStringLiteral("low"), Qt::CaseInsensitive) == 0) {
    return PdfExporter::Quality::Low;
  }
  return std::nullopt;
}

std::optional<AutoProcessOptions::ColorHandling> parseColorHandling(const QString& value) {
  if (value.compare(QStringLiteral("bw"), Qt::CaseInsensitive) == 0) {
    return AutoProcessOptions::ColorHandling::ForceBlackAndWhite;
  }
  if (value.compare(QStringLiteral("gray"), Qt::CaseInsensitive) == 0) {
    return AutoProcessOptions::ColorHandling::ForceGrayscale;
  }
  if (value.compare(QStringLiteral("nocolor"), Qt::CaseInsensitive) == 0) {
    return AutoProcessOptions::ColorHandling::BlackAndWhiteAndGrayscale;
  }
  if (value.compare(QStringLiteral("auto"), Qt::CaseInsensitive) == 0) {
    return AutoProcessOptions::ColorHandling::BestGuess;
  }
  return std::nullopt;
}

AutoProcessOptions::ColorHandling colorHandlingOf(finalize::AutoColorModePolicy policy) {
  switch (policy) {
    case finalize::AutoColorModePolicy::ForceBlackAndWhite:
      return AutoProcessOptions::ColorHandling::ForceBlackAndWhite;
    case finalize::AutoColorModePolicy::NeverColor:
      return AutoProcessOptions::ColorHandling::BlackAndWhiteAndGrayscale;
    case finalize::AutoColorModePolicy::ForceGrayscale:
      return AutoProcessOptions::ColorHandling::ForceGrayscale;
    case finalize::AutoColorModePolicy::BestGuess:
      break;
  }
  return AutoProcessOptions::ColorHandling::BestGuess;
}

}  // namespace

bool CliRunner::isCliInvocation(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    if ((std::strcmp(arg, "-o") == 0) || (std::strcmp(arg, "--output") == 0)
        || (std::strncmp(arg, "--output=", 9) == 0) || (std::strcmp(arg, "--cli") == 0)
        || (std::strcmp(arg, "--help") == 0) || (std::strcmp(arg, "-h") == 0)
        || (std::strcmp(arg, "--version") == 0) || (std::strcmp(arg, "-v") == 0)) {
      return true;
    }
  }
  return false;
}

void CliRunner::preferOffscreenPlatform(const char* argv0) {
  if (!qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
    // The caller picked a platform - respect it.
    return;
  }

#ifdef Q_OS_MAC
  // Inside a bundle the only plugins that exist are the ones deployed into it,
  // and older bundles were built without the offscreen one.
  const QString exePath = QString::fromLocal8Bit(argv0 ? argv0 : "");
  const QString pluginsDir = QFileInfo(exePath).absolutePath() + QStringLiteral("/../PlugIns/platforms");
  if (!QFileInfo::exists(pluginsDir + QStringLiteral("/libqoffscreen.dylib"))) {
    return;
  }
#else
  Q_UNUSED(argv0)
#endif

  qputenv("QT_QPA_PLATFORM", "offscreen");
}

bool CliRunner::parse(const QStringList& args, Options* options, QString* errorMessage) {
  const auto fail = [errorMessage](const QString& message) {
    if (errorMessage) {
      *errorMessage = message;
    }
    return false;
  };

  QCommandLineParser parser;
  parser.setApplicationDescription(
      QObject::tr("Converts scans into a finished PDF without opening the workflow window.\n"
                  "\n"
                  "Examples:\n"
                  "  scantailor --output book.pdf scans.pdf\n"
                  "  scantailor --profile ~/profiles/thin-paper.stp -o book.pdf ./scans"));
  parser.addPositionalArgument(QStringLiteral("input"),
                               QObject::tr("PDF file, folder of images, or .ScanTailor project."));

  const QCommandLineOption outputOption({QStringLiteral("o"), QStringLiteral("output")},
                                        QObject::tr("Write the converted PDF to <file>."),
                                        QStringLiteral("file"));
  const QCommandLineOption profileOption({QStringLiteral("p"), QStringLiteral("profile")},
                                         QObject::tr("Profile file (.stp/.xml), or the name of an installed "
                                                     "profile, or Default / Source."),
                                         QStringLiteral("profile"));
  const QCommandLineOption workDirOption(QStringLiteral("work-dir"),
                                         QObject::tr("Folder for intermediate page images "
                                                     "(default: a temporary folder)."),
                                         QStringLiteral("dir"));
  const QCommandLineOption dpiOption(QStringLiteral("dpi"),
                                     QObject::tr("Import resolution for PDF input, and the resolution assumed "
                                                 "for images that do not declare one."),
                                     QStringLiteral("dpi"));
  const QCommandLineOption rtlOption(QStringLiteral("rtl"),
                                     QObject::tr("Right-to-left page order (folder input)."));
  const QCommandLineOption ocrOption(QStringLiteral("ocr"), QObject::tr("Run OCR and embed a text layer."));
  const QCommandLineOption noOcrOption(QStringLiteral("no-ocr"), QObject::tr("Skip OCR."));
  const QCommandLineOption ocrLanguageOption(QStringLiteral("ocr-language"),
                                             QObject::tr("OCR language code, e.g. ja-JP. Use 'auto' for "
                                                         "multilingual recognition."),
                                             QStringLiteral("code"));
  const QCommandLineOption accurateOcrOption(QStringLiteral("accurate-ocr"),
                                             QObject::tr("Accurate instead of fast recognition."));
  const QCommandLineOption fastOcrOption(QStringLiteral("fast-ocr"), QObject::tr("Fast instead of accurate "
                                                                                "recognition."));
  const QCommandLineOption colorOption(QStringLiteral("color"),
                                       QObject::tr("Color handling: auto, nocolor, gray, or bw."), QStringLiteral("mode"));
  const QCommandLineOption qualityOption(QStringLiteral("quality"),
                                         QObject::tr("PDF quality: high, medium, or low."), QStringLiteral("q"));
  const QCommandLineOption maxDpiOption(QStringLiteral("max-dpi"),
                                        QObject::tr("Downsample pages above this resolution. 0 keeps the "
                                                    "original."),
                                        QStringLiteral("dpi"));
  const QCommandLineOption compressGrayscaleOption(QStringLiteral("compress-grayscale"),
                                                   QObject::tr("JPEG-compress grayscale pages."));
  const QCommandLineOption cliOption(QStringLiteral("cli"),
                                     QObject::tr("Force command line mode (implied by --output)."));

  parser.addOptions({outputOption, profileOption, workDirOption, dpiOption, rtlOption, ocrOption, noOcrOption,
                     ocrLanguageOption, accurateOcrOption, fastOcrOption, colorOption, qualityOption, maxDpiOption,
                     compressGrayscaleOption, cliOption});
  parser.addHelpOption();
  parser.addVersionOption();

  // Exits the process for --help and --version, and on unknown options.
  parser.process(args);

  const QStringList positional = parser.positionalArguments();
  if (positional.isEmpty()) {
    return fail(QObject::tr("No input given. Pass a PDF, a folder of images, or a .ScanTailor project."));
  }
  if (positional.size() > 1) {
    return fail(QObject::tr("Only one input is supported, but %1 were given.").arg(positional.size()));
  }
  options->input = positional.first();

  if (!parser.isSet(outputOption)) {
    return fail(QObject::tr("No output given. Pass --output <file.pdf>."));
  }
  options->output = parser.value(outputOption);
  if (!options->output.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive)) {
    options->output += QStringLiteral(".pdf");
  }

  options->profile = parser.value(profileOption);
  options->workDir = parser.value(workDirOption);
  options->rtl = parser.isSet(rtlOption);

  if (parser.isSet(dpiOption)) {
    bool ok = false;
    options->importDpi = parser.value(dpiOption).toInt(&ok);
    if (!ok || options->importDpi <= 0) {
      return fail(QObject::tr("--dpi expects a positive number."));
    }
  }

  if (parser.isSet(ocrOption) && parser.isSet(noOcrOption)) {
    return fail(QObject::tr("--ocr and --no-ocr contradict each other."));
  }
  if (parser.isSet(ocrOption)) {
    options->ocr = true;
  } else if (parser.isSet(noOcrOption)) {
    options->ocr = false;
  }

  if (parser.isSet(accurateOcrOption) && parser.isSet(fastOcrOption)) {
    return fail(QObject::tr("--accurate-ocr and --fast-ocr contradict each other."));
  }
  if (parser.isSet(accurateOcrOption)) {
    options->accurateOcr = true;
  } else if (parser.isSet(fastOcrOption)) {
    options->accurateOcr = false;
  }

  if (parser.isSet(ocrLanguageOption)) {
    options->ocrLanguage = parser.value(ocrLanguageOption);
  }

  if (parser.isSet(colorOption)) {
    options->colorHandling = parseColorHandling(parser.value(colorOption));
    if (!options->colorHandling) {
      return fail(QObject::tr("--color expects auto, nocolor, gray, or bw."));
    }
  }

  if (parser.isSet(qualityOption)) {
    options->quality = parseQuality(parser.value(qualityOption));
    if (!options->quality) {
      return fail(QObject::tr("--quality expects high, medium, or low."));
    }
  }

  if (parser.isSet(maxDpiOption)) {
    bool ok = false;
    const int maxDpi = parser.value(maxDpiOption).toInt(&ok);
    if (!ok || maxDpi < 0) {
      return fail(QObject::tr("--max-dpi expects a number, 0 or greater."));
    }
    options->maxDpi = maxDpi;
  }

  if (parser.isSet(compressGrayscaleOption)) {
    options->compressGrayscale = true;
  }

  return true;
}

CliRunner::CliRunner(Options options, QObject* parent) : QObject(parent), m_options(std::move(options)) {}

CliRunner::~CliRunner() = default;

void CliRunner::start() {
  const QFileInfo inputInfo(m_options.input);
  if (!inputInfo.exists()) {
    finish(EXIT_FAILURE, tr("Input does not exist: %1").arg(m_options.input));
  }
  m_options.input = inputInfo.absoluteFilePath();
  m_options.output = QFileInfo(m_options.output).absoluteFilePath();

  const QDir outputDir = QFileInfo(m_options.output).absoluteDir();
  if (!outputDir.exists() && !QDir().mkpath(outputDir.absolutePath())) {
    finish(EXIT_FAILURE, tr("Cannot create the output folder: %1").arg(outputDir.absolutePath()));
  }

  QString errorMessage;
  // The filter settings read the defaults in their constructors, so the profile
  // has to be in place before the window builds its stages.
  if (!applyProfile(&errorMessage)) {
    finish(EXIT_FAILURE, errorMessage);
  }
  report(tr("Profile: %1").arg(DefaultParamsProvider::getInstance().getProfileName()));

  m_window = new MainWindow(/*restoreGeometry=*/false);
  m_window->setAttribute(Qt::WA_DeleteOnClose, false);
  m_window->setHeadless(true);
  connect(m_window, &MainWindow::autoProcessFinished, this, &CliRunner::onAutoProcessFinished);

  report(tr("Loading %1").arg(m_options.input));
  if (!loadInput(&errorMessage)) {
    finish(EXIT_FAILURE, errorMessage);
  }
  startProcessing();
}

/**
 * \brief Makes the requested profile the one every stage reads its defaults from.
 *
 * \p profile is either a path to a profile file or the name of an installed one.
 * Without a profile the application's current profile is kept, so a plain run
 * matches what the window would do.
 */
bool CliRunner::applyProfile(QString* errorMessage) {
  const auto fail = [errorMessage](const QString& message) {
    if (errorMessage) {
      *errorMessage = message;
    }
    return false;
  };

  DefaultParamsProvider& provider = DefaultParamsProvider::getInstance();

  if (!m_options.profile.isEmpty()) {
    const QFileInfo profileFile(m_options.profile);
    std::unique_ptr<DefaultParams> params;
    QString profileName = m_options.profile;

    if (profileFile.isFile()) {
      profileName = profileFile.completeBaseName();
      DefaultParamsProfileManager::LoadStatus status = DefaultParamsProfileManager::IO_ERROR;
      params = DefaultParamsProfileManager(profileFile.absolutePath()).readProfile(profileName, &status);
      if (status == DefaultParamsProfileManager::INCOMPATIBLE_VERSION_ERROR) {
        return fail(tr("Profile was written by an incompatible version: %1").arg(profileFile.filePath()));
      }
    } else {
      const DefaultParamsProfileManager profileManager;
      if (m_options.profile == QLatin1String("Default")) {
        params = profileManager.createDefaultProfile();
      } else if (m_options.profile == QLatin1String("Source")) {
        params = profileManager.createSourceProfile();
      } else {
        params = profileManager.readProfile(m_options.profile);
      }
    }

    if (!params) {
      return fail(tr("Cannot read the profile: %1").arg(m_options.profile));
    }
    provider.setParams(std::move(params), profileName);
  }

  // Export options given on the command line win over the profile's. They are
  // applied to the defaults because that is where the Export stage reads them.
  if (m_options.quality || m_options.maxDpi || m_options.compressGrayscale) {
    auto params = std::make_unique<DefaultParams>(provider.getParams());
    DefaultParams::ExportParams exportParams = params->getExportParams();
    if (m_options.quality) {
      exportParams.setQuality(*m_options.quality);
    }
    if (m_options.maxDpi) {
      exportParams.setDpiLimitDisabled(*m_options.maxDpi == 0);
      if (*m_options.maxDpi > 0) {
        exportParams.setMaxDpi(*m_options.maxDpi);
      }
    }
    if (m_options.compressGrayscale) {
      exportParams.setGrayscaleCompressionEnabled(*m_options.compressGrayscale);
    }
    params->setExportParams(exportParams);
    provider.setParams(std::move(params), provider.getProfileName());
  }

  return true;
}

AutoProcessOptions CliRunner::buildAutoProcessOptions() const {
  const DefaultParams& params = DefaultParamsProvider::getInstance().getParams();
  const DefaultParams::OcrParams& ocrParams = params.getOcrParams();

  AutoProcessOptions autoOptions;
  autoOptions.includeOcr = m_options.ocr.value_or(ocrParams.isOcrEnabled());
  autoOptions.redetectColor = true;
  autoOptions.colorHandling =
      m_options.colorHandling.value_or(colorHandlingOf(params.getFinalizeParams().getAutoColorModePolicy()));
  autoOptions.accurateOcr = m_options.accurateOcr.value_or(ocrParams.isAccurateRecognitionEnabled());
  autoOptions.ocrLanguageCorrection = ocrParams.isLanguageCorrectionEnabled();
  autoOptions.ocrLanguage = m_options.ocrLanguage.value_or(ocrParams.getLanguage());
  return autoOptions;
}

bool CliRunner::loadInput(QString* errorMessage) {
  const auto fail = [errorMessage](const QString& message) {
    if (errorMessage) {
      *errorMessage = message;
    }
    return false;
  };

  const QFileInfo inputInfo(m_options.input);

  if (inputInfo.isDir()) {
    // A folder may hold a saved project rather than loose images.
    const QString projectFile = ProjectFolder::findProjectFile(m_options.input);
    if (!projectFile.isEmpty()) {
      m_window->openProject(projectFile);
      return true;
    }
    return loadImageFolder(m_options.input, errorMessage);
  }

  if (isProjectFile(m_options.input)) {
    m_window->openProject(m_options.input);
    return true;
  }

  if (isPdfFile(m_options.input)) {
    const PdfReader::PdfInfo pdfInfo = PdfReader::readPdfInfo(m_options.input);
    if (pdfInfo.pageCount == 0) {
      return fail(PdfReadError::message(m_options.input));
    }
    PdfReader::setImportDpi(m_options.input, m_options.importDpi > 0 ? m_options.importDpi : pdfInfo.detectedDpi);
    if (!m_window->importPdfFileToProject(m_options.input, m_options.workDir)) {
      return fail(tr("Failed to import the PDF: %1").arg(m_options.input));
    }
    return true;
  }

  if (isSupportedImage(inputInfo)) {
    // A single image is just a folder import with one entry.
    return loadImageFolder(inputInfo.absolutePath(), errorMessage);
  }

  return fail(tr("Unsupported input: %1").arg(m_options.input));
}

/**
 * \brief Builds a project out of every supported image in \p folder.
 *
 * Images without a usable resolution are only accepted when --dpi supplies one,
 * because the interactive import would stop and ask at this point.
 */
bool CliRunner::loadImageFolder(const QString& folder, QString* errorMessage) {
  const auto fail = [errorMessage](const QString& message) {
    if (errorMessage) {
      *errorMessage = message;
    }
    return false;
  };

  QFileInfoList entries = QDir(folder).entryInfoList(QDir::Files);
  entries.erase(std::remove_if(entries.begin(), entries.end(),
                               [](const QFileInfo& entry) { return !isSupportedImage(entry); }),
                entries.end());
  std::sort(entries.begin(), entries.end(), SmartFilenameOrdering());

  if (entries.isEmpty()) {
    return fail(tr("No supported images in %1").arg(folder));
  }

  std::vector<ImageFileInfo> files;
  QStringList dpiLessFiles;
  for (const QFileInfo& entry : entries) {
    std::vector<ImageMetadata> perPageMetadata;
    const ImageMetadataLoader::Status status = ImageMetadataLoader::load(
        entry.absoluteFilePath(), [&](const ImageMetadata& metadata) { perPageMetadata.push_back(metadata); });
    if (status != ImageMetadataLoader::LOADED) {
      err() << tr("Skipping unreadable image: %1").arg(entry.absoluteFilePath()) << Qt::endl;
      continue;
    }

    for (ImageMetadata& metadata : perPageMetadata) {
      if (!metadata.isDpiOK()) {
        if (m_options.importDpi > 0) {
          metadata.setDpi(Dpi(m_options.importDpi, m_options.importDpi));
        } else {
          dpiLessFiles.append(entry.fileName());
        }
      }
    }
    files.emplace_back(entry, perPageMetadata);
  }

  if (!dpiLessFiles.isEmpty()) {
    return fail(tr("%1 image(s) have no usable resolution, starting with %2. Pass --dpi to set one.")
                    .arg(dpiLessFiles.size())
                    .arg(dpiLessFiles.first()));
  }
  if (files.empty()) {
    return fail(tr("No readable images in %1").arg(folder));
  }

  const QString outDir =
      m_options.workDir.isEmpty() ? finalize::Settings::getTempOutputDir(folder) : m_options.workDir;
  m_window->createProjectFromFiles(folder, outDir, files, m_options.rtl, /*fixDpi=*/false);
  return true;
}

/**
 * \brief Runs the auto process over the project loadInput() built.
 *
 * This waits for loadInput() to return rather than reacting to projectLoaded:
 * a PDF import emits that signal, then saves the project (which pumps the event
 * loop) and reopens it. Starting on the first signal let the run begin before
 * the reopen, which stopped it again and failed every PDF conversion.
 */
void CliRunner::startProcessing() {
  report(tr("Processing %1 page(s)").arg(static_cast<int>(m_window->allPages().numPages())));
  const AutoProcessOptions autoOptions = buildAutoProcessOptions();
  // Let the window settle into the loaded project before driving it.
  QTimer::singleShot(0, m_window, [this, autoOptions] { m_window->startAutoProcess(autoOptions); });
}

void CliRunner::onAutoProcessFinished(const bool completed) {
  if (m_autoProcessHandled) {
    return;
  }
  m_autoProcessHandled = true;

  if (!completed) {
    finish(EXIT_FAILURE, tr("Processing did not finish."));
  }

  report(tr("Exporting to %1").arg(m_options.output));
  QString errorMessage;
  if (!m_window->exportPdfHeadless(m_options.output, &errorMessage)) {
    finish(EXIT_FAILURE, errorMessage);
  }
  finish(EXIT_SUCCESS, tr("Wrote %1").arg(m_options.output));
}

/**
 * \brief Prints \p message and leaves the process.
 *
 * Like the benchmark runner, this exits without unwinding: the work is done and
 * committed to disk, while Qt's normal quit path can leave a GUI process
 * resident on macOS.
 */
void CliRunner::finish(const int exitCode, const QString& message) {
  if (exitCode == EXIT_SUCCESS) {
    out() << message << Qt::endl;
  } else {
    err() << tr("Error: %1").arg(message) << Qt::endl;
  }
  out().flush();
  err().flush();
  std::fflush(nullptr);
  std::_Exit(exitCode);
}
