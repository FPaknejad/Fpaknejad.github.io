#include "LocalFileNoteRepository.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

namespace handnote::core {

LocalFileNoteRepository::LocalFileNoteRepository(QString filePath,
                                                 QObject *parent)
    : NoteRepository(parent),
      m_filePath(std::move(filePath))
{
}

QList<Notebook> LocalFileNoteRepository::loadAll()
{
    QList<Notebook> notebooks;

    QFile file(m_filePath);
    if (!file.exists()) {
        return notebooks; // empty, first run
    }

    if (!file.open(QIODevice::ReadOnly)) {
        // could log error later
        return notebooks;
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return notebooks;
    }

    const QJsonObject root = doc.object();
    const QJsonArray arr = root["notebooks"].toArray();

    notebooks.reserve(arr.size());
    for (const auto &v : arr) {
        notebooks.append(notebookFromJson(v.toObject()));
    }

    return notebooks;
}

bool LocalFileNoteRepository::saveAll(const QList<Notebook> &notebooks)
{
    QJsonArray arr;
    // QJsonArray does not provide reserve(); it grows dynamically as elements are appended.
    // arr.reserve(notebooks.size());
    for (const auto &n : notebooks) {
        arr.append(toJson(n));
    }

    QJsonObject root;
    root["notebooks"] = arr;

    const QJsonDocument doc(root);

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

} // namespace handnote::core
