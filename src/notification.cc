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

#include "notification.hh"

#include <QAction>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

#include "main-window.hh"

#include <QDebug>

Notification::Notification(QWidget *p)
    : QDockWidget(p)
    , m_textEdit(new QTextEdit)
    , m_layout(new QVBoxLayout)
    , m_priority(LowPriority)
{
    setFeatures(QDockWidget::NoDockWidgetFeatures);
    setTitleBarWidget(NULL);
    setMaximumHeight(120);

    QIcon icon = QIcon::fromTheme("dialog-information");
    QLabel *label = new QLabel;
    label->setPixmap(icon.pixmap(48, 48));

    m_textEdit = new QTextEdit;
    m_textEdit->setReadOnly(true);
    setWindowFlags(Qt::FramelessWindowHint);

    QWidget *buttons = new QWidget;
    QPushButton *button = new QPushButton(tr("Close"));
    connect(button, SIGNAL(clicked()), this, SLOT(close()));
    m_layout->addWidget(button);
    buttons->setLayout(m_layout);

    QWidget *mainWidget = new QWidget;
    QLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(label);
    mainLayout->addWidget(m_textEdit);
    mainLayout->addWidget(buttons);
    mainWidget->setLayout(mainLayout);

    setWidget(mainWidget);
    changeBackground();
    parent()->addDockWidget(Qt::TopDockWidgetArea, this, Qt::Horizontal);
    hide();
}

MainWindow *Notification::parent() const
{
    return static_cast<MainWindow *>(QDockWidget::parent());
}

QString Notification::message() const { return m_textEdit->toPlainText(); }

void Notification::setMessage(const QString &str) { m_textEdit->setHtml(str); }

void Notification::addAction(QAction *action)
{
    QPushButton *button = new QPushButton(action->text());
    connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
    connect(button, SIGNAL(clicked()), this, SLOT(hide()));
    m_layout->insertWidget(0, button);
}

Notification::Priority Notification::priority() const { return m_priority; }

void Notification::setPriority(const Notification::Priority &value)
{
    m_priority = value;
    changeBackground();
}

void Notification::changeBackground()
{
    QColor color;
    switch (priority()) {
    case LowPriority:
        color.setRgb(255, 238, 170); // light yellow
        break;
    case MediumPriority:
        color.setRgb(255, 179, 128); // light orange
        break;
    case HighPriority:
        color.setRgb(233, 175, 175); // light red
        break;
    }
    setStyleSheet(
        QString(" QTextEdit, .QWidget { border: 0px; background-color: %1; }")
            .arg(color.name()));
}
