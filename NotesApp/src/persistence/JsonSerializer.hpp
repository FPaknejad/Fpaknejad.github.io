// JsonSerializer.hpp  (for reference – adjust yours to match)
#pragma once

#include <QJsonObject>
#include <QJsonArray>
#include "NoteTypes.hpp"

namespace handnote::core {

QJsonObject toJson(const StrokePoint& p);
StrokePoint strokePointFromJson(const QJsonObject& obj);

QJsonObject toJson(const Stroke& stroke);
Stroke strokeFromJson(const QJsonObject& obj);

QJsonObject toJson(const TextBlock& block);
TextBlock textBlockFromJson(const QJsonObject& obj);

QJsonObject toJson(const PageTemplate& pageTemplate);
PageTemplate pageTemplateFromJson(const QJsonObject& obj);

QJsonObject toJson(const HandwritingConfig& cfg);
HandwritingConfig handwritingConfigFromJson(const QJsonObject& obj);

QJsonObject toJson(const Page& page);
Page pageFromJson(const QJsonObject& obj);

QJsonObject toJson(const Section& section);
Section sectionFromJson(const QJsonObject& obj);

QJsonObject notebookToJson(const Notebook& notebook);
Notebook notebookFromJson(const QJsonObject& obj);

} // namespace handnote::core
