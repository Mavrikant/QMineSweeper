#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QString>
#include <QVector>

class QApplication;

// Owns the list of supported UI languages and the rules for picking one.
// Resolution order on startup: user override (QSettings) → system locale →
// "en". The translator .qm files are embedded at build time under :/i18n/
// by qt_add_translations in CMakeLists.txt.
namespace Language
{
struct LocaleEntry
{
    QString code;         // e.g. "en", "tr_TR", "zh_CN"
    QString nativeName;   // shown in the menu: "English", "Türkçe", …
    QString englishName;  // for tooltips / docs
    QString flagResource; // e.g. ":/icons/flags/tr_TR.png"
};

[[nodiscard]] const QVector<LocaleEntry> &supported();

// Returns the chosen locale code: QSettings override, else best match of
// QLocale::system().uiLanguages() against supported(), else "en".
[[nodiscard]] QString resolveStartupLocale();

// QSettings-backed override. setUserOverride("") or clearUserOverride()
// reverts to system-auto.
[[nodiscard]] QString userOverride();
void setUserOverride(const QString &code);
void clearUserOverride();

// Loads :/i18n/QMineSweeper_<code>.qm and installs it on the app. Returns
// false if no translator matched (caller can then call with "en"). Owns an
// internal QTranslator so it stays alive for the app's lifetime.
bool applyTranslator(QApplication *app, const QString &code);
} // namespace Language

#endif // LANGUAGE_H
