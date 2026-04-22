#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <QString>
#include <QVariantMap>

// Thin wrapper around sentry-native. Builds as a no-op when sentry is
// not compiled in (QMS_SENTRY_ENABLED unset). All functions are safe to
// call unconditionally.
namespace Telemetry
{
// Whether the SDK was linked at build time.
[[nodiscard]] bool isCompiledIn() noexcept;

// User opt-in state (persisted in QSettings under "telemetry/enabled").
[[nodiscard]] bool isEnabled();
void setEnabled(bool enabled, const QString &release);

// Whether we have already asked the user about telemetry.
[[nodiscard]] bool hasAskedConsent();
void setHasAskedConsent(bool asked);

// Initialize sentry-native iff the SDK is compiled in and the user opted in.
// Idempotent; safe to call more than once. Pass the application version string.
void initialize(const QString &release);

// Flush pending events and release the SDK. Must be called before the
// process exits — hook into QCoreApplication::aboutToQuit.
void shutdown();

// Record a custom usage event (tagged with the given properties).
void recordEvent(const QString &name, const QVariantMap &properties = {});

// Add a breadcrumb that annotates the next captured event / crash.
void addBreadcrumb(const QString &category, const QString &message);
} // namespace Telemetry

#endif // TELEMETRY_H
