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
#include "preferences.hh"

#include <QtGui>
#include <QtGroupBoxPropertyBrowser>

#ifdef ENABLE_LIBRARY_DOWNLOAD
#include <QNetworkProxy>
#endif // ENABLE_LIBRARY_DOWNLOAD

#include "main-window.hh"
#include "songbook.hh"
#include "library.hh"
#include "file-chooser.hh"

#include <QDebug>

// Config Dialog

ConfigDialog::ConfigDialog(CMainWindow* parent)
  : QDialog(parent)
  , m_parent(parent)
{
  m_contentsWidget = new QListWidget;
  m_contentsWidget->setViewMode(QListView::IconMode);
  m_contentsWidget->setIconSize(QSize(62, 62));
  m_contentsWidget->setMovement(QListView::Static);
  m_contentsWidget->setMinimumHeight(500);
  m_contentsWidget->setMaximumWidth(110);
  m_contentsWidget->setSpacing(12);
  m_contentsWidget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::MinimumExpanding);

  m_pagesWidget = new QStackedWidget;
  m_pagesWidget->addWidget(new OptionsPage(this));
  m_pagesWidget->addWidget(new SongbookPage(this));
  m_pagesWidget->addWidget(new DisplayPage(this));
  m_pagesWidget->addWidget(new EditorPage(this));
#ifdef ENABLE_LIBRARY_DOWNLOAD
  m_pagesWidget->addWidget(new NetworkPage(this));
#endif // ENABLE_LIBRARY_DOWNLOAD

  QPushButton *closeButton = new QPushButton(tr("Close"));

  createIcons();
  m_contentsWidget->setCurrentRow(0);

  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QHBoxLayout *horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(m_contentsWidget);
  horizontalLayout->addWidget(m_pagesWidget, 1);

  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  buttonsLayout->addStretch(1);
  buttonsLayout->addWidget(closeButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(horizontalLayout);
  mainLayout->addSpacing(12);
  mainLayout->addLayout(buttonsLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Preferences"));
}

CMainWindow* ConfigDialog::parent() const
{
  return m_parent;
}

void ConfigDialog::createIcons()
{
  QListWidgetItem *optionsButton = new QListWidgetItem(m_contentsWidget);
  optionsButton->setIcon(QIcon::fromTheme("preferences-system", QIcon(":/icons/tango/48x48/categories/preferences-system.png")));
  optionsButton->setText(tr("Options"));
  optionsButton->setTextAlignment(Qt::AlignHCenter);
  optionsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *songbookButton = new QListWidgetItem(m_contentsWidget);
  songbookButton->setIcon(QIcon(":/icons/songbook/256x256/book.png"));
  songbookButton->setText(tr("Songbook"));
  songbookButton->setTextAlignment(Qt::AlignHCenter);
  songbookButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *displayButton = new QListWidgetItem(m_contentsWidget);
  displayButton->setIcon(QIcon::fromTheme("preferences-desktop", QIcon(":/icons/tango/48x48/categories/preferences-desktop.png")));
  displayButton->setText(tr("Display"));
  displayButton->setTextAlignment(Qt::AlignHCenter);
  displayButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *editorButton = new QListWidgetItem(m_contentsWidget);
  editorButton->setIcon(QIcon::fromTheme("accessories-text-editor", QIcon(":/icons/tango/48x48/apps/accessories-text-editor.png")));
  editorButton->setText(tr("Editor"));
  editorButton->setTextAlignment(Qt::AlignHCenter);
  editorButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#ifdef ENABLE_LIBRARY_DOWNLOAD
  QListWidgetItem *networkButton = new QListWidgetItem(m_contentsWidget);
  networkButton->setIcon(QIcon::fromTheme("preferences-system-network", QIcon(":/icons/tango/48x48/categories/preferences-system-network.png")));
  networkButton->setText(tr("Network"));
  networkButton->setTextAlignment(Qt::AlignHCenter);
  networkButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif // ENABLE_LIBRARY_DOWNLOAD

  connect(m_contentsWidget,
          SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

void ConfigDialog::changePage(QListWidgetItem *current,
                              QListWidgetItem *previous)
{
  if (!current)
    current = previous;

  m_pagesWidget->setCurrentIndex(m_contentsWidget->row(current));
}

void ConfigDialog::closeEvent(QCloseEvent *event)
{
  for( int i = 0 ; i < m_pagesWidget->count() ; ++i )
    {
      m_pagesWidget->widget(i)->close();
    }
}


// Page

Page::Page(ConfigDialog *configDialog)
  : QScrollArea(configDialog)
  , m_content(new QWidget)
  , m_configDialog(configDialog)
{
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

ConfigDialog * Page::configDialog() const
{
  return m_configDialog;
}

void Page::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}

void Page::readSettings()
{}

void Page::writeSettings()
{}

void Page::setLayout(QLayout *layout)
{
  m_content->setLayout(layout);
  setWidget(m_content);
}


// Display Page

DisplayPage::DisplayPage(ConfigDialog *configDialog)
  : Page(configDialog)
{
  QGroupBox *displayColumnsGroupBox = new QGroupBox(tr("Display Columns"));

  m_titleCheckBox = new QCheckBox(tr("Title"));
  m_artistCheckBox = new QCheckBox(tr("Artist"));
  m_pathCheckBox = new QCheckBox(tr("Path"));
  m_albumCheckBox = new QCheckBox(tr("Album"));
  m_lilypondCheckBox = new QCheckBox(tr("Lilypond"));
  m_langCheckBox = new QCheckBox(tr("Language"));

  QVBoxLayout *displayColumnsLayout = new QVBoxLayout;
  displayColumnsLayout->addWidget(m_titleCheckBox);
  displayColumnsLayout->addWidget(m_artistCheckBox);
  displayColumnsLayout->addWidget(m_pathCheckBox);
  displayColumnsLayout->addWidget(m_albumCheckBox);
  displayColumnsLayout->addWidget(m_lilypondCheckBox);
  displayColumnsLayout->addWidget(m_langCheckBox);
  displayColumnsGroupBox->setLayout(displayColumnsLayout);

  QGroupBox *displayWindowsGroupBox = new QGroupBox(tr("Display windows"));
  m_compilationLogCheckBox = new QCheckBox(tr("Compilation log"));

  QVBoxLayout *displayWindowsLayout = new QVBoxLayout;
  displayWindowsLayout->addWidget(m_compilationLogCheckBox);
  displayWindowsGroupBox->setLayout(displayWindowsLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(displayColumnsGroupBox);
  mainLayout->addWidget(displayWindowsGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);

  readSettings();
}

void DisplayPage::readSettings()
{
  QSettings settings;
  settings.beginGroup("display");
  m_titleCheckBox->setChecked(settings.value("title", true).toBool());
  m_artistCheckBox->setChecked(settings.value("artist", true).toBool());
  m_pathCheckBox->setChecked(settings.value("path", false).toBool());
  m_albumCheckBox->setChecked(settings.value("album", true).toBool());
  m_lilypondCheckBox->setChecked(settings.value("lilypond", false).toBool());
  m_langCheckBox->setChecked(settings.value("lang", true).toBool());
  m_compilationLogCheckBox->setChecked(settings.value("logs", false).toBool());
  settings.endGroup();
}

void DisplayPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("display");
  settings.setValue("title", m_titleCheckBox->isChecked());
  settings.setValue("artist", m_artistCheckBox->isChecked());
  settings.setValue("path", m_pathCheckBox->isChecked());
  settings.setValue("album", m_albumCheckBox->isChecked());
  settings.setValue("lilypond", m_lilypondCheckBox->isChecked());
  settings.setValue("lang", m_langCheckBox->isChecked());
  settings.setValue("logs", m_compilationLogCheckBox->isChecked());
  settings.endGroup();
}


// Option Page

OptionsPage::OptionsPage(ConfigDialog *configDialog)
  : Page(configDialog)
  , m_workingPath(0)
  , m_workingPathValid(0)
  , m_buildCommand(0)
  , m_cleanCommand(0)
  , m_cleanallCommand(0)
{
  m_workingPathValid = new QLabel;

  m_workingPath = new CFileChooser();
  m_workingPath->setFileMode(QFileDialog::Directory);
  m_workingPath->setOptions(QFileDialog::ShowDirsOnly);
  m_workingPath->setCaption(tr("Songbook path"));

  connect(m_workingPath, SIGNAL(pathChanged(const QString&)),
          this, SLOT(checkWorkingPath(const QString&)));

  m_buildCommand = new QLineEdit(this);
  m_cleanCommand = new QLineEdit(this);
  m_cleanallCommand = new QLineEdit(this);

  readSettings();

  // working path
  QGroupBox *workingPathGroupBox
    = new QGroupBox(tr("Directory for Patacrep Songbook"));

  checkWorkingPath(m_workingPath->path());

  QLayout *workingPathLayout = new QVBoxLayout;
  workingPathLayout->addWidget(m_workingPath);
  workingPathLayout->addWidget(m_workingPathValid);
  workingPathGroupBox->setLayout(workingPathLayout);

  // external tools
  QGroupBox *toolsGroupBox
    = new QGroupBox(tr("External tools"));

  QBoxLayout *toolsLayout = new QVBoxLayout;
  QBoxLayout *buildCommandLayout = new QHBoxLayout;
  QPushButton *buildCommandResetButton = new QPushButton(tr("reset"), this);
  buildCommandLayout->addWidget(m_buildCommand);
  buildCommandLayout->addWidget(buildCommandResetButton);
  toolsLayout->addLayout(buildCommandLayout);
  QBoxLayout *cleanCommandLayout = new QHBoxLayout;
  QPushButton *cleanCommandResetButton = new QPushButton(tr("reset"), this);
  cleanCommandLayout->addWidget(m_cleanCommand);
  cleanCommandLayout->addWidget(cleanCommandResetButton);
  toolsLayout->addLayout(cleanCommandLayout);
  QBoxLayout *cleanallCommandLayout = new QHBoxLayout;
  QPushButton *cleanallCommandResetButton = new QPushButton(tr("reset"), this);
  cleanallCommandLayout->addWidget(m_cleanallCommand);
  cleanallCommandLayout->addWidget(cleanallCommandResetButton);
  toolsLayout->addLayout(cleanallCommandLayout);
  toolsGroupBox->setLayout(toolsLayout);

  connect(buildCommandResetButton, SIGNAL(clicked()), SLOT(resetBuildCommand()));
  connect(cleanCommandResetButton, SIGNAL(clicked()), SLOT(resetCleanCommand()));
  connect(cleanallCommandResetButton, SIGNAL(clicked()), SLOT(resetCleanallCommand()));

  // main layout
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(workingPathGroupBox);
  mainLayout->addWidget(toolsGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

void OptionsPage::readSettings()
{
  QSettings settings;
  settings.beginGroup("library");
  m_workingPath->setPath(settings.value("workingPath", QDir::homePath()).toString());
  settings.endGroup();

  settings.beginGroup("tools");
  m_buildCommand->setText(settings.value("buildCommand", PLATFORM_BUILD_COMMAND).toString());
  m_cleanCommand->setText(settings.value("cleanCommand", PLATFORM_CLEAN_COMMAND).toString());
  m_cleanallCommand->setText(settings.value("cleanallCommand", PLATFORM_CLEANALL_COMMAND).toString());
  settings.endGroup();
}

void OptionsPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("library");
  settings.setValue("workingPath", m_workingPath->path());
  settings.endGroup();

  settings.beginGroup("tools");
  settings.setValue("buildCommand", m_buildCommand->text());
  settings.setValue("cleanCommand", m_cleanCommand->text());
  settings.setValue("cleanallCommand", m_cleanallCommand->text());
  settings.endGroup();
}

void OptionsPage::checkWorkingPath(const QString &path)
{
  QDir directory(path);

  bool error = true;
  bool warning = true;

  QString message;

  if (!directory.exists())
    {
      message = tr("the directory does not exist");
    }
  else if (!directory.exists("makefile"))
    {
      message = tr("makefile not found");
    }
  else if (!directory.exists("songbook.py"))
    {
      message = tr("songbook software (songbook.py) not found");
    }
  else if (!directory.exists("songs"))
    {
      message = tr("songs/ directory not found");
    }
  else if (!directory.exists("img"))
    {
      message = tr("img/ directory not found");
    }
  else if (!directory.exists("lilypond"))
    {
      error = false;
      message = tr("lilypond/ directory not found");
    }
  else if (!directory.exists("utils"))
    {
      error = false;
      message = tr("utils/ directory not found");
    }
  else
    {
      error = false;
      warning = false;
      message = tr("The directory is valid");
    }

  QString mask("<font color=%1>%2%3.</font>");
  if (error)
    {
      mask = mask.arg("red").arg(tr("Error: "));
    }
  else if (warning)
    {
      mask = mask.arg("orange").arg(tr("Warning: "));
    }
  else
    {
      mask = mask.arg("green").arg("");
    }
  m_workingPathValid->setText(mask.arg(message));
}

void OptionsPage::resetBuildCommand()
{
  m_buildCommand->setText(PLATFORM_BUILD_COMMAND);
}

void OptionsPage::resetCleanCommand()
{
  m_cleanCommand->setText(PLATFORM_CLEAN_COMMAND);
}

void OptionsPage::resetCleanallCommand()
{
  m_cleanallCommand->setText(PLATFORM_CLEANALL_COMMAND);
}

// Editor Page

EditorPage::EditorPage(ConfigDialog *configDialog)
  : Page(configDialog)
  , m_numberLinesCheckBox(new QCheckBox(tr("Display line numbers")))
  , m_highlightCurrentLineCheckBox(new QCheckBox(tr("Highlight current line")))
  , m_fontButton(new QPushButton)
{
  readSettings();

  if(m_fontstr.isEmpty())
    {
      m_font = QFont("Monospace",11);
      m_font.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
    }

  updateFontButton();
  connect(m_fontButton, SIGNAL(clicked()), this, SLOT(selectFont()));

  QFormLayout* layout = new QFormLayout;
  layout->addRow(m_numberLinesCheckBox);
  layout->addRow(m_highlightCurrentLineCheckBox);
  layout->addRow(tr("Font:"), m_fontButton);

  m_verseColor = QColor(138,226,52).lighter(170);
  m_verse.setStyleSheet(QString("background-color: %1;").arg(m_verseColor.name()));
  m_chorusColor = QColor(252,175,62).lighter(140);
  m_chorus.setStyleSheet(QString("background-color: %1;").arg(m_chorusColor.name()));
  m_scriptureColor = QColor(173,127,168).lighter(150);
  m_scripture.setStyleSheet(QString("background-color: %1;").arg(m_scriptureColor.name()));

  connect(&m_verse, SIGNAL(clicked()), this, SLOT(selectVerseColor()));
  connect(&m_chorus, SIGNAL(clicked()), this, SLOT(selectChorusColor()));
  connect(&m_scripture, SIGNAL(clicked()), this, SLOT(selectScriptureColor()));

  layout->addRow(tr("Highlight verse color:"), &m_verse);
  layout->addRow(tr("Highlight chorus color:"), &m_chorus);
  layout->addRow(tr("Highlight scripture color:"), &m_scripture);

  setLayout(layout);
}

void EditorPage::readSettings()
{
  QSettings settings;
  settings.beginGroup("editor");
  m_numberLinesCheckBox->setChecked(settings.value("lines", true).toBool());
  m_highlightCurrentLineCheckBox->setChecked(settings.value("highlight", true).toBool());
  m_fontstr = settings.value("font", QString()).toString();
  if(!m_fontstr.isEmpty())
    m_font.fromString(m_fontstr);

  m_verseColor = settings.value("verseColor").value<QColor>();
  m_verseColor = settings.value("chorusColor").value<QColor>();
  m_verseColor = settings.value("scriptureColor").value<QColor>();

  settings.endGroup();
}

void EditorPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("editor");
  settings.setValue("lines", m_numberLinesCheckBox->isChecked());
  settings.setValue("highlight", m_highlightCurrentLineCheckBox->isChecked());
  settings.setValue("font", m_font.toString());
  settings.setValue("verseColor", m_verseColor);
  settings.setValue("chorusColor", m_chorusColor);
  settings.setValue("scriptureColor", m_scriptureColor);
  settings.endGroup();
}

void EditorPage::selectFont()
{
  bool ok;
  m_font = QFontDialog::getFont(&ok, m_font, this);
  if(ok) updateFontButton();
}

void EditorPage::selectVerseColor()
{
  m_verseColor = QColorDialog::getColor(m_verseColor, this);
  m_verse.setStyleSheet(QString("background-color: %1;").arg(m_verseColor.name()));
}

void EditorPage::selectChorusColor()
{
  m_chorusColor = QColorDialog::getColor(m_chorusColor, this);
  m_chorus.setStyleSheet(QString("background-color: %1;").arg(m_chorusColor.name()));
}

void EditorPage::selectScriptureColor()
{
  m_scriptureColor = QColorDialog::getColor(m_scriptureColor, this);
  m_scripture.setStyleSheet(QString("background-color: %1;").arg(m_scriptureColor.name()));
}

void EditorPage::updateFontButton()
{
  m_fontButton->setText(QString("%1 %2")
			.arg(QFontInfo(m_font).family())
			.arg(QString::number(QFontInfo(m_font).pointSize())));
}


#ifdef ENABLE_LIBRARY_DOWNLOAD
// Network Page

NetworkPage::NetworkPage(ConfigDialog *configDialog)
  : Page(configDialog)
  , m_hostname()
  , m_port()
  , m_user()
  , m_password()
{
  m_hostname = new QLineEdit;
  m_port = new QSpinBox;
  m_port->setRange(0, 65535);
  m_user = new QLineEdit;
  m_password = new QLineEdit;
  m_password->setEchoMode(QLineEdit::Password);

  readSettings();

  // check application
  QGroupBox *proxyGroupBox
    = new QGroupBox(tr("Proxy settings"));

  QFormLayout *proxyLayout = new QFormLayout;
  proxyLayout->addRow(tr("Hostname:"), m_hostname);
  proxyLayout->addRow(tr("Port:"), m_port);
  proxyLayout->addRow(tr("User:"), m_user);
  proxyLayout->addRow(tr("Password:"), m_password);
  proxyGroupBox->setLayout(proxyLayout);

  // main layout
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(proxyGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

void NetworkPage::readSettings()
{
  QSettings settings;
  settings.beginGroup("proxy");
  m_hostname->setText(settings.value("hostname", QString()).toString());
  m_port->setValue(settings.value("port", 0).toInt());
  m_user->setText(settings.value("user", QString()).toString());
  m_password->setText(settings.value("password", QString()).toString());
  settings.endGroup();
}

void NetworkPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("proxy");
  settings.setValue("hostname", m_hostname->text());
  settings.setValue("port", m_port->value());
  settings.setValue("user", m_user->text());
  settings.setValue("password", m_password->text());
  settings.endGroup();

  QNetworkProxy proxy;
  if (m_hostname->text().isEmpty())
    {
      proxy.setType(QNetworkProxy::NoProxy);
    }
  else
    {
      proxy.setType(QNetworkProxy::HttpProxy);
      proxy.setHostName(m_hostname->text());
      proxy.setPort(m_port->value());
      proxy.setUser(m_user->text());
      proxy.setPassword(m_password->text());
    }
  QNetworkProxy::setApplicationProxy(proxy);
}
#endif // ENABLE_LIBRARY_DOWNLOAD

// Songbook Page

SongbookPage::SongbookPage(ConfigDialog *configDialog)
  : Page(configDialog)
  , m_propertyEditor(new QtGroupBoxPropertyBrowser)
{
  CSongbook *songbook = configDialog->parent()->songbook();

  QComboBox* templateComboBox = new QComboBox;
  templateComboBox->addItems(songbook->library()->templates());

  int index = songbook->library()->templates().indexOf(songbook->tmpl());
  if(index == -1)
    index = songbook->library()->templates().indexOf("patacrep.tmpl");
  templateComboBox->setCurrentIndex(index);

  connect(templateComboBox, SIGNAL(currentIndexChanged(const QString &)),
	  songbook, SLOT(setTmpl(const QString &)));
  connect(songbook, SIGNAL(wasModified(bool)), SLOT(updatePropertyEditor()));

  songbook->changeTemplate(songbook->tmpl());
  songbook->initializeEditor(m_propertyEditor);

  QGroupBox *songbookGroupBox = new QGroupBox(tr("Songbook"));

  QBoxLayout *templateLayout = new QHBoxLayout;
  templateLayout->addWidget(new QLabel(tr("Template:")));
  templateLayout->addWidget(templateComboBox);

  QBoxLayout *songbookLayout = new QVBoxLayout;
  songbookLayout->addLayout(templateLayout);
  songbookLayout->addWidget(m_propertyEditor);
  songbookGroupBox->setLayout(songbookLayout);

  // main layout
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(songbookGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

void SongbookPage::updatePropertyEditor()
{
  configDialog()->parent()->songbook()->initializeEditor(m_propertyEditor);
}
