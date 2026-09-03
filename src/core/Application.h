// Copyright (C) 2019  Joseph Artsimovich <joseph.artsimovich@gmail.com>, 4lex4 <4lex49@zoho.com>
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#ifndef SCANTAILOR_CORE_APPLICATION_H_
#define SCANTAILOR_CORE_APPLICATION_H_

#include <QApplication>
#include <QTimer>
#include <QtCore/QTranslator>

class Application : public QApplication {
  Q_OBJECT
 public:
  Application(int& argc, char** argv);

  bool notify(QObject* receiver, QEvent* e) override;
  bool event(QEvent* e) override;

 signals:
  void fileOpenRequested(const QString& filePath);

 public:
  const QString& getCurrentLocale() const;

  void installLanguage(const QString& locale);

  std::list<QString> getLanguagesList() const;

  bool isPortableVersion() const;

  const QString& getPortableConfigPath() const;

  /**
   * \brief Move a macOS install off of the settings kept inside its app bundle.
   *
   * Older builds turned themselves portable whenever they could write next to
   * the executable, which on macOS is inside the bundle - settings and profiles
   * ended up somewhere the next update simply replaces. Copies what is there to
   * the locations the app now uses, leaving the originals alone. Does nothing on
   * other platforms, or once the destination already has the file.
   *
   * Reads the application and organization names, so call it after those are set.
   */
  void migrateLegacyBundleConfig();

 private:
  void initTranslations();

  void initPortableVersion();

  void loadFonts();

  QTranslator m_translator;
  QString m_currentLocale;
  std::map<QString, QString> m_translationsMap;
  QString m_portableConfigPath;
  QString m_legacyBundleConfigPath;
};


#endif
