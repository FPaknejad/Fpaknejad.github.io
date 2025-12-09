#include "MainWindow.h"
#include "HandwritingCanvas.h"

#include <QDockWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHeaderView>
#include <QFont>
#include <QImage>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupStyle();
    setupSidebar();
    setupCentralArea();
    setupToolbar();

    setWindowTitle("HandNote");
    resize(1280, 800);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupStyle()
{
    // simple UI
    setStyleSheet(R"(
        QMainWindow {
            background-color: #202124;
        }
        QToolBar {
            background-color: #303134;
            border: none;
            padding: 4px;
            spacing: 6px;
        }
        QToolBar QToolButton {
            background-color: #3c4043;
            color: #e8eaed;
            border-radius: 4px;
            padding: 4px 10px;
        }
        QToolBar QToolButton:checked {
            background-color: #1a73e8;
        }
        QToolBar QToolButton:hover {
            background-color: #4b4f52;
        }
        QTextEdit {
            background-color: #202124;
            color: #e8eaed;
            border: none;
            padding: 8px;
            font-family: "Consolas", "JetBrains Mono", monospace;
            font-size: 11pt;
        }
        QDockWidget {
            background-color: #202124;
            border: none;
        }
        QTreeView {
            background-color: #202124;
            color: #e8eaed;
            border: none;
            padding: 4px;
            font-size: 10pt;
        }
        QTreeView::item:selected {
            background-color: #3c4043;
        }
    )");
}

void MainWindow::setupSidebar()
{
    sidebarDock = new QDockWidget("Notes", this);
    sidebarDock->setFeatures(QDockWidget::DockWidgetMovable |
                             QDockWidget::DockWidgetClosable |
                             QDockWidget::DockWidgetFloatable);

    notesTree = new QTreeView(sidebarDock);
    notesTree->setHeaderHidden(true);
    notesTree->setIndentation(16);

    notesModel = new QStandardItemModel(notesTree);

    // dummy folder structure to make UI look alive
    auto *root = notesModel->invisibleRootItem();

    auto *workFolder = new QStandardItem("Work");
    auto *personalFolder = new QStandardItem("Personal");
    auto *ideasFolder = new QStandardItem("Ideas");

    workFolder->appendRow(new QStandardItem("Meeting notes"));
    workFolder->appendRow(new QStandardItem("Project A"));
    personalFolder->appendRow(new QStandardItem("Journal"));
    ideasFolder->appendRow(new QStandardItem("App concepts"));

    root->appendRow(workFolder);
    root->appendRow(personalFolder);
    root->appendRow(ideasFolder);

    notesTree->setModel(notesModel);
    notesTree->expandAll();
    notesTree->setMinimumWidth(220);

    sidebarDock->setWidget(notesTree);
    addDockWidget(Qt::LeftDockWidgetArea, sidebarDock);
}

void MainWindow::setupCentralArea()
{
    textEditor = new QTextEdit(this);
    textEditor->setPlaceholderText("Type your note here…");

    canvas = new HandwritingCanvas(this);

    auto *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(textEditor);
    splitter->addWidget(canvas);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    setCentralWidget(splitter);

    // auto-convert when stroke finished if enabled
    connect(canvas, &HandwritingCanvas::strokeFinished, this, [this]() {
        if (!autoConvertEnabled)
            return;

        const QImage img = canvas->toImage();
        const QString text = convertHandwritingToText(img);
        if (!text.isEmpty()) {
            textEditor->append(text);
            canvas->clearCanvas();
        }
    });
}

void MainWindow::setupToolbar()
{
    auto *bar = addToolBar("Main");
    bar->setMovable(false);

    // Sidebar toggle
    QAction* toggleSidebar = bar->addAction("Sidebar");
    toggleSidebar->setCheckable(true);
    toggleSidebar->setChecked(true);

    connect(toggleSidebar, &QAction::toggled, this, [this](bool on){
        sidebarDock->setVisible(on);
    });

    // Pen mode
    QAction* penMode = bar->addAction("Pen Mode");
    penMode->setCheckable(true);
    penMode->setChecked(true);

    connect(penMode, &QAction::toggled, this, [this](bool on){
        penModeEnabled = on;
        canvas->setEnabled(on);
    });

    // Auto-convert toggle
    QAction* convertToggle = bar->addAction("Auto Convert");
    convertToggle->setCheckable(true);

    connect(convertToggle, &QAction::toggled, this, [this](bool on){
        autoConvertEnabled = on;
    });

    // Convert now
    QAction* convertNow = bar->addAction("Convert Now");
    connect(convertNow, &QAction::triggered, this, [this]{
        const QImage img = canvas->toImage();
        const QString text = convertHandwritingToText(img);
        if (!text.isEmpty()) {
            textEditor->append(text);
            canvas->clearCanvas();
        }
    });
}

QString MainWindow::convertHandwritingToText(const QImage & /*img*/)
{
    // real handwriting recognition.
    // For now,  placeholder
    return "[handwriting converted text]";
}
