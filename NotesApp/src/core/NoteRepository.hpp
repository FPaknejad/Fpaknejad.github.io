#pragma once

#include <QObject>
#include <QList>
#include "NoteModel.hpp"

namespace handnote::core {

class NoteRepository : public QObject
{
    Q_OBJECT
public:
    explicit NoteRepository(QObject *parent = nullptr)
        : QObject(parent)
    {}

    ~NoteRepository() override = default;

    virtual QList<Notebook> loadAll() = 0;
    virtual bool saveAll(const QList<Notebook> &notebooks) = 0;
};

} // namespace handnote::core
