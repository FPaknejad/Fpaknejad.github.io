#pragma once

#include <QRectF>
#include <QString>
#include <QVector>
#include <QUuid>

namespace handnote::core {

// A single point in a pen stroke
struct StrokePoint
{
    float x {0.0f};
    float y {0.0f};
    float pressure {1.0f};
};

// A continuous stroke drawn on the page
struct Stroke
{
    QUuid id {QUuid::createUuid()};
    QVector<StrokePoint> points;
};

struct TextBlock
{
    QUuid   id {QUuid::createUuid()};
    QString text;
    QRectF  boundingBox;
    int     lineIndex {0};
    bool    isHandwritten {false};
};

// Future page templates (lined, dotted, music sheet, grid, etc.)
struct PageTemplate
{
    QString id {QStringLiteral("lined-default")};
    QString name {QStringLiteral("Lined")};
    int     lineHeightPx {28};
    bool    showMargin {true};
};

// Per-page handwriting preferences to allow quick toggling and
// throttling of conversion so we don't burn CPU/GPU on low-end devices.
struct HandwritingConfig
{
    bool autoConvert {false};
    int  debounceMs {2000};          // throttle recognition (ms)
    bool snapToTemplateLines {true}; // snap converted text to template lines
};

// A single page inside a section.
struct Page
{
    QUuid id;
    QString title;
    QVector<TextBlock> textBlocks;
    QVector<Stroke> strokes;
    PageTemplate templateInfo {};
    HandwritingConfig handwriting {};

    Page() = default;

    explicit Page(const QUuid& id_, const QString& title_ = {})
        : id(id_), title(title_) {}
};

// A logical group of pages in the sidebar.
struct Section
{
    QUuid id;
    QString title;
    QVector<Page> pages;

    Section() = default;

    explicit Section(const QUuid& id_, const QString& title_ = {})
        : id(id_), title(title_) {}
};

// The root notebook (you can support multiple notebooks later).
struct Notebook
{
    QUuid id;
    QString name;
    QVector<Section> sections;

    Notebook() = default;

    explicit Notebook(const QUuid& id_, const QString& name_ = {})
        : id(id_), name(name_) {}
};

} // namespace handnote::core
