#include "LocalFileNoteRepository.hpp"
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace handnote::core;

LocalFileNoteRepository::LocalFileNoteRepository(const QString& filePath, QObject* parent)
    : NoteRepository(parent), m_filePath(filePath) {}

LocalFileNoteRepository::LocalFileNoteRepository(QObject* parent)
    : NoteRepository(parent) {}

Notebook LocalFileNoteRepository::loadAll()
{
    return loadNotebook(m_filePath);
}

void LocalFileNoteRepository::saveAll(const Notebook& notebook)
{
    saveNotebook(m_filePath, notebook);
}

// Helper
Notebook LocalFileNoteRepository::loadNotebook(const QString& path)
{
    Notebook notebook;
    QFile file(path);

    if (path.isEmpty())
        return notebook;

    if (!file.exists())
        return notebook;

    if (!file.open(QIODevice::ReadOnly))
        return notebook;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return notebook;

    notebook = notebookFromJson(doc.object());
    return notebook; // return the deserialized notebook
}

void LocalFileNoteRepository::saveNotebook(const QString& path,
                                           const Notebook& notebook)
{
    if (path.isEmpty())
        return;

    QSaveFile file(path);

    if (!file.open(QIODevice::WriteOnly))
        return;

    QJsonDocument doc(notebookToJson(notebook));
    file.write(doc.toJson(QJsonDocument::Compact));
    file.commit();
}
