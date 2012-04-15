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
#ifndef __SONG_EDITOR_HH__
#define __SONG_EDITOR_HH__

#include "config.hh"
#include "code-editor.hh"

#include <QToolBar>
#include <QList>

class QAction;
class QActionGroup;
class Hunspell;
class CHighlighter;
class FindReplaceDialog;

class CSongEditor : public CodeEditor
{
  Q_OBJECT

public:
  CSongEditor();
  ~CSongEditor();

  QString path();
  void setPath(const QString & APath);

  QToolBar* toolBar() const;

  virtual void keyPressEvent(QKeyEvent *event);

  //void addAction(QAction*);
  QActionGroup * actionGroup() const;

  void readSettings();
  void writeSettings();
  QStringList getWordPropositions(const QString &word);
  Hunspell* checker() const;
  void installHighlighter();

  bool isSpellCheckingEnabled() const;
  void setSpellCheckingEnabled(const bool);

signals:
  void labelChanged(const QString &label);
  void wordAdded(const QString &word);

private slots:
  //write modifications of the textEdit into sg file.
  void save();
  void documentWasModified();
  void insertVerse();
  void insertChorus();

#ifdef ENABLE_SPELLCHECK
  void correctWord();
  void addWord();
  void ignoreWord();

protected:
  void contextMenuEvent(QContextMenuEvent *event);
  QString currentWord();
#endif //ENABLE_SPELLCHECK

private:
  QString syntaxicColoration(const QString &);
  void indentSelection();
  void indentLine(const QTextCursor & cursor);
  void trimLine(const QTextCursor & cursor);

  QToolBar* m_toolBar;
  QString m_path;
  QActionGroup *m_actions;
  CHighlighter* m_highlighter;

  QAction* m_spellCheckingAct;
  bool m_isSpellCheckingEnabled;
#ifdef ENABLE_SPELLCHECK
  QList<QAction *> m_misspelledWordsActs;
  QPoint m_lastPos;
  QStringList m_addedWords;
  uint m_maxSuggestedWords;
  QString m_dictionary;
#endif //ENABLE_SPELLCHECK

  FindReplaceDialog* m_findReplaceDialog;

};

#endif // __SONG_EDITOR_HH__
