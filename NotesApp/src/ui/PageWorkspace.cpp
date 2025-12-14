#include "PageWorkspace.hpp"

#include <QSignalBlocker>

using namespace handnote::core;

PageWorkspace::PageWorkspace(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setPlaceholderText("Type your note here...");

    m_canvas = new HandwritingCanvas(this);
    m_canvas->setMinimumHeight(240);

    layout->addWidget(m_textEdit, 3);
    layout->addWidget(m_canvas, 2);

    connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
        if (m_isLoading)
            return;
        emit textEdited(m_textEdit->toPlainText());
    });

    connect(m_canvas, &HandwritingCanvas::strokeFinished, this, [this]() {
        emit strokeFinished(canvasImage());
    });
}

void PageWorkspace::setPage(const Page *page)
{
    m_isLoading = true;

    if (!page) {
        m_textEdit->clear();
        m_canvas->clearCanvas();
        m_isLoading = false;
        return;
    }

    QString text;
    for (const TextBlock &tb : page->textBlocks) {
        if (!text.isEmpty())
            text += "\n\n";
        text += tb.text;
    }

    {
        QSignalBlocker block(m_textEdit);
        m_textEdit->setPlainText(text);
    }

    m_canvas->clearCanvas(); // TODO: restore strokes later
    m_isLoading = false;
}

QImage PageWorkspace::canvasImage() const
{
    return m_canvas->toImage();
}

void PageWorkspace::clearCanvas()
{
    m_canvas->clearCanvas();
}

void PageWorkspace::setPenModeEnabled(bool enabled)
{
    m_canvas->setEnabled(enabled);
}
