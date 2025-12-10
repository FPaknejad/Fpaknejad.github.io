#include "MainWindow.hpp"
#include "HandwritingCanvas.hpp"
#include "core/LocalFileNoteRepository.hpp"

#include <QToolBar>
#include <QAction>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextEdit>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QColorDialog>
#include <QMenu>
#include <QInputDialog>
#include <QLineEdit>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QUuid>
#include <QRectF>

using namespace handnote::core;

namespace {
    constexpr int RoleNotebook = Qt::UserRole + 1;
    constexpr int RoleSection  = Qt::UserRole + 2;
    constexpr int RolePage     = Qt::UserRole + 3;
}

// -----------------------------------------------------
// ctor / dtor
// -----------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    loadInitialData();
    rebuildTree();
}

MainWindow::~MainWindow()
{
    if (m_repo) {
        m_repo->saveAll(m_notebooks);
    }
}

// -----------------------------------------------------
// UI setup
// -----------------------------------------------------

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("HandNote"));
    resize(1200, 800);

    setupToolbar();
    setupCentralLayout();

    statusBar()->showMessage("Ready");
}

void MainWindow::setupToolbar()
{
    m_toolbar = addToolBar("Main");
    m_toolbar->setMovable(false);

    m_sidebarAction = m_toolbar->addAction("Sidebar");
    m_sidebarAction->setCheckable(true);
    m_sidebarAction->setChecked(true);
    connect(m_sidebarAction, &QAction::toggled,
            this, &MainWindow::onSidebarToggled);

    m_penModeAction = m_toolbar->addAction("Pen Mode");
    m_penModeAction->setCheckable(true);
    m_penModeAction->setChecked(true);
    connect(m_penModeAction, &QAction::toggled,
            this, &MainWindow::onPenModeToggled);

    m_autoConvertAction = m_toolbar->addAction("Auto Convert");
    m_autoConvertAction->setCheckable(true);
    connect(m_autoConvertAction, &QAction::toggled,
            this, &MainWindow::onAutoConvertToggled);

    m_convertNowAction = m_toolbar->addAction("Convert Now");
    connect(m_convertNowAction, &QAction::triggered,
            this, &MainWindow::onConvertNowClicked);
}

void MainWindow::setupCentralLayout()
{
    // main splitter
    m_splitter = new QSplitter(this);
    setCentralWidget(m_splitter);

    // ---------- LEFT: sidebar container ----------
    QWidget *sidebar = new QWidget(this);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(4, 4, 4, 4);
    sidebarLayout->setSpacing(4);

    // mini toolbar (+  -  🎨)
    QHBoxLayout *mini = new QHBoxLayout();

    auto *btnAdd    = new QPushButton("+");
    auto *btnDelete = new QPushButton("-");
    auto *btnColor  = new QPushButton("🎨");

    btnAdd->setFixedWidth(28);
    btnDelete->setFixedWidth(28);
    btnColor->setFixedWidth(28);

    mini->addWidget(btnAdd);
    mini->addWidget(btnDelete);
    mini->addWidget(btnColor);
    mini->addStretch();

    sidebarLayout->addLayout(mini);

    // tree
    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // IMPORTANT: disable drag/drop for now to avoid “jumping” bugs
    m_tree->setDragEnabled(false);
    m_tree->setAcceptDrops(false);
    m_tree->setDropIndicatorShown(false);
    m_tree->setDragDropMode(QAbstractItemView::NoDragDrop);

    sidebarLayout->addWidget(m_tree);

    m_splitter->addWidget(sidebar);
    m_splitter->setStretchFactor(0, 0);

    // ---------- RIGHT: text + canvas ----------
    QWidget *rightSide = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightSide);
    rightLayout->setContentsMargins(4, 4, 4, 4);
    rightLayout->setSpacing(4);

    m_textEdit = new QTextEdit(this);
    m_canvas   = new HandwritingCanvas(this);

    rightLayout->addWidget(m_textEdit, 1);
    rightLayout->addWidget(m_canvas,   2);

    m_splitter->addWidget(rightSide);
    m_splitter->setStretchFactor(1, 1);

    // ---------- connections ----------
    connect(btnAdd,    &QPushButton::clicked, this, &MainWindow::createNotebook);
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::deleteSelectedItem);
    connect(btnColor,  &QPushButton::clicked, this, &MainWindow::colorSelectedItem);

    connect(m_tree, &QTreeWidget::currentItemChanged,
            this, &MainWindow::onTreeCurrentItemChanged);

    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree, &QTreeWidget::customContextMenuRequested,
            this, &MainWindow::showContextMenu);

    connect(m_textEdit, &QTextEdit::textChanged,
            this, &MainWindow::onNoteTextChanged);

    connect(m_canvas, &HandwritingCanvas::strokeFinished,
            this, &MainWindow::onCanvasStrokeFinished);
}

// -----------------------------------------------------
// Data loading / sample data
// -----------------------------------------------------

void MainWindow::loadInitialData()
{
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appData.isEmpty())
        appData = QDir::homePath() + "/.handnote";

    QDir().mkpath(appData);
    QString jsonPath = appData + "/notes.json";

    m_repo = std::make_unique<LocalFileNoteRepository>(jsonPath);
    m_repo->setParent(this);

    m_notebooks = m_repo->loadAll();
    ensureSampleDataIfEmpty();

    m_currentNotebook = -1;
    m_currentSection  = -1;
    m_currentPage     = -1;
}

void MainWindow::ensureSampleDataIfEmpty()
{
    if (!m_notebooks.isEmpty())
        return;

    Notebook nb;
    nb.id   = QUuid::createUuid();
    nb.name = "Notes";

    Section work;
    work.id   = QUuid::createUuid();
    work.name = "Work";

    Page meeting;
    meeting.id    = QUuid::createUuid();
    meeting.title = "Meeting notes";

    Page project;
    project.id    = QUuid::createUuid();
    project.title = "Project A";

    work.pages.append(meeting);
    work.pages.append(project);

    Section personal;
    personal.id   = QUuid::createUuid();
    personal.name = "Personal";

    Page journal;
    journal.id    = QUuid::createUuid();
    journal.title = "Journal";

    personal.pages.append(journal);

    nb.sections.append(work);
    nb.sections.append(personal);

    m_notebooks.append(nb);

    if (m_repo) {
        m_repo->saveAll(m_notebooks);
    }
}

// -----------------------------------------------------
// Tree building / selection
// -----------------------------------------------------

void MainWindow::rebuildTree()
{
    if (!m_tree)
        return;

    m_tree->clear();

    for (int ni = 0; ni < m_notebooks.size(); ++ni) {
        const Notebook &nb = m_notebooks[ni];

        auto *nbItem = new QTreeWidgetItem(QStringList(nb.name));
        nbItem->setData(0, RoleNotebook, ni);
        nbItem->setData(0, RoleSection,  -1);
        nbItem->setData(0, RolePage,     -1);
        m_tree->addTopLevelItem(nbItem);

        for (int si = 0; si < nb.sections.size(); ++si) {
            const Section &sec = nb.sections[si];

            auto *secItem = new QTreeWidgetItem(QStringList(sec.name));
            secItem->setData(0, RoleNotebook, ni);
            secItem->setData(0, RoleSection,  si);
            secItem->setData(0, RolePage,     -1);
            nbItem->addChild(secItem);

            for (int pi = 0; pi < sec.pages.size(); ++pi) {
                const Page &pg = sec.pages[pi];

                auto *pgItem = new QTreeWidgetItem(QStringList(pg.title));
                pgItem->setData(0, RoleNotebook, ni);
                pgItem->setData(0, RoleSection,  si);
                pgItem->setData(0, RolePage,     pi);
                secItem->addChild(pgItem);

                if (m_currentNotebook == -1) {
                    m_currentNotebook = ni;
                    m_currentSection  = si;
                    m_currentPage     = pi;
                    m_tree->setCurrentItem(pgItem);
                }
            }

            secItem->setExpanded(true);
        }

        nbItem->setExpanded(true);
    }

    loadPageIntoUi(currentPage());
}

bool MainWindow::indexFromItem(QTreeWidgetItem *item,
                               int &ni, int &si, int &pi) const
{
    if (!item)
        return false;

    ni = item->data(0, RoleNotebook).toInt();
    si = item->data(0, RoleSection).toInt();
    pi = item->data(0, RolePage).toInt();

    return (ni >= 0);
}

// -----------------------------------------------------
// Page <-> UI sync
// -----------------------------------------------------

Page *MainWindow::currentPage()
{
    if (m_currentNotebook < 0 || m_currentNotebook >= m_notebooks.size())
        return nullptr;

    Notebook &nb = m_notebooks[m_currentNotebook];

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

    if (!page) {
        m_textEdit->clear();
        m_canvas->clearCanvas();
        m_isLoadingPage = false;
        return;
    }

    QString text;
    for (const TextBlock &tb : page->textBlocks) {
        if (!text.isEmpty())
            text += "\n\n";
        text += tb.text;
    }
    m_textEdit->setPlainText(text);

    // TODO: restore strokes into canvas later
    m_canvas->clearCanvas();

    m_isLoadingPage = false;
}

void MainWindow::savePageFromUi(Page *page)
{
    if (!page)
        return;

    const QString text = m_textEdit->toPlainText();
    page->textBlocks.clear();

    if (!text.isEmpty()) {
        TextBlock tb;
        tb.id          = QUuid::createUuid();
        tb.text        = text;
        tb.boundingBox = QRectF();
        tb.lineIndex   = 0;
        page->textBlocks.append(tb);
    }

    if (m_repo) {
        m_repo->saveAll(m_notebooks);
    }
}

// -----------------------------------------------------
// Slots
// -----------------------------------------------------

void MainWindow::onTreeCurrentItemChanged(QTreeWidgetItem *current,
                                          QTreeWidgetItem * /*previous*/)
{
    if (!current)
        return;

    // save current page before switching
    savePageFromUi(currentPage());

    int ni, si, pi;
    if (!indexFromItem(current, ni, si, pi))
        return;

    if (ni >= 0 && si >= 0 && pi >= 0) {
        m_currentNotebook = ni;
        m_currentSection  = si;
        m_currentPage     = pi;
        loadPageIntoUi(currentPage());
    }
}

void MainWindow::onSidebarToggled(bool checked)
{
    if (m_tree) {
        m_tree->setVisible(checked);
    }
    statusBar()->showMessage(checked ? "Sidebar shown"
                                     : "Sidebar hidden",
                             1500);
}

void MainWindow::onPenModeToggled(bool checked)
{
    m_penModeEnabled = checked;
    statusBar()->showMessage(checked ? "Pen mode ON"
                                     : "Pen mode OFF",
                             1500);
}

void MainWindow::onAutoConvertToggled(bool checked)
{
    m_autoConvert = checked;
    statusBar()->showMessage(checked ? "Auto convert ON"
                                     : "Auto convert OFF",
                             1500);
}

void MainWindow::onConvertNowClicked()
{
    qDebug() << "[ConvertNow] Not implemented yet - will use canvas image later";
    statusBar()->showMessage("Convert Now (not implemented yet)", 2000);
}

void MainWindow::onCanvasStrokeFinished()
{
    qDebug() << "[HandwritingCanvas] strokeFinished()";
    if (m_autoConvert) {
        qDebug() << "[HandwritingCanvas] auto-convert placeholder";
    }
}

void MainWindow::onNoteTextChanged()
{
    if (m_isLoadingPage)
        return;

    savePageFromUi(currentPage());
}

// -----------------------------------------------------
// CRUD for sidebar / context menu
// -----------------------------------------------------

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

    Notebook nb;
    nb.id   = QUuid::createUuid();
    nb.name = name;

    m_notebooks.append(nb);
    if (m_repo) m_repo->saveAll(m_notebooks);
    rebuildTree();
}

void MainWindow::createSection(int notebookIndex)
{
    if (notebookIndex < 0 || notebookIndex >= m_notebooks.size())
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

    Section sec;
    sec.id   = QUuid::createUuid();
    sec.name = name;

    m_notebooks[notebookIndex].sections.append(sec);
    if (m_repo) m_repo->saveAll(m_notebooks);
    rebuildTree();
}

void MainWindow::createPage(int notebookIndex, int sectionIndex)
{
    if (notebookIndex < 0 || notebookIndex >= m_notebooks.size())
        return;
    if (sectionIndex < 0 || sectionIndex >= m_notebooks[notebookIndex].sections.size())
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

    Page p;
    p.id    = QUuid::createUuid();
    p.title = title;

    m_notebooks[notebookIndex].sections[sectionIndex].pages.append(p);
    if (m_repo) m_repo->saveAll(m_notebooks);
    rebuildTree();
}

void MainWindow::renameNotebook(int ni)
{
    if (ni < 0 || ni >= m_notebooks.size())
        return;

    bool ok = false;
    QString newName = QInputDialog::getText(
        this,
        "Rename Notebook",
        "New name:",
        QLineEdit::Normal,
        m_notebooks[ni].name,
        &ok
    );

    if (!ok || newName.isEmpty())
        return;

    m_notebooks[ni].name = newName;
    if (m_repo) m_repo->saveAll(m_notebooks);
    rebuildTree();
}

void MainWindow::renameSection(int ni, int si)
{
    if (ni < 0 || ni >= m_notebooks.size())
        return;
    if (si < 0 || si >= m_notebooks[ni].sections.size())
        return;

    bool ok = false;
    QString newName = QInputDialog::getText(
        this,
        "Rename Section",
        "New name:",
        QLineEdit::Normal,
        m_notebooks[ni].sections[si].name,
        &ok
    );

    if (!ok || newName.isEmpty())
        return;

    m_notebooks[ni].sections[si].name = newName;
    if (m_repo) m_repo->saveAll(m_notebooks);
    rebuildTree();
}

void MainWindow::renamePage(int ni, int si, int pi)
{
    if (ni < 0 || ni >= m_notebooks.size())
        return;
    if (si < 0 || si >= m_notebooks[ni].sections.size())
        return;
    if (pi < 0 || pi >= m_notebooks[ni].sections[si].pages.size())
        return;

    Page &p = m_notebooks[ni].sections[si].pages[pi];

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

    p.title = newTitle;
    if (m_repo) m_repo->saveAll(m_notebooks);
    rebuildTree();
}

void MainWindow::deleteNotebook(int ni)
{
    if (ni < 0 || ni >= m_notebooks.size())
        return;

    if (QMessageBox::question(this, "Delete Notebook",
                              "Delete this notebook and all its sections/pages?")
        != QMessageBox::Yes)
        return;

    m_notebooks.removeAt(ni);
    if (m_repo) m_repo->saveAll(m_notebooks);
    m_currentNotebook = m_currentSection = m_currentPage = -1;
    rebuildTree();
}

void MainWindow::deleteSection(int ni, int si)
{
    if (ni < 0 || ni >= m_notebooks.size())
        return;
    if (si < 0 || si >= m_notebooks[ni].sections.size())
        return;

    if (QMessageBox::question(this, "Delete Section",
                              "Delete this section and all its pages?")
        != QMessageBox::Yes)
        return;

    m_notebooks[ni].sections.removeAt(si);
    if (m_repo) m_repo->saveAll(m_notebooks);
    m_currentNotebook = m_currentSection = m_currentPage = -1;
    rebuildTree();
}

void MainWindow::deletePage(int ni, int si, int pi)
{
    if (ni < 0 || ni >= m_notebooks.size())
        return;
    if (si < 0 || si >= m_notebooks[ni].sections.size())
        return;
    if (pi < 0 || pi >= m_notebooks[ni].sections[si].pages.size())
        return;

    if (QMessageBox::question(this, "Delete Page",
                              "Delete this page?")
        != QMessageBox::Yes)
        return;

    m_notebooks[ni].sections[si].pages.removeAt(pi);
    if (m_repo) m_repo->saveAll(m_notebooks);
    m_currentNotebook = m_currentSection = m_currentPage = -1;
    rebuildTree();
}

// -----------------------------------------------------
// Sidebar mini-toolbar actions
// -----------------------------------------------------

void MainWindow::deleteSelectedItem()
{
    QTreeWidgetItem *item = m_tree->currentItem();
    if (!item)
        return;

    int ni, si, pi;
    if (!indexFromItem(item, ni, si, pi))
        return;

    if (pi >= 0) {
        deletePage(ni, si, pi);
    } else if (si >= 0) {
        deleteSection(ni, si);
    } else if (ni >= 0) {
        deleteNotebook(ni);
    }
}

void MainWindow::colorSelectedItem()
{
    QTreeWidgetItem *item = m_tree->currentItem();
    if (!item)
        return;

    QColor color = QColorDialog::getColor(Qt::yellow, this, "Choose color");
    if (!color.isValid())
        return;

    item->setBackground(0, QBrush(color));
}

// -----------------------------------------------------
// Context menu
// -----------------------------------------------------

void MainWindow::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_tree->itemAt(pos);
    QMenu menu(this);

    int ni, si, pi;
    if (!indexFromItem(item, ni, si, pi)) {
        // empty area: only allow new notebook
        QAction *aNewNotebook = menu.addAction("New Notebook");
        connect(aNewNotebook, &QAction::triggered,
                this, &MainWindow::createNotebook);
        menu.exec(m_tree->viewport()->mapToGlobal(pos));
        return;
    }

    if (pi >= 0) { // page
        QAction *rename = menu.addAction("Rename Page");
        QAction *del    = menu.addAction("Delete Page");

        connect(rename, &QAction::triggered,
                this, [this, ni, si, pi]() { renamePage(ni, si, pi); });
        connect(del, &QAction::triggered,
                this, [this, ni, si, pi]() { deletePage(ni, si, pi); });
    } else if (si >= 0) { // section
        QAction *newPage = menu.addAction("New Page");
        QAction *rename  = menu.addAction("Rename Section");
        QAction *del     = menu.addAction("Delete Section");

        connect(newPage, &QAction::triggered,
                this, [this, ni, si]() { createPage(ni, si); });
        connect(rename, &QAction::triggered,
                this, [this, ni, si]() { renameSection(ni, si); });
        connect(del, &QAction::triggered,
                this, [this, ni, si]() { deleteSection(ni, si); });
    } else { // notebook
        QAction *newSec = menu.addAction("New Section");
        QAction *rename = menu.addAction("Rename Notebook");
        QAction *del    = menu.addAction("Delete Notebook");

        connect(newSec, &QAction::triggered,
                this, [this, ni]() { createSection(ni); });
        connect(rename, &QAction::triggered,
                this, [this, ni]() { renameNotebook(ni); });
        connect(del, &QAction::triggered,
                this, [this, ni]() { deleteNotebook(ni); });
    }

    menu.exec(m_tree->viewport()->mapToGlobal(pos));
}
