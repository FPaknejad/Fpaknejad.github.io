#pragma once

#include <QObject>
#include <memory>
#include <vector>
#include <QTimer>
#include <QString>

#include "core/NoteRepository.hpp"
#include "domain/NoteModel.hpp"
#include "services/TreeSyncService.hpp"

namespace handnote::services {

// Coordinates the domain model, persistence layer, and (future) cloud sync.
// All mutations flow through here so we can centralize autosave, throttling,
// and feature toggles (auth, cloud, handwriting).
class NoteService : public QObject
{
    Q_OBJECT
public:
    NoteService(handnote::core::NoteModel& model,
                std::unique_ptr<handnote::core::NoteRepository> repository,
                std::shared_ptr<TreeSyncService> syncService,
                QObject* parent = nullptr);

    void load();
    void persistNow();

    QVector<handnote::core::Notebook> &notebooks();
    const QVector<handnote::core::Notebook> &notebooks() const;

    QString createNotebook(const QString &name);
    bool renameNotebook(const QString &id, const QString &newName);
    bool deleteNotebook(const QString &id);

    QString createSection(const QString &notebookId, const QString &title);
    bool renameSection(const QString &id, const QString &newTitle);
    bool deleteSection(const QString &id);
    bool moveSection(const QString &sectionId, const QString &destNotebookId, int destIndex);

    QString createPage(const QString &sectionId, const QString &title,
                       const handnote::core::PageTemplate &templateInfo = {});
    bool renamePage(const QString &id, const QString &newTitle);
    bool deletePage(const QString &id);
    bool movePage(const QString &pageId, const QString &destSectionId, int destIndex);
    bool updatePageText(const QString &id, const QString &text);
    bool replacePageStrokes(const QString &id, const QVector<handnote::core::Stroke> &strokes);
    bool updatePageTemplate(const QString &id, const handnote::core::PageTemplate &templateInfo);
    bool updateHandwritingConfig(const QString &id, const handnote::core::HandwritingConfig &cfg);

    void enableCloudSync(bool enabled);
    void requestSync();

signals:
    void persisted();

private:
    handnote::core::NoteModel& m_model;
    std::unique_ptr<handnote::core::NoteRepository> m_repository;
    std::shared_ptr<TreeSyncService> m_syncService;
    bool m_cloudSyncEnabled {false};
    bool m_dirty {false};
    QTimer m_saveTimer;

    void markDirty();
    bool hasNotebookData() const;
};

} // namespace handnote::services
