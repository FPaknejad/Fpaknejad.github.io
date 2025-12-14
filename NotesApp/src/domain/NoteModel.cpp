#include "NoteModel.hpp"
#include "NoteTypes.hpp"

#include <algorithm>

using namespace handnote::core;

namespace handnote::core {

NoteModel::NoteModel(QObject *parent)
    : QObject(parent)
{
}

QVector<Notebook> &NoteModel::notebooks()
{
    return m_notebooks;
}

const QVector<Notebook> &NoteModel::notebooks() const
{
    return m_notebooks;
}

// ---------------------------------------------------------
// Helpers: find by QUuid or QString
// ---------------------------------------------------------

static QUuid toUuid(const QString &id)
{
    return QUuid::fromString(id);
}

Notebook *NoteModel::findNotebookById(const QString &id)
{
    const QUuid uuid = toUuid(id);
    for (auto &nb : m_notebooks) {
        if (nb.id == uuid)
            return &nb;
    }
    return nullptr;
}

const Notebook *NoteModel::findNotebookById(const QString &id) const
{
    const QUuid uuid = toUuid(id);
    for (const auto &nb : m_notebooks) {
        if (nb.id == uuid)
            return &nb;
    }
    return nullptr;
}

Section *NoteModel::findSectionById(const QString &id)
{
    const QUuid uuid = toUuid(id);
    for (auto &nb : m_notebooks) {
        for (auto &sec : nb.sections) {
            if (sec.id == uuid)
                return &sec;
        }
    }
    return nullptr;
}

Page *NoteModel::findPageById(const QString &id)
{
    const QUuid uuid = toUuid(id);
    for (auto &nb : m_notebooks) {
        for (auto &sec : nb.sections) {
            for (auto &page : sec.pages) {
                if (page.id == uuid)
                    return &page;
            }
        }
    }
    return nullptr;
}

// ---------------------------------------------------------
// CRUD: Notebook
// ---------------------------------------------------------

QString NoteModel::createNotebook(const QString &name)
{
    Notebook nb;
    nb.id = QUuid::createUuid();
    nb.name = name;
    m_notebooks.append(nb);

    emit notebookListChanged();
    return nb.id.toString(QUuid::WithoutBraces);
}

bool NoteModel::renameNotebook(const QString &id, const QString &newName)
{
    if (auto *nb = findNotebookById(id)) {
        nb->name = newName;
        emit notebookListChanged();
        return true;
    }
    return false;
}

bool NoteModel::deleteNotebook(const QString &id)
{
    const QUuid uuid = toUuid(id);
    auto it = std::remove_if(m_notebooks.begin(), m_notebooks.end(),
                             [&](const Notebook &n) { return n.id == uuid; });
    if (it != m_notebooks.end()) {
        m_notebooks.erase(it, m_notebooks.end());
        emit notebookListChanged();
        return true;
    }
    return false;
}

// ---------------------------------------------------------
// CRUD: Section
// ---------------------------------------------------------

QString NoteModel::createSection(const QString &notebookId, const QString &title)
{
    if (auto *nb = findNotebookById(notebookId)) {
        Section sec;
        sec.id = QUuid::createUuid();
        sec.title = title;
        nb->sections.append(sec);

        emit notebookStructureChanged();
        return sec.id.toString(QUuid::WithoutBraces);
    }
    return {};
}

bool NoteModel::renameSection(const QString &id, const QString &newTitle)
{
    if (auto *sec = findSectionById(id)) {
        sec->title = newTitle;
        emit notebookStructureChanged();
        return true;
    }
    return false;
}

bool NoteModel::deleteSection(const QString &id)
{
    const QUuid uuid = toUuid(id);

    bool removed = false;
    for (auto &nb : m_notebooks) {
        auto it = std::remove_if(nb.sections.begin(), nb.sections.end(),
                                 [&](const Section &s) { return s.id == uuid; });
        if (it != nb.sections.end()) {
            nb.sections.erase(it, nb.sections.end());
            removed = true;
        }
    }

    if (removed)
        emit notebookStructureChanged();

    return removed;
}

bool NoteModel::moveSection(const QString &sectionId, const QString &destNotebookId, int destIndex)
{
    const QUuid secUuid = toUuid(sectionId);
    const QUuid destNbUuid = toUuid(destNotebookId);

    int srcNotebookIdx = -1;
    int srcSectionIdx = -1;
    Section movedSection;

    // Extract the section from its current notebook.
    for (int ni = 0; ni < m_notebooks.size(); ++ni) {
        auto &nb = m_notebooks[ni];
        for (int si = 0; si < nb.sections.size(); ++si) {
            if (nb.sections[si].id == secUuid) {
                movedSection = nb.sections.takeAt(si);
                srcNotebookIdx = ni;
                srcSectionIdx = si;
                break;
            }
        }
        if (srcNotebookIdx >= 0)
            break;
    }

    if (srcNotebookIdx < 0)
        return false;

    // Insert into the destination notebook.
    for (auto &nb : m_notebooks) {
        if (nb.id == destNbUuid) {
            const int sectionCount = nb.sections.size();
            int insertAt = destIndex;
            if (insertAt < 0)
                insertAt = 0;
            else if (insertAt > sectionCount)
                insertAt = sectionCount;
            nb.sections.insert(insertAt, movedSection);
            emit notebookStructureChanged();
            return true;
        }
    }

    // If destination not found, put it back to avoid data loss.
    m_notebooks[srcNotebookIdx].sections.insert(srcSectionIdx, movedSection);
    return false;
}

// ---------------------------------------------------------
// CRUD: Page
// ---------------------------------------------------------

QString NoteModel::createPage(const QString &sectionId, const QString &title,
                              const PageTemplate &templateInfo)
{
    const QUuid uuid = toUuid(sectionId);

    for (auto &nb : m_notebooks) {
        for (auto &sec : nb.sections) {
            if (sec.id == uuid) {
                Page page;
                page.id = QUuid::createUuid();
                page.title = title;
                page.templateInfo = templateInfo;
                sec.pages.append(page);

                emit notebookStructureChanged();
                return page.id.toString(QUuid::WithoutBraces);
            }
        }
    }
    return {};
}

bool NoteModel::renamePage(const QString &id, const QString &newTitle)
{
    if (auto *page = findPageById(id)) {
        page->title = newTitle;
        emit notebookStructureChanged();
        return true;
    }
    return false;
}

bool NoteModel::deletePage(const QString &id)
{
    const QUuid uuid = toUuid(id);

    bool removed = false;
    for (auto &nb : m_notebooks) {
        for (auto &sec : nb.sections) {
            auto it = std::remove_if(sec.pages.begin(), sec.pages.end(),
                                     [&](const Page &p) { return p.id == uuid; });
            if (it != sec.pages.end()) {
                sec.pages.erase(it, sec.pages.end());
                removed = true;
            }
        }
    }

    if (removed)
        emit notebookStructureChanged();

    return removed;
}

bool NoteModel::movePage(const QString &pageId, const QString &destSectionId, int destIndex)
{
    const QUuid pageUuid = toUuid(pageId);
    const QUuid destSecUuid = toUuid(destSectionId);

    Section *srcSection = nullptr;
    int srcPageIdx = -1;
    Page movedPage;

    for (auto &nb : m_notebooks) {
        for (auto &sec : nb.sections) {
            for (int pi = 0; pi < sec.pages.size(); ++pi) {
                if (sec.pages[pi].id == pageUuid) {
                    movedPage = sec.pages.takeAt(pi);
                    srcSection = &sec;
                    srcPageIdx = pi;
                    break;
                }
            }
            if (srcSection)
                break;
        }
        if (srcSection)
            break;
    }

    if (!srcSection)
        return false;

    for (auto &nb : m_notebooks) {
        for (auto &sec : nb.sections) {
            if (sec.id == destSecUuid) {
                const int pageCount = sec.pages.size();
                int insertAt = destIndex;
                if (insertAt < 0)
                    insertAt = 0;
                else if (insertAt > pageCount)
                    insertAt = pageCount;
                sec.pages.insert(insertAt, movedPage);
                emit notebookStructureChanged();
                return true;
            }
        }
    }

    // Destination not found; restore to avoid losing data.
    srcSection->pages.insert(srcPageIdx, movedPage);
    return false;
}

bool NoteModel::updatePageText(const QString &id, const QString &text)
{
    if (auto *page = findPageById(id)) {
        page->textBlocks.clear();

        if (!text.isEmpty()) {
            TextBlock block;
            block.id = QUuid::createUuid();
            block.text = text;
            block.boundingBox = QRectF();
            block.lineIndex = 0;
            page->textBlocks.append(block);
        }
        emit notebookStructureChanged();
        return true;
    }
    return false;
}

bool NoteModel::replacePageStrokes(const QString &id, const QVector<Stroke> &strokes)
{
    if (auto *page = findPageById(id)) {
        page->strokes = strokes;
        emit notebookStructureChanged();
        return true;
    }
    return false;
}

bool NoteModel::setPageTemplate(const QString &id, const PageTemplate &templateInfo)
{
    if (auto *page = findPageById(id)) {
        page->templateInfo = templateInfo;
        emit notebookStructureChanged();
        return true;
    }
    return false;
}

bool NoteModel::setHandwritingConfig(const QString &id, const HandwritingConfig &cfg)
{
    if (auto *page = findPageById(id)) {
        page->handwriting = cfg;
        emit notebookStructureChanged();
        return true;
    }
    return false;
}

// ---------------------------------------------------------
// Access helpers
// ---------------------------------------------------------

QString NoteModel::notebookName(const QString &id) const
{
    if (auto *nb = const_cast<NoteModel*>(this)->findNotebookById(id)) {
        return nb->name;
    }
    return {};
}

QString NoteModel::sectionTitle(const QString &id) const
{
    const QUuid uuid = toUuid(id);
    for (const auto &nb : m_notebooks) {
        for (const auto &sec : nb.sections) {
            if (sec.id == uuid)
                return sec.title;
        }
    }
    return {};
}

QString NoteModel::pageTitle(const QString &id) const
{
    const QUuid uuid = toUuid(id);
    for (const auto &nb : m_notebooks) {
        for (const auto &sec : nb.sections) {
            for (const auto &page : sec.pages) {
                if (page.id == uuid)
                    return page.title;
            }
        }
    }
    return {};
}

} // namespace handnote::core
