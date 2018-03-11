// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2011, Alexandre Dupas <alexandre.dupas@gmail.com>
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

#include "tab-widget.hh"

#include <QAction>
#include <QMouseEvent>
#include <QTabBar>

#include <QDebug>

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent), m_selectionBehaviorOnAdd(SelectCurrent)
{
    setDocumentMode(true);

    setStyleSheet(" QTabWidget::tab-bar {}");
    setTabBar(new TabBar(this));
    updateTabBarVisibility();

    QAction *action;
    action = new QAction(tr("Next tab"), this);
    action->setShortcut(QKeySequence::NextChild);
    connect(action, SIGNAL(triggered()), this, SLOT(next()));
    addAction(action);

    action = new QAction(tr("Previous tab"), this);
    action->setShortcut(QKeySequence::PreviousChild);
    connect(action, SIGNAL(triggered()), this, SLOT(prev()));
    addAction(action);

    action = new QAction(tr("Close tab"), this);
    action->setShortcut(QKeySequence::Close);
    connect(action, SIGNAL(triggered()), this, SLOT(closeTab()));
    addAction(action);
}

TabWidget::~TabWidget()
{
}

TabWidget::SelectionBehavior TabWidget::selectionBehaviorOnAdd() const
{
    return m_selectionBehaviorOnAdd;
}

void TabWidget::setSelectionBehaviorOnAdd(TabWidget::SelectionBehavior behavior)
{
    m_selectionBehaviorOnAdd = behavior;
}

void TabWidget::closeTab()
{
    emit(tabCloseRequested(currentIndex()));
}

void TabWidget::closeTab(int index)
{
    removeTab(index);

    updateTabBarVisibility();
}

int TabWidget::addTab(QWidget *widget)
{
    return addTab(widget, widget->windowTitle());
}

int TabWidget::addTab(QWidget *widget, const QString &label)
{
    int index = QTabWidget::addTab(widget, label);

    updateTabBarVisibility();

    if (selectionBehaviorOnAdd() == SelectNew)
        setCurrentIndex(index);

    return index;
}

void TabWidget::updateTabBarVisibility()
{
    if (count() > 1)
        tabBar()->show();
    else
        tabBar()->hide();
}

void TabWidget::next()
{
    if (currentIndex() == count() - 1) // last tab
        setCurrentIndex(0);            // first tab
    else
        setCurrentIndex(currentIndex() + 1);
}

void TabWidget::prev()
{
    if (currentIndex() == 0)          // first tab
        setCurrentIndex(count() - 1); // last tab
    else
        setCurrentIndex(currentIndex() - 1);
}

void TabWidget::changeTabText(const QString &text)
{
    QWidget *widget = qobject_cast<QWidget *>(QObject::sender());
    int index = indexOf(widget);

    if (index >= 0)
        setTabText(index, text);
}

//----------------------------------------------------------------------------

TabBar::TabBar(QWidget *parent) : QTabBar(parent)
{
}

TabBar::~TabBar()
{
}

void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton) {
        emit(tabCloseRequested(tabAt(event->pos())));
    }
    QTabBar::mouseReleaseEvent(event);
}
