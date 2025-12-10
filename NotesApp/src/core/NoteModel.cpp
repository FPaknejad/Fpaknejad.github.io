#include "NoteModel.hpp"
#include <QJsonObject>
#include <QJsonArray>

namespace handnote::core {

// --------------------------------------
// StrokePoint
// --------------------------------------
QJsonObject toJson(const StrokePoint &p)
{
    QJsonObject obj;
    obj["x"] = p.x;
    obj["y"] = p.y;
    obj["pressure"] = p.pressure;
    obj["timestamp"] = QString::number(p.timestamp);
    return obj;
}

StrokePoint strokePointFromJson(const QJsonObject &obj)
{
    StrokePoint p;
    p.x = obj["x"].toDouble();
    p.y = obj["y"].toDouble();
    p.pressure = static_cast<float>(obj["pressure"].toDouble());
    p.timestamp = obj["timestamp"].toString().toLongLong();
    return p;
}


// --------------------------------------
// Stroke
// --------------------------------------
QJsonObject toJson(const Stroke &s)
{
    QJsonObject obj;
    obj["id"] = uuidToString(s.id);

    QJsonArray arr;
    for (const auto &p : s.points)
        arr.append(toJson(p));

    obj["points"] = arr;
    return obj;
}

Stroke strokeFromJson(const QJsonObject &obj)
{
    Stroke s;
    s.id = uuidFromString(obj["id"].toString());

    QJsonArray arr = obj["points"].toArray();
    s.points.reserve(arr.size());
    for (const auto &v : arr)
        s.points.append(strokePointFromJson(v.toObject()));

    return s;
}


// --------------------------------------
// TextBlock
// --------------------------------------
QJsonObject toJson(const TextBlock &t)
{
    QJsonObject obj;
    obj["id"] = uuidToString(t.id);
    obj["text"] = t.text;

    QJsonObject box;
    box["x"] = t.boundingBox.x();
    box["y"] = t.boundingBox.y();
    box["w"] = t.boundingBox.width();
    box["h"] = t.boundingBox.height();
    obj["box"] = box;

    obj["lineIndex"] = t.lineIndex;
    return obj;
}

TextBlock textBlockFromJson(const QJsonObject &obj)
{
    TextBlock t;
    t.id = uuidFromString(obj["id"].toString());
    t.text = obj["text"].toString();

    auto box = obj["box"].toObject();
    t.boundingBox = QRectF(
        box["x"].toDouble(),
        box["y"].toDouble(),
        box["w"].toDouble(),
        box["h"].toDouble()
    );

    t.lineIndex = obj["lineIndex"].toInt();
    return t;
}


// --------------------------------------
// PageTemplate
// --------------------------------------
QJsonObject toJson(const PageTemplate &t)
{
    QJsonObject obj;
    obj["type"] = static_cast<int>(t.type);
    obj["topMargin"] = t.topMargin;
    obj["lineSpacing"] = t.lineSpacing;
    return obj;
}

PageTemplate templateFromJson(const QJsonObject &obj)
{
    PageTemplate t;
    t.type = static_cast<PageTemplate::Type>(obj["type"].toInt());
    t.topMargin = static_cast<float>(obj["topMargin"].toDouble());
    t.lineSpacing = static_cast<float>(obj["lineSpacing"].toDouble());
    return t;
}


// --------------------------------------
// Page
// --------------------------------------
QJsonObject toJson(const Page &p)
{
    QJsonObject obj;
    obj["id"] = uuidToString(p.id);
    obj["title"] = p.title;
    obj["template"] = toJson(p.tmpl);

    QJsonArray strokes;
    for (const auto &s : p.strokes)
        strokes.append(toJson(s));
    obj["strokes"] = strokes;

    QJsonArray texts;
    for (const auto &t : p.textBlocks)
        texts.append(toJson(t));
    obj["textBlocks"] = texts;

    return obj;
}

Page pageFromJson(const QJsonObject &obj)
{
    Page p;
    p.id = uuidFromString(obj["id"].toString());
    p.title = obj["title"].toString();
    p.tmpl = templateFromJson(obj["template"].toObject());

    QJsonArray strokes = obj["strokes"].toArray();
    for (const auto &v : strokes)
        p.strokes.append(strokeFromJson(v.toObject()));

    QJsonArray texts = obj["textBlocks"].toArray();
    for (const auto &v : texts)
        p.textBlocks.append(textBlockFromJson(v.toObject()));

    return p;
}


// --------------------------------------
// Section
// --------------------------------------
QJsonObject toJson(const Section &s)
{
    QJsonObject obj;
    obj["id"] = uuidToString(s.id);
    obj["name"] = s.name;

    QJsonArray pages;
    for (const auto &p : s.pages)
        pages.append(toJson(p));
    obj["pages"] = pages;

    return obj;
}

Section sectionFromJson(const QJsonObject &obj)
{
    Section s;
    s.id = uuidFromString(obj["id"].toString());
    s.name = obj["name"].toString();

    QJsonArray pages = obj["pages"].toArray();
    for (const auto &v : pages)
        s.pages.append(pageFromJson(v.toObject()));

    return s;
}


// --------------------------------------
// Notebook
// --------------------------------------
QJsonObject toJson(const Notebook &n)
{
    QJsonObject obj;
    obj["id"] = uuidToString(n.id);
    obj["name"] = n.name;

    QJsonArray sections;
    for (const auto &s : n.sections)
        sections.append(toJson(s));
    obj["sections"] = sections;

    return obj;
}

Notebook notebookFromJson(const QJsonObject &obj)
{
    Notebook n;
    n.id = uuidFromString(obj["id"].toString());
    n.name = obj["name"].toString();

    QJsonArray sections = obj["sections"].toArray();
    for (const auto &v : sections)
        n.sections.append(sectionFromJson(v.toObject()));

    return n;
}

} // namespace handnote::core
