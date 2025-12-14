#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <memory>

#include "NoteRepository.hpp"
#include "NoteModel.hpp"
#include "JsonSerializer.hpp"


namespace handnote::core {

class LocalFileNoteRepository : public NoteRepository {
    Q_OBJECT
public:
    explicit LocalFileNoteRepository(const QString& filePath, QObject* parent = nullptr);
    explicit LocalFileNoteRepository(QObject* parent = nullptr);

    Notebook loadAll() override;
    void saveAll(const Notebook& notebook) override;


    Notebook loadNotebook(const QString& path) override;
    void saveNotebook(const QString& path, const Notebook& notebook) override;
private:
    QString m_filePath;
};


} // namespace handnote::core
