#include "telemetry.h"

#include <QSettings>
#include <QSysInfo>
#include <QUuid>

#ifdef QMS_SENTRY_ENABLED
#include <sentry.h>
#endif

namespace
{
constexpr auto kConsentKey = "telemetry/enabled";
constexpr auto kAskedKey = "telemetry/asked";
constexpr auto kUserIdKey = "telemetry/anonymous_id";

bool g_initialized = false;

#ifdef QMS_SENTRY_ENABLED
void applyAnonymousUser()
{
    QSettings settings;
    QString uid = settings.value(kUserIdKey).toString();
    if (uid.isEmpty())
    {
        uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        settings.setValue(kUserIdKey, uid);
    }
    sentry_value_t user = sentry_value_new_object();
    const QByteArray uidBytes = uid.toUtf8();
    sentry_value_set_by_key(user, "id", sentry_value_new_string(uidBytes.constData()));
    sentry_set_user(user);
}
#endif
} // namespace

namespace Telemetry
{
bool isCompiledIn() noexcept
{
#ifdef QMS_SENTRY_ENABLED
    return true;
#else
    return false;
#endif
}

bool isEnabled()
{
    QSettings settings;
    return settings.value(kConsentKey, false).toBool();
}

bool hasAskedConsent()
{
    QSettings settings;
    return settings.value(kAskedKey, false).toBool();
}

void setHasAskedConsent(bool asked)
{
    QSettings settings;
    settings.setValue(kAskedKey, asked);
}

void initialize(const QString &release)
{
#ifdef QMS_SENTRY_ENABLED
    if (g_initialized || !isEnabled())
    {
        return;
    }
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, QMS_SENTRY_DSN);
    const QByteArray releaseBytes = release.toUtf8();
    sentry_options_set_release(options, releaseBytes.constData());
#ifdef QT_DEBUG
    sentry_options_set_environment(options, "debug");
    sentry_options_set_debug(options, 1);
#else
    sentry_options_set_environment(options, "release");
#endif
    sentry_init(options);
    applyAnonymousUser();

    // Platform tags for segmentation. Sentry already captures basic OS info,
    // but explicit tags make event search painless.
    sentry_set_tag("os", QSysInfo::prettyProductName().toUtf8().constData());
    sentry_set_tag("arch", QSysInfo::currentCpuArchitecture().toUtf8().constData());
    sentry_set_tag("qt_version", qVersion());

    // Start a release-health session. Closed by sentry_close() below, which
    // aggregates per-release stability and daily-active-user counts.
    sentry_start_session();

    g_initialized = true;
#else
    Q_UNUSED(release);
#endif
}

void shutdown()
{
#ifdef QMS_SENTRY_ENABLED
    if (g_initialized)
    {
        sentry_close();
        g_initialized = false;
    }
#endif
}

void setEnabled(bool enabled, const QString &release)
{
    QSettings settings;
    settings.setValue(kConsentKey, enabled);
#ifdef QMS_SENTRY_ENABLED
    if (enabled && !g_initialized)
    {
        initialize(release);
    }
    else if (!enabled && g_initialized)
    {
        shutdown();
    }
#else
    Q_UNUSED(release);
#endif
}

void recordEvent(const QString &name, const QVariantMap &properties)
{
#ifdef QMS_SENTRY_ENABLED
    if (!g_initialized)
    {
        return;
    }
    const QByteArray nameBytes = name.toUtf8();
    sentry_value_t event = sentry_value_new_message_event(SENTRY_LEVEL_INFO, "qminesweeper", nameBytes.constData());
    if (!properties.isEmpty())
    {
        sentry_value_t tags = sentry_value_new_object();
        for (auto it = properties.constBegin(); it != properties.constEnd(); ++it)
        {
            const QByteArray keyBytes = it.key().toUtf8();
            const QByteArray valBytes = it.value().toString().toUtf8();
            sentry_value_set_by_key(tags, keyBytes.constData(), sentry_value_new_string(valBytes.constData()));
        }
        sentry_value_set_by_key(event, "tags", tags);
    }
    sentry_capture_event(event);
#else
    Q_UNUSED(name);
    Q_UNUSED(properties);
#endif
}

void addBreadcrumb(const QString &category, const QString &message)
{
#ifdef QMS_SENTRY_ENABLED
    if (!g_initialized)
    {
        return;
    }
    const QByteArray messageBytes = message.toUtf8();
    const QByteArray categoryBytes = category.toUtf8();
    sentry_value_t crumb = sentry_value_new_breadcrumb(categoryBytes.constData(), messageBytes.constData());
    sentry_add_breadcrumb(crumb);
#else
    Q_UNUSED(category);
    Q_UNUSED(message);
#endif
}
} // namespace Telemetry
