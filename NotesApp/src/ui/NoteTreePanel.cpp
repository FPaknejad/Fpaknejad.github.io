#include "NoteTreePanel.hpp"

#include <QDropEvent>

using namespace handnote::core;

namespace {
class DragTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit DragTreeWidget(QWidget *parent = nullptr)
        : QTreeWidget(parent)
    {
        setDragEnabled(true);
        setAcceptDrops(true);
        setDropIndicatorShown(true);
        setDefaultDropAction(Qt::MoveAction);
        setDragDropMode(QAbstractItemView::InternalMove);
    }

signals:
    void sectionDropped(int fromNotebook, int sectionIndex, int toNotebook, int toSectionRow);
    void pageDropped(int fromNotebook, int fromSection, int pageIndex,
                     int toNotebook, int toSection, int toPageRow);

protected:
    void startDrag(Qt::DropActions supportedActions) override
    {
        m_dragItem = currentItem();
        QTreeWidget::startDrag(supportedActions);
    }

    void dropEvent(QDropEvent *event) override
    {
        if (!m_dragItem) {
            event->ignore();
            return;
        }

        QTreeWidgetItem *targetItem = itemAt(event->position().toPoint());
        if (!targetItem) {
            event->ignore();
            return;
        }

        const Qt::DropAction action = event->proposedAction();
        if (action != Qt::MoveAction) {
            event->ignore();
            return;
        }

        int srcNb = m_dragItem->data(0, NoteTreePanel::RoleNotebook).toInt();
        int srcSec = m_dragItem->data(0, NoteTreePanel::RoleSection).toInt();
        int srcPg = m_dragItem->data(0, NoteTreePanel::RolePage).toInt();

        int dstNb = targetItem->data(0, NoteTreePanel::RoleNotebook).toInt();
        int dstSec = targetItem->data(0, NoteTreePanel::RoleSection).toInt();
        int dstPg = targetItem->data(0, NoteTreePanel::RolePage).toInt();

        const auto indicator = dropIndicatorPosition();

        if (srcPg >= 0) {
            // Moving a page
            if (dstPg >= 0) {
                int insertRow = dstPg + (indicator == QAbstractItemView::BelowItem ? 1 : 0);
                if (srcNb == dstNb && srcSec == dstSec && srcPg < insertRow)
                    insertRow -= 1; // account for removal before insertion
                emit pageDropped(srcNb, srcSec, srcPg, dstNb, dstSec, insertRow);
                event->acceptProposedAction();
                return;
            }

            if (dstSec >= 0) {
                int insertRow = targetItem->childCount();
                if (indicator == QAbstractItemView::AboveItem)
                    insertRow = 0;
                emit pageDropped(srcNb, srcSec, srcPg, dstNb, dstSec, insertRow);
                event->acceptProposedAction();
                return;
            }
        } else if (srcSec >= 0) {
            // Moving a section
            if (dstSec >= 0) {
                int insertRow = dstSec + (indicator == QAbstractItemView::BelowItem ? 1 : 0);
                if (srcNb == dstNb && srcSec < insertRow)
                    insertRow -= 1;
                emit sectionDropped(srcNb, srcSec, dstNb, insertRow);
                event->acceptProposedAction();
                return;
            }

            if (dstPg >= 0) {
                // Dropping onto a page goes to that page's section.
                QTreeWidgetItem *parentSection = targetItem->parent();
                if (parentSection) {
                    int targetSectionIndex = parentSection->data(0, NoteTreePanel::RoleSection).toInt();
                    int insertRow = targetSectionIndex + (indicator == QAbstractItemView::BelowItem ? 1 : 0);
                    if (srcNb == dstNb && srcSec < insertRow)
                        insertRow -= 1;
                    emit sectionDropped(srcNb, srcSec, dstNb, insertRow);
                    event->acceptProposedAction();
                    return;
                }
            }

            if (dstNb >= 0 && dstSec < 0) {
                // Dropping onto the notebook header: append section.
                emit sectionDropped(srcNb, srcSec, dstNb, -1);
                event->acceptProposedAction();
                return;
            }
        }

        event->ignore();
    }

private:
    QTreeWidgetItem *m_dragItem = nullptr;
};
} // namespace

NoteTreePanel::NoteTreePanel(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
}

void NoteTreePanel::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *miniBar = new QHBoxLayout();
    m_btnAdd    = new QPushButton("+");
    m_btnDelete = new QPushButton("-");
    m_btnColor  = new QPushButton("Clr");

    m_btnAdd->setFixedWidth(30);
    m_btnDelete->setFixedWidth(30);
    m_btnColor->setFixedWidth(30);

    miniBar->addWidget(m_btnAdd);
    miniBar->addWidget(m_btnDelete);
    miniBar->addWidget(m_btnColor);
    miniBar->addStretch();

    m_tree = new DragTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addLayout(miniBar);
    layout->addWidget(m_tree);

    connect(m_btnAdd, &QPushButton::clicked, this, [this]() {
        emit requestCreateNotebook();
    });
    connect(m_btnDelete, &QPushButton::clicked, this, &NoteTreePanel::emitDeleteForCurrent);
    connect(m_btnColor, &QPushButton::clicked, this, &NoteTreePanel::colorCurrentItem);

    connect(m_tree, &QTreeWidget::currentItemChanged, this,
            [this](QTreeWidgetItem *current, QTreeWidgetItem *) {
                int ni, si, pi;
                if (indexFromItem(current, ni, si, pi))
                    emit selectionChanged(ni, si, pi);
            });

    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree, &QTreeWidget::customContextMenuRequested,
            this, &NoteTreePanel::handleContextMenu);

    auto *dragTree = qobject_cast<DragTreeWidget*>(m_tree);
    if (dragTree) {
        connect(dragTree, &DragTreeWidget::sectionDropped,
                this, &NoteTreePanel::requestMoveSection);
        connect(dragTree, &DragTreeWidget::pageDropped,
                this, &NoteTreePanel::requestMovePage);
    }
}

void NoteTreePanel::setNotebooks(const QVector<Notebook> &notebooks)
{
    m_notebooks = notebooks;
    rebuildTree();
}

bool NoteTreePanel::currentIndex(int &notebookIndex, int &sectionIndex, int &pageIndex) const
{
    return indexFromItem(m_tree->currentItem(), notebookIndex, sectionIndex, pageIndex);
}

void NoteTreePanel::rebuildTree()
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
            auto *secItem = new QTreeWidgetItem(QStringList(sec.title));
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
            }
            secItem->setExpanded(true);
        }
        nbItem->setExpanded(true);
    }

    // Select first available page if nothing selected.
    if (!m_notebooks.isEmpty()) {
        for (int ni = 0; ni < m_notebooks.size(); ++ni) {
            const auto &sections = m_notebooks[ni].sections;
            for (int si = 0; si < sections.size(); ++si) {
                const auto &pages = sections[si].pages;
                if (!pages.isEmpty()) {
                    QTreeWidgetItem *nbItem = m_tree->topLevelItem(ni);
                    QTreeWidgetItem *secItem = nbItem->child(si);
                    QTreeWidgetItem *pgItem = secItem->child(0);
                    m_tree->setCurrentItem(pgItem);
                    return;
                }
            }
        }
    }
}

bool NoteTreePanel::indexFromItem(QTreeWidgetItem *item, int &ni, int &si, int &pi) const
{
    if (!item)
        return false;

    ni = item->data(0, RoleNotebook).toInt();
    si = item->data(0, RoleSection).toInt();
    pi = item->data(0, RolePage).toInt();
    return (ni >= 0);
}

void NoteTreePanel::handleContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_tree->itemAt(pos);
    QMenu menu;

    if (!item) {
        menu.addAction("New Notebook", this, [this] { emit requestCreateNotebook(); });
    } else {
        int ni, si, pi;
        if (!indexFromItem(item, ni, si, pi))
            return;

        if (pi >= 0) {
            menu.addAction("Rename Page", this, [=] { emit requestRenamePage(ni, si, pi); });
            menu.addAction("Delete Page", this, [=] { emit requestDeletePage(ni, si, pi); });
        } else if (si >= 0) {
            menu.addAction("New Page", this, [=] { emit requestCreatePage(ni, si); });
            menu.addAction("Rename Section", this, [=] { emit requestRenameSection(ni, si); });
            menu.addAction("Delete Section", this, [=] { emit requestDeleteSection(ni, si); });
        } else {
            menu.addAction("New Section", this, [=] { emit requestCreateSection(ni); });
            menu.addAction("Rename Notebook", this, [=] { emit requestRenameNotebook(ni); });
            menu.addAction("Delete Notebook", this, [=] { emit requestDeleteNotebook(ni); });
        }
    }

    if (!menu.actions().isEmpty())
        menu.exec(m_tree->viewport()->mapToGlobal(pos));
}

void NoteTreePanel::emitDeleteForCurrent()
{
    int ni, si, pi;
    if (!currentIndex(ni, si, pi))
        return;

    if (pi >= 0) emit requestDeletePage(ni, si, pi);
    else if (si >= 0) emit requestDeleteSection(ni, si);
    else emit requestDeleteNotebook(ni);
}

void NoteTreePanel::colorCurrentItem()
{
    QTreeWidgetItem *item = m_tree->currentItem();
    if (!item)
        return;

    QColor color = QColorDialog::getColor(Qt::yellow, this, "Choose color");
    if (!color.isValid())
        return;

    item->setBackground(0, QBrush(color));
}

// #include "NoteTreePanel.moc"
