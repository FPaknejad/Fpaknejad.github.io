#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QList>
#include <QMainWindow>
#include <QToolBar>

#include "domain/NoteModel.hpp"
#include "services/NoteService.hpp"
#include "services/HandwritingService.hpp"
#include "NoteTreePanel.hpp"
#include "PageWorkspace.hpp"

class QToolBar;
class QAction;
class NoteController;
class QTextEdit;

namespace handnote::core {
    class LocalFileNoteRepository;
}
namespace handnote::services {
    class NoteService;
    class HandwritingService;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(handnote::core::NoteModel& model,
                        handnote::services::NoteService& noteService,
                        handnote::services::HandwritingService& handwritingService,
                        QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    // toolbar
    void onSidebarToggled(bool checked);
    void onPenModeToggled(bool checked);
    void onAutoConvertToggled(bool checked);
    void onConvertNowClicked();

    // tree selection
    void onTreeSelectionChanged(int notebookIndex, int sectionIndex, int pageIndex);
    void onSectionMoved(int fromNotebook, int sectionIndex, int toNotebook, int toSectionRow);
    void onPageMoved(int fromNotebook, int fromSection, int pageIndex,
                     int toNotebook, int toSection, int toPageRow);
    void onUndoTriggered();
    void onRedoTriggered();

    // sidebar mini-toolbar buttons
    void createNotebook();          // "+" button (and context menu)

private:
    // UI
    void setupUi();
    void setupToolbar();
    void setupCentralLayout();

    // Data
    void loadInitialData();
    void ensureSampleDataIfEmpty();
    void rebuildTree();

    // helpers
    handnote::core::Page *currentPage();
    void loadPageIntoUi(handnote::core::Page *page);
    void savePageFromUi(handnote::core::Page *page);
    void pushUndoSnapshot();
    void restoreNotebookState(const QVector<handnote::core::Notebook> &state);
    void updateUndoRedoActions();

    QVector<handnote::core::Notebook> &notebooks();
    const QVector<handnote::core::Notebook> &notebooks() const;
    QString currentPageId() const;

    // CRUD helpers used by context menu
    void createSection(int notebookIndex);
    void createPage(int notebookIndex, int sectionIndex);
    void renameNotebook(int notebookIndex);
    void renameSection(int notebookIndex, int sectionIndex);
    void renamePage(int notebookIndex, int sectionIndex, int pageIndex);
    void deleteNotebook(int notebookIndex);
    void deleteSection(int notebookIndex, int sectionIndex);
    void deletePage(int notebookIndex, int sectionIndex, int pageIndex);

private:
    // Data
    handnote::core::NoteModel& m_model;
    handnote::services::NoteService& m_noteService;
    handnote::services::HandwritingService& m_handwritingService;

    int  m_currentNotebook = -1;
    int  m_currentSection  = -1;
    int  m_currentPage     = -1;
    bool m_isLoadingPage   = false;
    bool m_penModeEnabled  = true;
    bool m_autoConvert     = false;

    // UI
    QToolBar       *m_toolbar           = nullptr;
    QAction        *m_sidebarAction     = nullptr;
    QAction        *m_penModeAction     = nullptr;
    QAction        *m_autoConvertAction = nullptr;
    QAction        *m_convertNowAction  = nullptr;
    QAction        *m_undoAction        = nullptr;
    QAction        *m_redoAction        = nullptr;

    NoteTreePanel  *m_treePanel         = nullptr;
    PageWorkspace  *m_workspace         = nullptr;

    NoteController* m_controller = nullptr;

    QList<QVector<handnote::core::Notebook>> m_undoStack;
    QList<QVector<handnote::core::Notebook>> m_redoStack;
    const int m_maxHistory = 50;
};

#endif // MAINWINDOW_HPP
