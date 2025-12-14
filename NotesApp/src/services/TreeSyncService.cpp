#include "TreeSyncService.hpp"

namespace handnote::services {

TreeSyncService::TreeSyncService(QObject *parent)
    : QObject(parent)
{
}

void TreeSyncService::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool TreeSyncService::isEnabled() const
{
    return m_enabled;
}

void TreeSyncService::scheduleUpload(const QVector<handnote::core::Notebook> &/*notebooks*/)
{
    if (!m_enabled)
        return;

    m_pending = true;
    emit syncStarted();

    // Placeholder: real implementation would diff and push changes.
    m_pending = false;
    emit syncFinished(true);
}

void TreeSyncService::flush()
{
    if (m_pending)
        emit syncFinished(true);
    m_pending = false;
}

} // namespace handnote::services
