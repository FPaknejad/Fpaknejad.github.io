#include "NoteService.hpp"

using namespace handnote::core;

namespace handnote::services {

namespace {
    bool isNotebookEmpty(const Notebook &nb)
    {
        return nb.id.isNull() && nb.name.isEmpty() && nb.sections.isEmpty();
    }
}

NoteService::NoteService(NoteModel &model,
                         std::unique_ptr<NoteRepository> repository,
                         std::shared_ptr<TreeSyncService> syncService,
                         QObject *parent)
    : QObject(parent),
      m_model(model),
      m_repository(std::move(repository)),
      m_syncService(std::move(syncService))
{
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(1200);
    connect(&m_saveTimer, &QTimer::timeout, this, &NoteService::persistNow);
}

void NoteService::load()
{
    if (!m_repository)
        return;

    m_model.notebooks().clear();
    const Notebook loaded = m_repository->loadAll();
    if (!isNotebookEmpty(loaded)) {
        m_model.notebooks().append(loaded);
    }
}

void NoteService::persistNow()
{
    m_saveTimer.stop();

    if (!m_repository || !hasNotebookData())
        return;

    m_repository->saveAll(m_model.notebooks().first());
    m_dirty = false;

    if (m_cloudSyncEnabled && m_syncService) {
        m_syncService->scheduleUpload(m_model.notebooks());
    }

    emit persisted();
}

QVector<Notebook> &NoteService::notebooks()
{
    return m_model.notebooks();
}

const QVector<Notebook> &NoteService::notebooks() const
{
    return m_model.notebooks();
}

QString NoteService::createNotebook(const QString &name)
{
    const QString id = m_model.createNotebook(name);
    markDirty();
    return id;
}

bool NoteService::renameNotebook(const QString &id, const QString &newName)
{
    const bool ok = m_model.renameNotebook(id, newName);
    if (ok)
        markDirty();
    return ok;
}

bool NoteService::deleteNotebook(const QString &id)
{
    const bool ok = m_model.deleteNotebook(id);
    if (ok)
        markDirty();
    return ok;
}

QString NoteService::createSection(const QString &notebookId, const QString &title)
{
    const QString id = m_model.createSection(notebookId, title);
    markDirty();
    return id;
}

bool NoteService::renameSection(const QString &id, const QString &newTitle)
{
    const bool ok = m_model.renameSection(id, newTitle);
    if (ok)
        markDirty();
    return ok;
}

bool NoteService::deleteSection(const QString &id)
{
    const bool ok = m_model.deleteSection(id);
    if (ok)
        markDirty();
    return ok;
}

bool NoteService::moveSection(const QString &sectionId, const QString &destNotebookId, int destIndex)
{
    const bool ok = m_model.moveSection(sectionId, destNotebookId, destIndex);
    if (ok)
        markDirty();
    return ok;
}

QString NoteService::createPage(const QString &sectionId, const QString &title,
                                const PageTemplate &templateInfo)
{
    const QString id = m_model.createPage(sectionId, title, templateInfo);
    markDirty();
    return id;
}

bool NoteService::renamePage(const QString &id, const QString &newTitle)
{
    const bool ok = m_model.renamePage(id, newTitle);
    if (ok)
        markDirty();
    return ok;
}

bool NoteService::deletePage(const QString &id)
{
    const bool ok = m_model.deletePage(id);
    if (ok)
        markDirty();
    return ok;
}

bool NoteService::movePage(const QString &pageId, const QString &destSectionId, int destIndex)
{
    const bool ok = m_model.movePage(pageId, destSectionId, destIndex);
    if (ok)
        markDirty();
    return ok;
}

bool NoteService::updatePageText(const QString &id, const QString &text)
{
    const bool ok = m_model.updatePageText(id, text);
    if (ok)
        markDirty();
    return ok;
}

bool NoteService::replacePageStrokes(const QString &id, const QVector<Stroke> &strokes)
{
    const bool ok = m_model.replacePageStrokes(id, strokes);
    if (ok)
        markDirty();
    return ok;
}

bool NoteService::updatePageTemplate(const QString &id, const PageTemplate &templateInfo)
{
    const bool ok = m_model.setPageTemplate(id, templateInfo);
    if (ok)
        markDirty();
    return ok;
}

bool NoteService::updateHandwritingConfig(const QString &id, const HandwritingConfig &cfg)
{
    const bool ok = m_model.setHandwritingConfig(id, cfg);
    if (ok)
        markDirty();
    return ok;
}

void NoteService::enableCloudSync(bool enabled)
{
    m_cloudSyncEnabled = enabled;
}

void NoteService::requestSync()
{
    if (!m_cloudSyncEnabled || !m_syncService)
        return;
    m_syncService->scheduleUpload(m_model.notebooks());
}

void NoteService::markDirty()
{
    m_dirty = true;
    m_saveTimer.start();
}

bool NoteService::hasNotebookData() const
{
    return !m_model.notebooks().isEmpty() && !isNotebookEmpty(m_model.notebooks().first());
}

} // namespace handnote::services
