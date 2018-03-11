// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//******************************************************************************

#include "diagram-area.hh"

#include "chord-list-model.hh"
#include "diagram-editor.hh"

#include <QAction>
#include <QBoxLayout>
#include <QHeaderView>
#include <QList>
#include <QMenu>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTableView>

#include <QDebug>

DiagramArea::DiagramArea(QWidget *parent)
    : QWidget(parent), m_isReadOnly(false), m_addDiagramButton(new QPushButton)
{
    QBoxLayout *addButtonLayout = new QVBoxLayout;
    m_addDiagramButton->setFlat(true);
    m_addDiagramButton->setToolTip(tr("Add a new diagram"));
    m_addDiagramButton->setIcon(QIcon::fromTheme(
        "list-add", QIcon(":/icons/tango/48x48/actions/list-add.png")));
    connect(m_addDiagramButton, SIGNAL(clicked()), this, SLOT(newDiagram()));
    addButtonLayout->addStretch();
    addButtonLayout->addWidget(m_addDiagramButton);

    // diagram model
    m_diagramModel = new ChordListModel();

    // proxy model (filtering)
    m_proxyModel = new QSortFilterProxyModel;
    m_proxyModel->setSourceModel(m_diagramModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1);

    // diagram view
    m_diagramView = new QTableView;
    m_diagramView->setModel(m_proxyModel);
    m_diagramView->verticalHeader()->hide();
    m_diagramView->horizontalHeader()->hide();
    m_diagramView->setStyleSheet(
        "QTableView::item { border: 0px; padding: 10px;}");
    m_diagramView->setShowGrid(false);
    m_diagramView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_diagramView, SIGNAL(clicked(const QModelIndex &)), this,
            SLOT(onViewClicked(const QModelIndex &)));
    connect(m_diagramModel, SIGNAL(layoutChanged()), this, SLOT(resizeRows()));

    connect(this, SIGNAL(layoutChanged()), this, SLOT(resizeRows()));
    connect(this, SIGNAL(readOnlyModeChanged()), this, SLOT(update()));

    QBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_diagramView, 1);
    mainLayout->addLayout(addButtonLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);

    update();
}

void DiagramArea::update()
{
    if (isReadOnly()) {
        m_addDiagramButton->setVisible(false);
        m_diagramView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_diagramView->setDragEnabled(false);
        m_diagramView->setAcceptDrops(false);
        m_diagramView->setDropIndicatorShown(false);
        m_diagramView->setDragDropMode(QAbstractItemView::NoDragDrop);

        disconnect(m_diagramView, SIGNAL(doubleClicked(const QModelIndex &)), 0,
                   0);
        disconnect(m_diagramView,
                   SIGNAL(customContextMenuRequested(const QPoint &)), 0, 0);
    } else {
        m_addDiagramButton->setVisible(true);
        m_diagramView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_diagramView->setDragEnabled(true);
        m_diagramView->setAcceptDrops(true);
        m_diagramView->setDropIndicatorShown(true);
        m_diagramView->setDragDropMode(QAbstractItemView::InternalMove);

        connect(m_diagramView, SIGNAL(doubleClicked(const QModelIndex &)), this,
                SLOT(editDiagram(const QModelIndex &)));
        connect(m_diagramView,
                SIGNAL(customContextMenuRequested(const QPoint &)), this,
                SLOT(contextMenu(const QPoint &)));
    }
}

void DiagramArea::onViewClicked(const QModelIndex &index)
{
    if (index.isValid())
        emit(diagramClicked(
            m_diagramModel->getChord(m_proxyModel->mapToSource(index))));
}

void DiagramArea::resizeRows()
{
    for (int i = 0; i < m_diagramModel->rowCount(); ++i)
        m_diagramView->setRowHeight(i, 120);
}

void DiagramArea::newDiagram()
{
    editDiagram(QModelIndex());
}

void DiagramArea::editDiagram(QModelIndex index)
{
    Q_ASSERT(!isReadOnly());

    if (!index.isValid())
        index = m_diagramView->indexAt(
            m_diagramView->mapFromGlobal(QCursor::pos()));

    bool newChord = !index.isValid();

    Chord *chord =
        newChord ? new Chord
                 : m_diagramModel->getChord(m_proxyModel->mapToSource(index));

    DiagramEditor dialog(this);
    dialog.setChord(chord);

    if (dialog.exec() == QDialog::Accepted) {
        if (newChord)
            addDiagram(chord->toString());
        else
            m_diagramModel->setData(m_proxyModel->mapToSource(index),
                                    chord->toString());

        emit(contentsChanged());
    }
}

void DiagramArea::addDiagram(const QString &chord)
{
    m_diagramModel->addItem(chord);
}

void DiagramArea::removeDiagram(QModelIndex index)
{
    if (!index.isValid())
        index = m_diagramView->indexAt(
            m_diagramView->mapFromGlobal(QCursor::pos()));

    if (!index.isValid() || isReadOnly())
        return;

    m_diagramModel->removeItem(m_proxyModel->mapToSource(index));
}

void DiagramArea::onDiagramChanged()
{
    Q_ASSERT(isReadOnly());
    emit(contentsChanged());
}

bool DiagramArea::isReadOnly() const
{
    return m_isReadOnly;
}

void DiagramArea::setReadOnly(bool value)
{
    if (m_isReadOnly != value) {
        m_isReadOnly = value;
        emit(readOnlyModeChanged());
    }
}

void DiagramArea::contextMenu(const QPoint &pos)
{
    Q_UNUSED(pos);

    QMenu menu;

    QAction *action = new QAction(tr("Edit"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(editDiagram()));
    menu.addAction(action);

    action = new QAction(tr("Remove"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(removeDiagram()));
    menu.addAction(action);

    menu.exec(QCursor::pos());
}

void DiagramArea::setTypeFilter(const Chord::Instrument &type)
{
    m_proxyModel->setFilterRole(Qt::DisplayRole);
    if (type == Chord::Guitar)
        m_proxyModel->setFilterRegExp("gtab");
    else
        m_proxyModel->setFilterRegExp("utab");
    emit(layoutChanged());
}

void DiagramArea::setNameFilter(const QString &name)
{
    clearFilters();
    m_proxyModel->setFilterRole(ChordListModel::NameRole);
    m_proxyModel->setFilterRegExp(name);
    emit(layoutChanged());
}

void DiagramArea::setStringsFilter(const QString &strings)
{
    m_proxyModel->setFilterRole(ChordListModel::StringsRole);
    m_proxyModel->setFilterRegExp(strings);
    emit(layoutChanged());
}

void DiagramArea::clearFilters()
{
    m_proxyModel->setFilterRole(Qt::DisplayRole);
    m_proxyModel->setFilterRegExp("");
    emit(layoutChanged());
}

void DiagramArea::setColumnCount(int value)
{
    m_diagramModel->setColumnCount(value);
}

void DiagramArea::setRowCount(int value)
{
    m_diagramModel->setRowCount(value);
}

QList<Chord *> DiagramArea::chords()
{
    QList<Chord *> list;
    for (int i = 0; i < m_diagramModel->rowCount(); ++i)
        for (int j = 0; j < m_diagramModel->columnCount(); ++j) {
            list << m_diagramModel->getChord(m_diagramModel->index(i, j));
        }

    return list;
}
