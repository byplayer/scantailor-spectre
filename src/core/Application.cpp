// Copyright (C) 2019  Joseph Artsimovich <joseph.artsimovich@gmail.com>, 4lex4 <4lex49@zoho.com>
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#include "Application.h"

#include <config.h>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFileOpenEvent>
#include <QFontDatabase>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "OutOfMemoryHandler.h"

Application::Application(int& argc, char** argv) : QApplication(argc, argv), m_currentLocale("en") {
  initTranslations();
  initPortableVersion();
  loadFonts();
}

bool Application::notify(QObject* receiver, QEvent* e) {
  try {
    return QApplication::notify(receiver, e);
  } catch (const std::bad_alloc&) {
    OutOfMemoryHandler::instance().handleOutOfMemorySituation();
    return false;
  }
}

bool Application::event(QEvent* e) {
  if (e->type() == QEvent::FileOpen) {
    QFileOpenEvent* fileEvent = static_cast<QFileOpenEvent*>(e);
    emit fileOpenRequested(fileEvent->file());
    return true;
  }
  return QApplication::event(e);
}

void Application::installLanguage(const QString& locale) {
  if (m_currentLocale == locale) {
    return;
  }

  if (m_translationsMap.find(locale) != m_translationsMap.end()) {
    bool loaded = m_translator.load(m_translationsMap[locale]);

    QCoreApplication::removeTranslator(&m_translator);
    QCoreApplication::installTranslator(&m_translator);

    m_currentLocale = (loaded) ? locale : "en";
  } else {
    QCoreApplication::removeTranslator(&m_translator);

    m_currentLocale = "en";
  }
}

const QString& Application::getCurrentLocale() const {
  return m_currentLocale;
}

std::list<QString> Application::getLanguagesList() const {
  std::list<QString> list{"en"};
  std::transform(m_translationsMap.begin(), m_translationsMap.end(), std::back_inserter(list),
                 [](const std::pair<QString, QString>& val) { return val.first; });
  return list;
}

void Application::initTranslations() {
  auto opt = Qt::SkipEmptyParts;
  const QStringList translationDirs(QString::fromUtf8(TRANSLATION_DIRS).split(QChar(':'), opt));

  const QStringList languageFileFilter("scantailor_*.qm");
  for (const QString& path : translationDirs) {
    QDir dir = (QDir::isAbsolutePath(path)) ? QDir(path) : QDir::cleanPath(applicationDirPath() + '/' + path);
    if (dir.exists()) {
      QStringList translationFileNames = QDir(dir.path()).entryList(languageFileFilter);
      for (const QString& fileName : translationFileNames) {
        QString locale(fileName);
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.indexOf('_') + 1);

        m_translationsMap[locale] = dir.absoluteFilePath(fileName);
      }
    }
  }
}

void Application::initPortableVersion() {
  const QString portableConfigDirName = QString::fromUtf8(PORTABLE_CONFIG_DIR);
  if (portableConfigDirName.isEmpty()) {
    return;
  }

  const QDir portableConfigPath(applicationDirPath() + '/' + portableConfigDirName);
  // Portable mode is opt-in: a portable copy ships the config directory along
  // with it. Creating the directory on demand instead, as this used to do, made
  // every install that happened to sit somewhere writable portable.
  if (!portableConfigPath.exists()) {
    return;
  }

#ifdef Q_OS_MAC
  // The application directory is inside the bundle here, and an update replaces
  // the bundle wholesale, so settings kept there would not survive one. Remember
  // the directory only so migrateLegacyBundleConfig() can copy out of it.
  m_legacyBundleConfigPath = portableConfigPath.absolutePath();
#else
  if (QTemporaryDir(portableConfigPath.absoluteFilePath(".writable-XXXXXX")).isValid()) {
    m_portableConfigPath = portableConfigPath.absolutePath();
  }
#endif
}

namespace {
/**
 * \brief Copy \p from to \p to, unless \p to is already there.
 *
 * A destination that exists is left alone: it is the newer of the two by
 * definition, since nothing writes to the bundle any more.
 */
void copyIfMissing(const QString& from, const QString& to) {
  if (!QFile::exists(from) || QFile::exists(to)) {
    return;
  }
  if (QDir().mkpath(QFileInfo(to).absolutePath())) {
    QFile::copy(from, to);
  }
}
}  // namespace

void Application::migrateLegacyBundleConfig() {
  if (m_legacyBundleConfigPath.isEmpty()) {
    return;
  }

  const QDir legacyDir(m_legacyBundleConfigPath);

  const QDir legacyProfiles(legacyDir.absoluteFilePath("profiles"));
  const QString profilesDest = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/profiles";
  for (const QFileInfo& profile : legacyProfiles.entryInfoList({"*.stp", "*.xml"}, QDir::Files)) {
    copyIfMissing(profile.absoluteFilePath(), profilesDest + '/' + profile.fileName());
  }

  // QSettings::setPath() put the ini under <config>/<organization>/<application>.
  const QString iniName = applicationName() + ".ini";
  copyIfMissing(legacyDir.absoluteFilePath(organizationName() + '/' + iniName),
                QSettings(QSettings::IniFormat, QSettings::UserScope, organizationName(), applicationName()).fileName());
}

void Application::loadFonts() {
  QDirIterator it(":/fonts");
  while (it.hasNext()) {
    QFontDatabase::addApplicationFont(it.next());
  }
}

bool Application::isPortableVersion() const {
  return !m_portableConfigPath.isNull();
}

const QString& Application::getPortableConfigPath() const {
  return m_portableConfigPath;
}
