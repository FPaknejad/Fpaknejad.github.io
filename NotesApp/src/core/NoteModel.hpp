#pragma once
#include <QString>
#include <QVector>
#include <QUuid>
#include <QRectF>
#include <QJsonObject>
#include <QJsonArray>

namespace handnote::core {

// ---------------------------------------------
// Helper: convert QUuid <-> string
// ---------------------------------------------
inline QString uuidToString(const QUuid &id) { return id.toString(QUuid::WithoutBraces); }
inline QUuid uuidFromString(const QString &s) { return QUuid::fromString("{" + s + "}"); }

// ---------------------------------------------
// Basic stroke structures
// ---------------------------------------------
struct StrokePoint {
    float x;
    float y;
    float pressure;
    qint64 timestamp;
};

struct Stroke {
    QUuid id;
    QVector<StrokePoint> points;
};

// ---------------------------------------------
// Text block for converted handwriting
// ---------------------------------------------
struct TextBlock {
    QUuid id;
    QString text;
    QRectF boundingBox;
    int lineIndex = 0;
};

// ---------------------------------------------
// Page template (ruled, blank, grid…)
// ---------------------------------------------
struct PageTemplate {
    enum class Type { Blank, Lined, Grid };
    Type type = Type::Lined;
    float topMargin = 50.0f;
    float lineSpacing = 20.0f;
};

// ---------------------------------------------
// Page object
// ---------------------------------------------
struct Page {
    QUuid id;
    QString title;
    PageTemplate tmpl;
    QVector<Stroke> strokes;
    QVector<TextBlock> textBlocks;
};

// ---------------------------------------------
// Section object (holds pages)
// ---------------------------------------------
struct Section {
    QUuid id;
    QString name;
    QVector<Page> pages;
};

// ---------------------------------------------
// Notebook object (holds sections)
// ---------------------------------------------
struct Notebook {
    QUuid id;
    QString name;
    QVector<Section> sections;
};

// ---------------------------------------------
// JSON conversion functions
// ---------------------------------------------
QJsonObject toJson(const StrokePoint &p);
StrokePoint strokePointFromJson(const QJsonObject &obj);

QJsonObject toJson(const Stroke &s);
Stroke strokeFromJson(const QJsonObject &obj);

QJsonObject toJson(const TextBlock &t);
TextBlock textBlockFromJson(const QJsonObject &obj);

QJsonObject toJson(const PageTemplate &t);
PageTemplate templateFromJson(const QJsonObject &obj);

QJsonObject toJson(const Page &p);
Page pageFromJson(const QJsonObject &obj);

QJsonObject toJson(const Section &s);
Section sectionFromJson(const QJsonObject &obj);

QJsonObject toJson(const Notebook &n);
Notebook notebookFromJson(const QJsonObject &obj);

} // namespace handnote::core
