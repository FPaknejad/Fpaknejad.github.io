#pragma once

#include <QObject>
#include <QVector>

#include "domain/NoteTypes.hpp"

namespace handnote::services {

// Placeholder cloud sync orchestrator.
// Signals give the UI a hook for future progress indicators.
class TreeSyncService : public QObject
{
    Q_OBJECT
public:
    explicit TreeSyncService(QObject *parent = nullptr);

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void scheduleUpload(const QVector<handnote::core::Notebook> &notebooks);
    void flush();

signals:
    void syncStarted();
    void syncFinished(bool success);

private:
    bool m_enabled {false};
    bool m_pending {false};
};

} // namespace handnote::services
