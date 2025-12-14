#pragma once

#include <QObject>
#include <QObject>
#include <QImage>
#include <QTimer>
#include <QString>
#include <functional>

namespace handnote::services {

// Strategy-like wrapper so we can swap in real handwriting engines later.
class HandwritingService : public QObject
{
    Q_OBJECT
public:
    explicit HandwritingService(QObject *parent = nullptr);

    void setAutoConversionEnabled(bool enabled);
    bool isAutoConversionEnabled() const;

    void setDebounceMs(int ms);

    // Live conversion while the user is writing.
    void requestLiveConversion(const QImage &image,
                               std::function<void(QString)> onTextReady);

    // Manual conversion (select-all / region). For now we treat the whole image.
    void convertSelection(const QImage &image,
                          std::function<void(QString)> onTextReady);

private:
    bool m_autoConversionEnabled {false};
    QTimer m_debounceTimer;
    std::function<void(QString)> m_liveCallback;

    void emitPlaceholderText(std::function<void(QString)> callback) const;
};

} // namespace handnote::services
