#include "language.h"

#include <QApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

namespace
{
constexpr auto kSettingsKey = "language/code";

const QVector<Language::LocaleEntry> &localeTable()
{
    static const QVector<Language::LocaleEntry> kTable = {
        {QStringLiteral("en"), QStringLiteral("English"), QStringLiteral("English"), QStringLiteral(":/icons/flags/en.png")},
        {QStringLiteral("tr_TR"), QStringLiteral("Türkçe"), QStringLiteral("Turkish"), QStringLiteral(":/icons/flags/tr_TR.png")},
        {QStringLiteral("zh_CN"), QStringLiteral("中文（简体）"), QStringLiteral("Chinese (Simplified)"), QStringLiteral(":/icons/flags/zh_CN.png")},
        {QStringLiteral("hi_IN"), QStringLiteral("हिन्दी"), QStringLiteral("Hindi"), QStringLiteral(":/icons/flags/hi_IN.png")},
        {QStringLiteral("es_ES"), QStringLiteral("Español"), QStringLiteral("Spanish"), QStringLiteral(":/icons/flags/es_ES.png")},
        {QStringLiteral("ar_SA"), QStringLiteral("العربية"), QStringLiteral("Arabic"), QStringLiteral(":/icons/flags/ar_SA.png")},
        {QStringLiteral("fr_FR"), QStringLiteral("Français"), QStringLiteral("French"), QStringLiteral(":/icons/flags/fr_FR.png")},
        {QStringLiteral("ru_RU"), QStringLiteral("Русский"), QStringLiteral("Russian"), QStringLiteral(":/icons/flags/ru_RU.png")},
        {QStringLiteral("pt_BR"), QStringLiteral("Português"), QStringLiteral("Portuguese"), QStringLiteral(":/icons/flags/pt_BR.png")},
        {QStringLiteral("de_DE"), QStringLiteral("Deutsch"), QStringLiteral("German"), QStringLiteral(":/icons/flags/de_DE.png")},
    };
    return kTable;
}

bool isSupportedCode(const QString &code)
{
    for (const auto &e : localeTable())
    {
        if (e.code == code)
        {
            return true;
        }
    }
    return false;
}

// Accept loose aliases returned by QLocale::uiLanguages() such as "en",
// "en-GB", "zh-Hans-CN", "pt-BR". Match against our supported list by
// progressively relaxing: exact → primary-language → language+script
// fallback.
QString bestSystemMatch()
{
    const QStringList ui = QLocale::system().uiLanguages();
    for (const QString &raw : ui)
    {
        // Normalise "en-GB" → "en_GB".
        const QString norm = QString(raw).replace('-', '_');
        if (isSupportedCode(norm))
        {
            return norm;
        }
        // Language-only codes: "tr" → first supported locale starting with "tr_".
        const QString lang = norm.section('_', 0, 0);
        if (lang == QStringLiteral("en"))
        {
            return QStringLiteral("en");
        }
        for (const auto &e : localeTable())
        {
            if (e.code.section('_', 0, 0) == lang)
            {
                return e.code;
            }
        }
    }
    return {};
}

QTranslator *g_translator = nullptr; // owned by QApplication once installed
} // namespace

namespace Language
{
const QVector<LocaleEntry> &supported() { return localeTable(); }

QString userOverride()
{
    QSettings settings;
    const QString code = settings.value(kSettingsKey).toString();
    return isSupportedCode(code) ? code : QString{};
}

void setUserOverride(const QString &code)
{
    QSettings settings;
    if (code.isEmpty() || !isSupportedCode(code))
    {
        settings.remove(kSettingsKey);
    }
    else
    {
        settings.setValue(kSettingsKey, code);
    }
}

void clearUserOverride()
{
    QSettings settings;
    settings.remove(kSettingsKey);
}

QString resolveStartupLocale()
{
    const QString override = userOverride();
    if (!override.isEmpty())
    {
        return override;
    }
    const QString sys = bestSystemMatch();
    if (!sys.isEmpty())
    {
        return sys;
    }
    return QStringLiteral("en");
}

bool applyTranslator(QApplication *app, const QString &code)
{
    if (!app)
    {
        return false;
    }
    if (g_translator)
    {
        QApplication::removeTranslator(g_translator);
        delete g_translator;
        g_translator = nullptr;
    }

    QLocale::setDefault(QLocale(code));

    // English is the source language — no .qm needed.
    if (code == QStringLiteral("en"))
    {
        return true;
    }

    auto *t = new QTranslator(app);
    const QString base = QStringLiteral("QMineSweeper_%1").arg(code);
    if (!t->load(base, QStringLiteral(":/i18n")))
    {
        delete t;
        return false;
    }
    QApplication::installTranslator(t);
    g_translator = t;
    return true;
}
} // namespace Language
