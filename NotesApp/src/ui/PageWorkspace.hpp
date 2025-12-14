#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include "HandwritingCanvas.hpp"
#include "domain/NoteTypes.hpp"

class PageWorkspace : public QWidget
{
    Q_OBJECT

public:
    explicit PageWorkspace(QWidget *parent = nullptr);

    void setPage(const handnote::core::Page *page);
    QImage canvasImage() const;
    void clearCanvas();
    void setPenModeEnabled(bool enabled);

signals:
    void textEdited(const QString &text);
    void strokeFinished(const QImage &image);

private:
    QTextEdit *m_textEdit = nullptr;
    HandwritingCanvas *m_canvas = nullptr;
    bool m_isLoading = false;
};
