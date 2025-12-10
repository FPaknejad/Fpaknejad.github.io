#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>

#include "core/NoteModel.hpp"

class QToolBar;
class QAction;
class QTreeWidget;
class QTreeWidgetItem;
class QTextEdit;
class QSplitter;
class QMenu;
class HandwritingCanvas;

namespace handnote::core {
    class LocalFileNoteRepository;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // top toolbar
    void onSidebarToggled(bool checked);
    void onPenModeToggled(bool checked);
    void onAutoConvertToggled(bool checked);
    void onConvertNowClicked();

    // canvas + text
    void onCanvasStrokeFinished();
    void onNoteTextChanged();

    // tree selection + context menu
    void onTreeCurrentItemChanged(QTreeWidgetItem *current,
                                  QTreeWidgetItem *previous);
    void showContextMenu(const QPoint &pos);

    // sidebar mini-toolbar buttons
    void createNotebook();          // "+" button or context menu
    void deleteSelectedItem();      // "-" button
    void colorSelectedItem();       // "🎨" button

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

    bool indexFromItem(QTreeWidgetItem *item,
                       int &notebookIndex,
                       int &sectionIndex,
                       int &pageIndex) const;

    // CRUD helpers used by context menu / buttons
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
    std::unique_ptr<handnote::core::LocalFileNoteRepository> m_repo;
    QList<handnote::core::Notebook> m_notebooks;

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

    QSplitter      *m_splitter      = nullptr;
    QTreeWidget    *m_tree          = nullptr;
    QTextEdit      *m_textEdit      = nullptr;
    HandwritingCanvas *m_canvas     = nullptr;
};

#endif // MAINWINDOW_HPP
