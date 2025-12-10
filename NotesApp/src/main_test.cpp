#include <QApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

#include "MainWindow.hpp"
#include "core/LocalFileNoteRepository.hpp"
#include "core/NoteModel.hpp"

using namespace handnote::core;

// ---------------------------------------------------
// Optional: JSON test wrapped into a function
// ---------------------------------------------------
void runJsonTest()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString jsonPath = dataDir + "/notes_test.json";

    LocalFileNoteRepository repo(jsonPath);

    Notebook nb;
    nb.id = QUuid::createUuid();
    nb.name = "Test Notebook";

    Section sec;
    sec.id = QUuid::createUuid();
    sec.name = "My Section";

    Page page;
    page.id = QUuid::createUuid();
    page.title = "My Test Page";

    page.tmpl.type = PageTemplate::Type::Lined;
    page.tmpl.topMargin = 50.0f;
    page.tmpl.lineSpacing = 20.0f;

    Stroke s;
    s.id = QUuid::createUuid();
    s.points.append({10, 20, 0.8f, 123});
    s.points.append({15, 25, 0.9f, 133});
    page.strokes.append(s);

    TextBlock tb;
    tb.id = QUuid::createUuid();
    tb.text = "Hello world";
    tb.boundingBox = QRectF(100, 150, 200, 40);
    tb.lineIndex = 3;
    page.textBlocks.append(tb);

    sec.pages.append(page);
    nb.sections.append(sec);

    QList<Notebook> notebooks;
    notebooks.append(nb);

    qDebug() << "Saving test JSON to:" << jsonPath;
    repo.saveAll(notebooks);

    auto loaded = repo.loadAll();
    qDebug() << "Loaded" << loaded.size() << "notebooks.";
    if (!loaded.isEmpty()) {
        auto &n = loaded.first();
        qDebug() << "Notebook name:" << n.name;
        qDebug() << "Sections:" << n.sections.size();
        qDebug() << "Pages in first section:" << n.sections.first().pages.size();
        const auto &p2 = n.sections.first().pages.first();
        qDebug() << "Linespacing:" << p2.tmpl.lineSpacing;
        qDebug() << "Strokes:" << p2.strokes.size();
        qDebug() << "TextBlocks:" << p2.textBlocks.size();
    }

    qDebug() << "\nJSON TEST COMPLETE — no crash means model is good!\n";
}

// ---------------------------------------------------
// REAL APPLICATION STARTS HERE
// ---------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Run the test (non-blocking)
    runJsonTest();

    // Now start the UI
    MainWindow w;
    w.show();

    return app.exec();
}
