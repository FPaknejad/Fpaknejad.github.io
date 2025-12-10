#pragma once

#include "NoteRepository.hpp"
#include <QString>

namespace handnote::core {

class LocalFileNoteRepository : public NoteRepository
{
    Q_OBJECT
public:
    explicit LocalFileNoteRepository(QString filePath,
                                     QObject *parent = nullptr);

    QList<Notebook> loadAll() override;
    bool saveAll(const QList<Notebook> &notebooks) override;

private:
    QString m_filePath;
};

} // namespace handnote::core
