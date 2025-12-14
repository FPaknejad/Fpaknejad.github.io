#include "JsonSerializer.hpp"
#include <QJsonArray>
#include <QJsonValue>

namespace handnote::core {

// Small helpers – avoid QUuid = QString
static QUuid uuidFromString(const QString &str)
{
    return QUuid::fromString(str);
}

static QString uuidToString(const QUuid &id)
{
    // WithoutBraces looks nicer, but you can change if needed
    return id.toString(QUuid::WithoutBraces);
}

// ---------------------------------------------------------
// StrokePoint
// ---------------------------------------------------------

QJsonObject toJson(const StrokePoint &p)
{
    QJsonObject obj;
    obj["x"] = p.x;
    obj["y"] = p.y;
    obj["pressure"] = p.pressure;
    return obj;
}

StrokePoint strokePointFromJson(const QJsonObject &obj)
{
    StrokePoint p;
    p.x = static_cast<float>(obj["x"].toDouble());
    p.y = static_cast<float>(obj["y"].toDouble());
    p.pressure = static_cast<float>(obj["pressure"].toDouble(1.0));
    return p;
}

// ---------------------------------------------------------
// Stroke
// ---------------------------------------------------------

QJsonObject toJson(const Stroke &stroke)
{
    QJsonObject obj;
    obj["id"] = uuidToString(stroke.id);

    QJsonArray pointsArray;
    //pointsArray.reserve(stroke.points.size());
    for (const auto &pt : stroke.points)
        pointsArray.append(toJson(pt));

    obj["points"] = pointsArray;
    return obj;
}

Stroke strokeFromJson(const QJsonObject &obj)
{
    Stroke stroke;
    stroke.id = uuidFromString(obj["id"].toString());

    const QJsonArray pointsArray = obj["points"].toArray();
    stroke.points.reserve(pointsArray.size());
    for (const auto &value : pointsArray) {
        stroke.points.append(strokePointFromJson(value.toObject()));
    }

    return stroke;
}

// ---------------------------------------------------------
// Text block
// ---------------------------------------------------------

QJsonObject toJson(const TextBlock &block)
{
    QJsonObject obj;
    obj["id"] = uuidToString(block.id);
    obj["text"] = block.text;
    obj["lineIndex"] = block.lineIndex;
    obj["isHandwritten"] = block.isHandwritten;

    QJsonObject bbox;
    bbox["x"] = block.boundingBox.x();
    bbox["y"] = block.boundingBox.y();
    bbox["w"] = block.boundingBox.width();
    bbox["h"] = block.boundingBox.height();
    obj["boundingBox"] = bbox;

    return obj;
}

TextBlock textBlockFromJson(const QJsonObject &obj)
{
    TextBlock block;
    block.id = uuidFromString(obj["id"].toString());
    block.text = obj["text"].toString();
    block.lineIndex = obj["lineIndex"].toInt(0);
    block.isHandwritten = obj["isHandwritten"].toBool(false);

    const QJsonObject bbox = obj["boundingBox"].toObject();
    block.boundingBox = QRectF(
        bbox["x"].toDouble(),
        bbox["y"].toDouble(),
        bbox["w"].toDouble(),
        bbox["h"].toDouble()
    );

    return block;
}

// ---------------------------------------------------------
// Page template
// ---------------------------------------------------------

QJsonObject toJson(const PageTemplate &pageTemplate)
{
    QJsonObject obj;
    obj["id"] = pageTemplate.id;
    obj["name"] = pageTemplate.name;
    obj["lineHeightPx"] = pageTemplate.lineHeightPx;
    obj["showMargin"] = pageTemplate.showMargin;
    return obj;
}

PageTemplate pageTemplateFromJson(const QJsonObject &obj)
{
    PageTemplate pageTemplate;
    if (obj.isEmpty())
        return pageTemplate;

    pageTemplate.id = obj["id"].toString(pageTemplate.id);
    pageTemplate.name = obj["name"].toString(pageTemplate.name);
    pageTemplate.lineHeightPx = obj["lineHeightPx"].toInt(pageTemplate.lineHeightPx);
    pageTemplate.showMargin = obj["showMargin"].toBool(pageTemplate.showMargin);
    return pageTemplate;
}

// ---------------------------------------------------------
// Handwriting config
// ---------------------------------------------------------

QJsonObject toJson(const HandwritingConfig &cfg)
{
    QJsonObject obj;
    obj["autoConvert"] = cfg.autoConvert;
    obj["debounceMs"] = cfg.debounceMs;
    obj["snapToTemplateLines"] = cfg.snapToTemplateLines;
    return obj;
}

HandwritingConfig handwritingConfigFromJson(const QJsonObject &obj)
{
    HandwritingConfig cfg;
    if (obj.isEmpty())
        return cfg;

    cfg.autoConvert = obj["autoConvert"].toBool(cfg.autoConvert);
    cfg.debounceMs = obj["debounceMs"].toInt(cfg.debounceMs);
    cfg.snapToTemplateLines = obj["snapToTemplateLines"].toBool(cfg.snapToTemplateLines);
    return cfg;
}

// ---------------------------------------------------------
// Page
// ---------------------------------------------------------

QJsonObject toJson(const Page &page)
{
    QJsonObject obj;
    obj["id"] = uuidToString(page.id);
    obj["title"] = page.title;

    QJsonArray textBlocksArray;
    for (const auto &tb : page.textBlocks)
        textBlocksArray.append(toJson(tb));

    QJsonArray strokesArray;
    //strokesArray.reserve(page.strokes.size());
    for (const auto &s : page.strokes)
        strokesArray.append(toJson(s));

    obj["textBlocks"] = textBlocksArray;
    obj["strokes"] = strokesArray;
    obj["template"] = toJson(page.templateInfo);
    obj["handwriting"] = toJson(page.handwriting);
    return obj;
}

Page pageFromJson(const QJsonObject &obj)
{
    Page page;
    page.id = uuidFromString(obj["id"].toString());
    page.title = obj["title"].toString();

    const QJsonArray textBlocksArray = obj["textBlocks"].toArray();
    page.textBlocks.reserve(textBlocksArray.size());
    for (const auto &value : textBlocksArray) {
        page.textBlocks.append(textBlockFromJson(value.toObject()));
    }

    const QJsonArray strokesArray = obj["strokes"].toArray();
    page.strokes.reserve(strokesArray.size());
    for (const auto &value : strokesArray) {
        page.strokes.append(strokeFromJson(value.toObject()));
    }

    if (obj.contains("template"))
        page.templateInfo = pageTemplateFromJson(obj["template"].toObject());

    if (obj.contains("handwriting"))
        page.handwriting = handwritingConfigFromJson(obj["handwriting"].toObject());

    return page;
}

// ---------------------------------------------------------
// Section
// ---------------------------------------------------------

QJsonObject toJson(const Section &section)
{
    QJsonObject obj;
    obj["id"] = uuidToString(section.id);
    obj["title"] = section.title;

    QJsonArray pagesArray;
    //pagesArray.reserve(section.pages.size());
    for (const auto &p : section.pages)
        pagesArray.append(toJson(p));

    obj["pages"] = pagesArray;
    return obj;
}

Section sectionFromJson(const QJsonObject &obj)
{
    Section section;
    section.id = uuidFromString(obj["id"].toString());
    section.title = obj["title"].toString();

    const QJsonArray pagesArray = obj["pages"].toArray();
    section.pages.reserve(pagesArray.size());
    for (const auto &value : pagesArray) {
        section.pages.append(pageFromJson(value.toObject()));
    }

    return section;
}

// ---------------------------------------------------------
// Notebook
// ---------------------------------------------------------

QJsonObject notebookToJson(const Notebook &notebook)
{
    QJsonObject obj;
    obj["id"] = uuidToString(notebook.id);
    obj["name"] = notebook.name;

    QJsonArray sectionsArray;
    //sectionsArray.reserve(notebook.sections.size());
    for (const auto &s : notebook.sections)
        sectionsArray.append(toJson(s));

    obj["sections"] = sectionsArray;
    return obj;
}

Notebook notebookFromJson(const QJsonObject &obj)
{
    Notebook notebook;
    notebook.id = uuidFromString(obj["id"].toString());
    notebook.name = obj["name"].toString();

    const QJsonArray sectionsArray = obj["sections"].toArray();
    notebook.sections.reserve(sectionsArray.size());
    for (const auto &value : sectionsArray) {
        notebook.sections.append(sectionFromJson(value.toObject()));
    }

    return notebook;
}

} // namespace handnote::core
