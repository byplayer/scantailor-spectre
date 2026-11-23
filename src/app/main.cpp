// Copyright (C) 2019  Joseph Artsimovich <joseph.artsimovich@gmail.com>, 4lex4 <4lex49@zoho.com>
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#include <config.h>
#include <core/Application.h>
#include <core/ApplicationSettings.h>
#include <core/ColorSchemeFactory.h>
#include <core/ColorSchemeManager.h>
#include <core/FontIconPack.h>
#include <core/IconProvider.h>
#include <core/StyledIconPack.h>

#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSettings>
#include <QStringList>
#include <QTextStream>
#include <QTimer>

#include "AppController.h"
#include "CliRunner.h"
#include "MainWindow.h"

#ifdef Q_OS_MAC
#include "MetalLifecycle.h"
#endif

int main(int argc, char* argv[]) {
  // A conversion asked for on the command line never shows a window, so it runs
  // on the offscreen platform plugin: no Dock icon, no window server needed.
  const bool cliMode = CliRunner::isCliInvocation(argc, argv);
  if (cliMode) {
    CliRunner::preferOffscreenPlatform(argv[0]);
  }

  // Force OpenGL backend for Qt Quick so QWebEngineView's internal QQuickWidget
  // composites correctly. Without this, QWebEngineView renders as a black rectangle
  // on macOS because the default Metal backend conflicts with WebEngine's OpenGL
  // compositor.
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  // Qt6 enables high DPI scaling by default
  Application app(argc, argv);

#ifdef Q_OS_MAC
  if (!cliMode) {
    // Initialize Metal lifecycle observer to detect app backgrounding
    // This must be done after QApplication is created but before any Metal operations
    metalLifecycleInit();

    // macOS style: don't quit when last window is closed, only on Cmd+Q or Quit menu
    Application::setQuitOnLastWindowClosed(false);
  }
#endif

#ifdef _WIN32
  // Get rid of all references to Qt's installation directory.
  Application::setLibraryPaths(QStringList(Application::applicationDirPath()));
#endif

  QStringList args = Application::arguments();

  // This information is used by QSettings.
  Application::setApplicationName(APPLICATION_NAME);
  Application::setOrganizationName(ORGANIZATION_NAME);

  QSettings::setDefaultFormat(QSettings::IniFormat);
  if (app.isPortableVersion()) {
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, app.getPortableConfigPath());
  }
  app.migrateLegacyBundleConfig();

  app.installLanguage(ApplicationSettings::getInstance().getLanguage());

  {
    std::unique_ptr<ColorScheme> scheme
        = ColorSchemeFactory().create(ApplicationSettings::getInstance().getColorScheme());
    ColorSchemeManager::instance().setColorScheme(*scheme);
  }
  IconProvider::getInstance().setIconPack(StyledIconPack::createDefault());

  if (cliMode) {
    CliRunner::Options options;
    QString errorMessage;
    if (!CliRunner::parse(args, &options, &errorMessage)) {
      QTextStream(stderr) << QCoreApplication::translate("main", "Error: %1").arg(errorMessage) << Qt::endl;
      return 2;
    }
    auto* runner = new CliRunner(options);
    QTimer::singleShot(0, runner, &CliRunner::start);
    return Application::exec();
  }

#ifdef Q_OS_MAC
  // macOS: Use AppController to manage StartupWindow and MainWindow
  AppController controller;

  // Connect file open events (from Finder double-click, drag-drop, Open With)
  QObject::connect(&app, &Application::fileOpenRequested, &controller, &AppController::openProject);

  if (args.size() > 1) {
    controller.openProject(args.at(1));
  } else {
    controller.start();
  }
#else
  // Other platforms: Show MainWindow directly (original behavior)
  auto* mainWnd = new MainWindow();
  mainWnd->setAttribute(Qt::WA_DeleteOnClose);
  QSettings settings;
  if (settings.value("mainWindow/maximized", false).toBool()) {
    QTimer::singleShot(0, mainWnd, &QMainWindow::showMaximized);
  } else {
    mainWnd->show();
  }
  if (args.size() > 1) {
    mainWnd->openProject(args.at(1));
  }
#endif

  return Application::exec();
}  // main
