#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <memory>

#include "MainWindow.hpp"
#include "core/LocalFileNoteRepository.hpp"
#include "domain/NoteModel.hpp"
#include "services/NoteService.hpp"
#include "services/TreeSyncService.hpp"
#include "services/HandwritingService.hpp"
#include "services/AuthService.hpp"

using namespace handnote::core;
using namespace handnote::services;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // -----------------------------------------------------
    // Setup storage path
    // -----------------------------------------------------
    const QString appDataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir dir(appDataDir);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    const QString notebookPath = dir.filePath(QStringLiteral("notebook.json"));

    // -----------------------------------------------------
    // Repository + Model
    // -----------------------------------------------------
    NoteModel model;
    auto repository = std::make_unique<LocalFileNoteRepository>(notebookPath);
    auto syncService = std::make_shared<TreeSyncService>();
    AuthService authService;
    authService.restoreLastSession();
    syncService->setEnabled(authService.isSignedIn());
    NoteService noteService(model, std::move(repository), syncService);
    HandwritingService handwritingService;

    noteService.enableCloudSync(syncService->isEnabled());

    noteService.load();
    if (noteService.notebooks().isEmpty()) {
        noteService.createNotebook(QStringLiteral("My Notebook"));
        noteService.persistNow();
    }

    // -----------------------------------------------------
    // UI
    // -----------------------------------------------------
    MainWindow mainWindow(model, noteService, handwritingService);
    mainWindow.show();

    const int result = app.exec();

    // Save current state before exit.
    if (!noteService.notebooks().isEmpty()) {
        noteService.persistNow();
    }

    return result;
}
