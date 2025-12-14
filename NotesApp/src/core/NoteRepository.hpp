#pragma once
#include <QObject>
#include <QString>

namespace handnote::core {

struct Notebook;

class NoteRepository : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual Notebook loadAll() = 0;
    virtual void saveAll(const Notebook& notebook) = 0;

    virtual Notebook loadNotebook(const QString& path) = 0;
    virtual void saveNotebook(const QString& path, const Notebook& notebook) = 0;

    virtual ~NoteRepository() = default;
};

} // namespace handnote::core
