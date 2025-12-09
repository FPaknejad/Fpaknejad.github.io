#pragma once

#include <QMainWindow>

class QTextEdit;
class QTreeView;
class QDockWidget;
class QStandardItemModel;
class HandwritingCanvas;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupSidebar();
    void setupCentralArea();
    void setupToolbar();
    void setupStyle();
    QString convertHandwritingToText(const QImage &img);

private:
    QDockWidget*       sidebarDock   {};
    QTreeView*         notesTree     {};
    QStandardItemModel* notesModel   {};
    QTextEdit*         textEditor    {};
    HandwritingCanvas* canvas        {};

    bool penModeEnabled     = true;
    bool autoConvertEnabled = false;
};
