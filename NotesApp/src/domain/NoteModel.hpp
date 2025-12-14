#pragma once

#include <QObject>
#include <QString>
#include <QVector>

#include "NoteTypes.hpp"

namespace handnote::core {

class NoteModel : public QObject
{
    Q_OBJECT

public:
    explicit NoteModel(QObject *parent = nullptr);

    QVector<Notebook> &notebooks();
    const QVector<Notebook> &notebooks() const;

    Notebook *findNotebookById(const QString &id);
    const Notebook *findNotebookById(const QString &id) const;
    Section *findSectionById(const QString &id);
    Page *findPageById(const QString &id);

    QString createNotebook(const QString &name);
    bool renameNotebook(const QString &id, const QString &newName);
    bool deleteNotebook(const QString &id);

    QString createSection(const QString &notebookId, const QString &title);
    bool renameSection(const QString &id, const QString &newTitle);
    bool deleteSection(const QString &id);
    bool moveSection(const QString &sectionId, const QString &destNotebookId, int destIndex);

    QString createPage(const QString &sectionId, const QString &title,
                       const PageTemplate &templateInfo = {});
    bool renamePage(const QString &id, const QString &newTitle);
    bool deletePage(const QString &id);
    bool movePage(const QString &pageId, const QString &destSectionId, int destIndex);
    bool updatePageText(const QString &id, const QString &text);
    bool replacePageStrokes(const QString &id, const QVector<Stroke> &strokes);
    bool setPageTemplate(const QString &id, const PageTemplate &templateInfo);
    bool setHandwritingConfig(const QString &id, const HandwritingConfig &cfg);

    QString notebookName(const QString &id) const;
    QString sectionTitle(const QString &id) const;
    QString pageTitle(const QString &id) const;

signals:
    void notebookListChanged();
    void notebookStructureChanged();

private:
    QVector<Notebook> m_notebooks;
};

} // namespace handnote::core
