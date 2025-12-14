#include "HandwritingService.hpp"

#include <QDateTime>
#include <utility>

namespace handnote::services {

HandwritingService::HandwritingService(QObject *parent)
    : QObject(parent)
{
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(750);
    connect(&m_debounceTimer, &QTimer::timeout, this, [this]() {
        if (m_liveCallback) {
            emitPlaceholderText(m_liveCallback);
            m_liveCallback = nullptr;
        }
    });
}

void HandwritingService::setAutoConversionEnabled(bool enabled)
{
    m_autoConversionEnabled = enabled;
}

bool HandwritingService::isAutoConversionEnabled() const
{
    return m_autoConversionEnabled;
}

void HandwritingService::setDebounceMs(int ms)
{
    m_debounceTimer.setInterval(ms);
}

void HandwritingService::requestLiveConversion(const QImage &/*image*/,
                                               std::function<void(QString)> onTextReady)
{
    if (!m_autoConversionEnabled) {
        return;
    }

    // Debounce so we don't spam recognition engines while drawing quickly.
    m_liveCallback = std::move(onTextReady);
    m_debounceTimer.start();
}

void HandwritingService::convertSelection(const QImage &/*image*/,
                                          std::function<void(QString)> onTextReady)
{
    emitPlaceholderText(std::move(onTextReady));
}

void HandwritingService::emitPlaceholderText(std::function<void(QString)> callback) const
{
    if (!callback)
        return;

    // Placeholder output; swap with real recognizer later.
    const QString synthetic = QStringLiteral("[handwriting] %1")
                                  .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
    callback(synthetic);
}

} // namespace handnote::services
