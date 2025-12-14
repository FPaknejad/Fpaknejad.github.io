#include "MainWindow.hpp"

#include <QAction>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QApplication>
#include <QTextEdit>
#include <QKeySequence>
#include <QShortcut>
#include <QUuid>
#include <algorithm>

using namespace handnote::core;
using namespace handnote::services;

namespace {
    bool isNotebookEmpty(const Notebook& nb)
    {
        return nb.id.isNull() && nb.name.isEmpty() && nb.sections.isEmpty();
    }
}

MainWindow::MainWindow(handnote::core::NoteModel& model,
                       NoteService& noteService,
                       HandwritingService& handwritingService,
                       QWidget* parent)
    : QMainWindow(parent),
      m_model(model),
      m_noteService(noteService),
      m_handwritingService(handwritingService)
{
    loadInitialData();
    setupUi();
    rebuildTree();
}

MainWindow::~MainWindow()
{
    m_noteService.persistNow();
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("HandNote"));

    setupToolbar();
    setupCentralLayout();

    updateUndoRedoActions();
    statusBar()->showMessage("Ready");
}

void MainWindow::setupToolbar()
{
    m_toolbar = addToolBar("Main");
    m_toolbar->setMovable(false);

    m_undoAction = m_toolbar->addAction("Undo");
    m_undoAction->setShortcut(QKeySequence::Undo);
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::onUndoTriggered);

    m_redoAction = m_toolbar->addAction("Redo");
    m_redoAction->setShortcut(QKeySequence::Redo);
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::onRedoTriggered);

    m_sidebarAction = m_toolbar->addAction("Sidebar");
    m_sidebarAction->setCheckable(true);
    m_sidebarAction->setChecked(true);
    connect(m_sidebarAction, &QAction::toggled,
            this, &MainWindow::onSidebarToggled);

    m_penModeAction = m_toolbar->addAction("Pen Mode");
    m_penModeAction->setCheckable(true);
    m_penModeAction->setChecked(m_penModeEnabled);
    connect(m_penModeAction, &QAction::toggled,
            this, &MainWindow::onPenModeToggled);

    m_autoConvertAction = m_toolbar->addAction("Auto Convert");
    m_autoConvertAction->setCheckable(true);
    m_autoConvertAction->setChecked(m_autoConvert);
    connect(m_autoConvertAction, &QAction::toggled,
            this, &MainWindow::onAutoConvertToggled);

    m_convertNowAction = m_toolbar->addAction("Convert Now");
    connect(m_convertNowAction, &QAction::triggered,
            this, &MainWindow::onConvertNowClicked);
}

void MainWindow::setupCentralLayout()
{
    m_treePanel = new NoteTreePanel(this);
    m_workspace = new PageWorkspace(this);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(m_treePanel);
    splitter->addWidget(m_workspace);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    connect(m_treePanel, &NoteTreePanel::selectionChanged,
            this, &MainWindow::onTreeSelectionChanged);
    connect(m_treePanel, &NoteTreePanel::requestCreateNotebook,
            this, &MainWindow::createNotebook);
    connect(m_treePanel, &NoteTreePanel::requestCreateSection,
            this, &MainWindow::createSection);
    connect(m_treePanel, &NoteTreePanel::requestCreatePage,
            this, &MainWindow::createPage);
    connect(m_treePanel, &NoteTreePanel::requestRenameNotebook,
            this, &MainWindow::renameNotebook);
    connect(m_treePanel, &NoteTreePanel::requestRenameSection,
            this, &MainWindow::renameSection);
    connect(m_treePanel, &NoteTreePanel::requestRenamePage,
            this, &MainWindow::renamePage);
    connect(m_treePanel, &NoteTreePanel::requestDeleteNotebook,
            this, &MainWindow::deleteNotebook);
    connect(m_treePanel, &NoteTreePanel::requestDeleteSection,
            this, &MainWindow::deleteSection);
    connect(m_treePanel, &NoteTreePanel::requestDeletePage,
            this, &MainWindow::deletePage);
    connect(m_treePanel, &NoteTreePanel::requestMoveSection,
            this, &MainWindow::onSectionMoved);
    connect(m_treePanel, &NoteTreePanel::requestMovePage,
            this, &MainWindow::onPageMoved);

    new QShortcut(QKeySequence::Undo, this, SLOT(onUndoTriggered()));
    new QShortcut(QKeySequence::Redo, this, SLOT(onRedoTriggered()));

    connect(m_workspace, &PageWorkspace::textEdited, this, [this](const QString &text) {
        if (auto *page = currentPage()) {
            pushUndoSnapshot();
            m_noteService.updatePageText(page->id.toString(QUuid::WithoutBraces), text);
            updateUndoRedoActions();
        }
    });
    connect(m_workspace, &PageWorkspace::strokeFinished, this, [this](const QImage &image) {
        if (!m_autoConvert)
            return;

        const QString pageId = currentPageId();
        m_handwritingService.requestLiveConversion(image, [this, pageId](const QString &text) {
            if (pageId != currentPageId())
                return;
            if (!text.isEmpty()) {
                // TODO: snap to template lines when page.handwriting.snapToTemplateLines is true.
                Page *page = currentPage();
                const QString currentText = (page && !page->textBlocks.isEmpty())
                                                ? page->textBlocks.first().text
                                                : QString();
                const QString merged = currentText.isEmpty() ? text : currentText + "\n" + text;
                m_noteService.updatePageText(pageId, merged);
            }
        });
    });
}

void MainWindow::loadInitialData()
{
    m_noteService.load();
    ensureSampleDataIfEmpty();

    m_undoStack.clear();
    m_redoStack.clear();
    pushUndoSnapshot();

    m_currentNotebook = -1;
    m_currentSection  = -1;
    m_currentPage     = -1;
}

void MainWindow::ensureSampleDataIfEmpty()
{
    if (!notebooks().isEmpty() && !isNotebookEmpty(notebooks().first()))
        return;

    notebooks().clear();

    const QString nbId = m_noteService.createNotebook("Notes");

    const QString workId = m_noteService.createSection(nbId, "Work");
    m_noteService.createPage(workId, "Meeting notes");
    m_noteService.createPage(workId, "Project A");

    const QString personalId = m_noteService.createSection(nbId, "Personal");
    m_noteService.createPage(personalId, "Journal");

    const QString ideasId = m_noteService.createSection(nbId, "Ideas");
    m_noteService.createPage(ideasId, "App concepts");

    m_noteService.persistNow();
}

void MainWindow::rebuildTree()
{
    if (!m_treePanel)
        return;

    m_treePanel->setNotebooks(notebooks());
    loadPageIntoUi(currentPage());
}

Page *MainWindow::currentPage()
{
    if (m_currentNotebook < 0 || m_currentNotebook >= notebooks().size())
        return nullptr;

    Notebook &nb = notebooks()[m_currentNotebook];

    if (m_currentSection < 0 || m_currentSection >= nb.sections.size())
        return nullptr;

    Section &sec = nb.sections[m_currentSection];

    if (m_currentPage < 0 || m_currentPage >= sec.pages.size())
        return nullptr;

    return &sec.pages[m_currentPage];
}

void MainWindow::loadPageIntoUi(Page *page)
{
    m_isLoadingPage = true;

    m_workspace->setPage(page);

    if (page && m_autoConvertAction) {
        m_autoConvertAction->setChecked(page->handwriting.autoConvert);
        m_autoConvert = page->handwriting.autoConvert;
        m_handwritingService.setAutoConversionEnabled(m_autoConvert);
        m_handwritingService.setDebounceMs(page->handwriting.debounceMs);
    }

    m_workspace->setPenModeEnabled(m_penModeEnabled);
    m_isLoadingPage = false;
}

void MainWindow::savePageFromUi(Page *page)
{
    if (!page)
        return;
    // No-op: text changes are saved via PageWorkspace::textEdited signal.
}

QVector<Notebook> &MainWindow::notebooks()
{
    return m_model.notebooks();
}

const QVector<Notebook> &MainWindow::notebooks() const
{
    return m_model.notebooks();
}

QString MainWindow::currentPageId() const
{
    const Page *page = const_cast<MainWindow*>(this)->currentPage();
    if (!page)
        return {};
    return page->id.toString(QUuid::WithoutBraces);
}

void MainWindow::pushUndoSnapshot()
{
    m_undoStack.append(notebooks());
    while (m_undoStack.size() > m_maxHistory)
        m_undoStack.removeFirst();
    m_redoStack.clear();
    updateUndoRedoActions();
}

void MainWindow::restoreNotebookState(const QVector<Notebook> &state)
{
    notebooks() = state;
    rebuildTree();
}

void MainWindow::updateUndoRedoActions()
{
    if (m_undoAction)
        m_undoAction->setEnabled(!m_undoStack.isEmpty());
    if (m_redoAction)
        m_redoAction->setEnabled(!m_redoStack.isEmpty());
}

void MainWindow::onSidebarToggled(bool checked)
{
    if (m_treePanel)
        m_treePanel->setVisible(checked);

    statusBar()->showMessage(checked ? "Sidebar shown"
                                     : "Sidebar hidden",
                             1500);
}

void MainWindow::onPenModeToggled(bool checked)
{
    m_penModeEnabled = checked;
    if (m_workspace)
        m_workspace->setPenModeEnabled(checked);
    statusBar()->showMessage(checked ? "Pen mode ON"
                                     : "Pen mode OFF",
                             1500);
}

void MainWindow::onAutoConvertToggled(bool checked)
{
    pushUndoSnapshot();

    m_autoConvert = checked;
    m_handwritingService.setAutoConversionEnabled(checked);

    if (auto *page = currentPage()) {
        HandwritingConfig cfg = page->handwriting;
        cfg.autoConvert = checked;
        m_noteService.updateHandwritingConfig(page->id.toString(QUuid::WithoutBraces), cfg);
    }

    statusBar()->showMessage(checked ? "Auto convert ON"
                                     : "Auto convert OFF",
                             1500);

    updateUndoRedoActions();
}

void MainWindow::onConvertNowClicked()
{
    pushUndoSnapshot();

    const QString pageId = currentPageId();
    const QImage image = m_workspace->canvasImage();

    m_handwritingService.convertSelection(image, [this, pageId](const QString &text) {
        if (pageId != currentPageId())
            return;

        if (!text.isEmpty()) {
            Page *page = currentPage();
            const QString currentText = (page && !page->textBlocks.isEmpty())
                                            ? page->textBlocks.first().text
                                            : QString();
            const QString merged = currentText.isEmpty() ? text : currentText + "\n" + text;
            m_noteService.updatePageText(pageId, merged);
        }
    });

    statusBar()->showMessage("Conversion requested", 2000);
    updateUndoRedoActions();
}

void MainWindow::onUndoTriggered()
{
    if (m_undoStack.isEmpty())
        return;

    const auto current = notebooks();
    const auto prev = m_undoStack.takeLast();
    m_redoStack.append(current);
    restoreNotebookState(prev);
    updateUndoRedoActions();
    statusBar()->showMessage("Undone", 1500);
}

void MainWindow::onRedoTriggered()
{
    if (m_redoStack.isEmpty())
        return;

    const auto current = notebooks();
    const auto next = m_redoStack.takeLast();
    m_undoStack.append(current);
    restoreNotebookState(next);
    updateUndoRedoActions();
    statusBar()->showMessage("Redone", 1500);
}

void MainWindow::onTreeSelectionChanged(int notebookIndex, int sectionIndex, int pageIndex)
{
    if (notebookIndex < 0 || sectionIndex < 0 || pageIndex < 0)
        return;

    m_currentNotebook = notebookIndex;
    m_currentSection  = sectionIndex;
    m_currentPage     = pageIndex;
    loadPageIntoUi(currentPage());
}

void MainWindow::onSectionMoved(int fromNotebook, int sectionIndex, int toNotebook, int toSectionRow)
{
    if (fromNotebook < 0 || sectionIndex < 0 || toNotebook < 0)
        return;
    if (fromNotebook >= notebooks().size() || toNotebook >= notebooks().size())
        return;
    if (sectionIndex >= notebooks()[fromNotebook].sections.size())
        return;

    const QString sectionId = notebooks()[fromNotebook].sections[sectionIndex].id.toString(QUuid::WithoutBraces);
    const QString destNotebookId = notebooks()[toNotebook].id.toString(QUuid::WithoutBraces);

    int destIndex = toSectionRow;
    const int destCount = notebooks()[toNotebook].sections.size();
    if (destIndex < 0 || destIndex > destCount)
        destIndex = destCount;

    // Adjust target when moving within the same notebook and past original index.
    if (fromNotebook == toNotebook && sectionIndex < destIndex)
        destIndex -= 1;

    pushUndoSnapshot();
    if (m_noteService.moveSection(sectionId, destNotebookId, destIndex)) {
        m_currentNotebook = toNotebook;
        m_currentSection  = destIndex;
        m_currentPage     = -1;
        rebuildTree();
    }
    updateUndoRedoActions();
}

void MainWindow::onPageMoved(int fromNotebook, int fromSection, int pageIndex,
                             int toNotebook, int toSection, int toPageRow)
{
    if (fromNotebook < 0 || fromSection < 0 || pageIndex < 0 ||
        toNotebook < 0 || toSection < 0)
        return;
    if (fromNotebook >= notebooks().size() || toNotebook >= notebooks().size())
        return;
    if (fromSection >= notebooks()[fromNotebook].sections.size() ||
        toSection >= notebooks()[toNotebook].sections.size())
        return;

    const auto &srcSec = notebooks()[fromNotebook].sections[fromSection];
    if (pageIndex >= srcSec.pages.size())
        return;

    const QString pageId = srcSec.pages[pageIndex].id.toString(QUuid::WithoutBraces);
    const QString destSectionId = notebooks()[toNotebook].sections[toSection].id.toString(QUuid::WithoutBraces);

    int destIndex = toPageRow;
    const int destCount = notebooks()[toNotebook].sections[toSection].pages.size();
    if (destIndex < 0 || destIndex > destCount)
        destIndex = destCount;

    if (fromNotebook == toNotebook && fromSection == toSection && pageIndex < destIndex)
        destIndex -= 1;

    pushUndoSnapshot();
    if (m_noteService.movePage(pageId, destSectionId, destIndex)) {
        m_currentNotebook = toNotebook;
        m_currentSection  = toSection;
        m_currentPage     = destIndex;
        rebuildTree();
    }
    updateUndoRedoActions();
}

void MainWindow::createNotebook()
{
    bool ok = false;
    QString name = QInputDialog::getText(
        this,
        "New Notebook",
        "Name:",
        QLineEdit::Normal,
        "New notebook",
        &ok
    );

    if (!ok || name.isEmpty())
        return;

    pushUndoSnapshot();
    m_noteService.createNotebook(name);
    rebuildTree();
    updateUndoRedoActions();
}

void MainWindow::createSection(int notebookIndex)
{
    if (notebookIndex < 0 || notebookIndex >= notebooks().size())
        return;

    bool ok = false;
    QString name = QInputDialog::getText(
        this,
        "New Section",
        "Name:",
        QLineEdit::Normal,
        "New section",
        &ok
    );

    if (!ok || name.isEmpty())
        return;

    const QString notebookId = notebooks()[notebookIndex].id.toString(QUuid::WithoutBraces);
    pushUndoSnapshot();
    m_noteService.createSection(notebookId, name);
    rebuildTree();
    updateUndoRedoActions();
}

void MainWindow::createPage(int notebookIndex, int sectionIndex)
{
    if (notebookIndex < 0 || notebookIndex >= notebooks().size())
        return;

    Notebook &nb = notebooks()[notebookIndex];
    if (sectionIndex < 0 || sectionIndex >= nb.sections.size())
        return;

    bool ok = false;
    QString title = QInputDialog::getText(
        this,
        "New Page",
        "Title:",
        QLineEdit::Normal,
        "New page",
        &ok
    );

    if (!ok || title.isEmpty())
        return;

    const QString sectionId = nb.sections[sectionIndex].id.toString(QUuid::WithoutBraces);
    pushUndoSnapshot();
    m_noteService.createPage(sectionId, title);
    rebuildTree();
    updateUndoRedoActions();
}

void MainWindow::renameNotebook(int ni)
{
    if (ni < 0 || ni >= notebooks().size())
        return;

    bool ok = false;
    QString newName = QInputDialog::getText(
        this,
        "Rename Notebook",
        "New name:",
        QLineEdit::Normal,
        notebooks()[ni].name,
        &ok
    );

    if (!ok || newName.isEmpty())
        return;

    pushUndoSnapshot();
    m_noteService.renameNotebook(notebooks()[ni].id.toString(QUuid::WithoutBraces), newName);
    rebuildTree();
    updateUndoRedoActions();
}

void MainWindow::renameSection(int ni, int si)
{
    if (ni < 0 || ni >= notebooks().size())
        return;
    if (si < 0 || si >= notebooks()[ni].sections.size())
        return;

    bool ok = false;
    QString newName = QInputDialog::getText(
        this,
        "Rename Section",
        "New name:",
        QLineEdit::Normal,
        notebooks()[ni].sections[si].title,
        &ok
    );

    if (!ok || newName.isEmpty())
        return;

    const QString sectionId = notebooks()[ni].sections[si].id.toString(QUuid::WithoutBraces);
    pushUndoSnapshot();
    m_noteService.renameSection(sectionId, newName);
    rebuildTree();
    updateUndoRedoActions();
}

void MainWindow::renamePage(int ni, int si, int pi)
{
    if (ni < 0 || ni >= notebooks().size())
        return;
    if (si < 0 || si >= notebooks()[ni].sections.size())
        return;
    if (pi < 0 || pi >= notebooks()[ni].sections[si].pages.size())
        return;

    Page &p = notebooks()[ni].sections[si].pages[pi];

    bool ok = false;
    QString newTitle = QInputDialog::getText(
        this,
        "Rename Page",
        "New title:",
        QLineEdit::Normal,
        p.title,
        &ok
    );

    if (!ok || newTitle.isEmpty())
        return;

    pushUndoSnapshot();
    m_noteService.renamePage(p.id.toString(QUuid::WithoutBraces), newTitle);
    rebuildTree();
    updateUndoRedoActions();
}

void MainWindow::deleteNotebook(int ni)
{
    if (ni < 0 || ni >= notebooks().size())
        return;

    if (QMessageBox::question(this, "Delete Notebook",
                              "Delete this notebook and all its sections/pages?")
        != QMessageBox::Yes)
        return;

    pushUndoSnapshot();
    m_noteService.deleteNotebook(notebooks()[ni].id.toString(QUuid::WithoutBraces));
    m_currentNotebook = m_currentSection = m_currentPage = -1;
    rebuildTree();
    updateUndoRedoActions();
}

void MainWindow::deleteSection(int ni, int si)
{
    if (ni < 0 || ni >= notebooks().size())
        return;
    if (si < 0 || si >= notebooks()[ni].sections.size())
        return;

    if (QMessageBox::question(this, "Delete Section",
                              "Delete this section and all its pages?")
        != QMessageBox::Yes)
        return;

    pushUndoSnapshot();
    m_noteService.deleteSection(notebooks()[ni].sections[si].id.toString(QUuid::WithoutBraces));
    m_currentNotebook = m_currentSection = m_currentPage = -1;
    rebuildTree();
    updateUndoRedoActions();
}

void MainWindow::deletePage(int ni, int si, int pi)
{
    if (ni < 0 || ni >= notebooks().size())
        return;
    if (si < 0 || si >= notebooks()[ni].sections.size())
        return;
    if (pi < 0 || pi >= notebooks()[ni].sections[si].pages.size())
        return;

    if (QMessageBox::question(this, "Delete Page",
                              "Delete this page?")
        != QMessageBox::Yes)
        return;

    pushUndoSnapshot();
    m_noteService.deletePage(notebooks()[ni].sections[si].pages[pi].id.toString(QUuid::WithoutBraces));
    m_currentNotebook = m_currentSection = m_currentPage = -1;
    rebuildTree();
    updateUndoRedoActions();
}
