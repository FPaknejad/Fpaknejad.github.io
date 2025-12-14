#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QBrush>
#include <QColorDialog>

#include "domain/NoteTypes.hpp"

class NoteTreePanel : public QWidget
{
    Q_OBJECT

public:
    explicit NoteTreePanel(QWidget *parent = nullptr);

    void setNotebooks(const QVector<handnote::core::Notebook> &notebooks);
    bool currentIndex(int &notebookIndex, int &sectionIndex, int &pageIndex) const;

    static constexpr int RoleNotebook = Qt::UserRole + 1;
    static constexpr int RoleSection  = Qt::UserRole + 2;
    static constexpr int RolePage     = Qt::UserRole + 3;

signals:
    void selectionChanged(int notebookIndex, int sectionIndex, int pageIndex);

    void requestCreateNotebook();
    void requestCreateSection(int notebookIndex);
    void requestCreatePage(int notebookIndex, int sectionIndex);

    void requestRenameNotebook(int notebookIndex);
    void requestRenameSection(int notebookIndex, int sectionIndex);
    void requestRenamePage(int notebookIndex, int sectionIndex, int pageIndex);

    void requestDeleteNotebook(int notebookIndex);
    void requestDeleteSection(int notebookIndex, int sectionIndex);
    void requestDeletePage(int notebookIndex, int sectionIndex, int pageIndex);
    void requestMoveSection(int fromNotebook, int sectionIndex, int toNotebook, int toSectionRow);
    void requestMovePage(int fromNotebook, int fromSection, int pageIndex,
                         int toNotebook, int toSection, int toPageRow);

private:
    QTreeWidget *m_tree = nullptr;
    QPushButton *m_btnAdd = nullptr;
    QPushButton *m_btnDelete = nullptr;
    QPushButton *m_btnColor = nullptr;
    QVector<handnote::core::Notebook> m_notebooks;

    void buildUi();
    void rebuildTree();
    bool indexFromItem(QTreeWidgetItem *item, int &ni, int &si, int &pi) const;

    void handleContextMenu(const QPoint &pos);
    void emitDeleteForCurrent();
    void colorCurrentItem();
};
