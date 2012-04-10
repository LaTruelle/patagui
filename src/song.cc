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
#include "song.hh"

#include "utils/utils.hh"

#include <QFile>
#include <QFileInfo>

#include <QDebug>

QRegExp Song::reSgFile("(.*)\\\\begin\\{?song\\}?\\{([^\\}]+)\\}[^[]*\\[([^]]*)\\](.*)\\s*\\\\endsong.*");
QRegExp Song::reSong("\\\\begin\\{?song\\}?\\{([^[\\}]+)\\}[^[]*\\[([^]]*)\\]");
QRegExp Song::reArtist("by=([^,]+)");
QRegExp Song::reAlbum("album=([^,]+)");
QRegExp Song::reCoverName("cov=([^,]+)");
QRegExp Song::reLilypond("\\\\lilypond");
QRegExp Song::reLanguage("\\\\selectlanguage\\{([^\\}]+)");
QRegExp Song::reColumnCount("\\\\songcolumns\\{([^\\}]+)");
QRegExp Song::reCapo("\\\\capo\\{([^\\}]+)");
QRegExp Song::reCover("\\\\cover");
QRegExp Song::reBlankLine("^\\s*$");
QRegExp Song::reGtab("\\\\gtab\\{([^\\}]+\\}\\{[^\\}]+)");

QRegExp Song::reBegin("\\\\begin");
QRegExp Song::reEnd("\\\\end");
QRegExp Song::reBeginVerse("\\\\begin\\{verse\\}");
QRegExp Song::reEndVerse("\\\\end\\{verse\\}");
QRegExp Song::reBeginChorus("\\\\begin\\{chorus\\}");
QRegExp Song::reEndChorus("\\\\end\\{chorus\\}");
QRegExp Song::reBeginScripture("\\\\begin\\{scripture\\}");
QRegExp Song::reEndScripture("\\\\end\\{scripture\\}");

Song Song::fromFile(const QString &path)
{
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      qWarning() << "Song::fromFile: unable to open " << path;
      return Song();
    }

  QTextStream stream (&file);
  stream.setCodec("UTF-8");
  QString fileStr = stream.readAll();
  file.close();

  return Song::fromString(fileStr, path);
}

Song Song::fromString(const QString &text, const QString &path)
{
  Song song;
  reSgFile.indexIn(text);

  QString prefix = reSgFile.cap(1);
  QString options = reSgFile.cap(3);
  QString content = reSgFile.cap(4);

  // path
  song.path = path;

  // path (for cover)
  song.coverPath = QFileInfo(path).absolutePath();

  reColumnCount.indexIn(prefix);
  song.columnCount = reColumnCount.cap(1).toInt();

  // title
  song.title = reSgFile.cap(2);

  // options
  reArtist.indexIn(options);
  song.artist = SbUtils::latexToUtf8(reArtist.cap(1));

  reAlbum.indexIn(options);
  song.album = SbUtils::latexToUtf8(reAlbum.cap(1));

  reCoverName.indexIn(options);
  song.coverName = reCoverName.cap(1);

  // content
  song.isLilypond = QBool(reLilypond.indexIn(content) > -1);
  song.coverPath = QFileInfo(path).absolutePath();

  //locale
  reLanguage.indexIn(prefix);
  song.locale = QLocale(languageFromString(reLanguage.cap(1)), QLocale::AnyCountry);

  song.capo = 0;

  QStringList lines = content.split("\n");
  QString line;
  bool preliminaryFinished = false;
  foreach (line, lines)
    {
      if (!preliminaryFinished)
        {
          if (reCapo.indexIn(line) != -1)
            {
              song.capo = reCapo.cap(1).toInt();
              continue;
            }
          else if (reGtab.indexIn(line) != -1)
            {
              song.gtabs << reGtab.cap(1);
              continue;
            }
          else if (reCover.indexIn(line) != -1 || reBlankLine.indexIn(line) != -1)
            {
              continue;
            }
          else
            {
              preliminaryFinished = true;
            }
        }
      if (preliminaryFinished)
        {
          song.lyrics << line;
        }
    }
  // remove blank line at the end of input
  while (song.lyrics.last().trimmed().isEmpty())
    {
      song.lyrics.removeLast();
    }

  return song;
}

QString Song::toString(const Song &song)
{
  QString text;
  text.append(QString("\\selectlanguage{%1}\n").arg(Song::languageToString(song.locale.language())));
  if (song.columnCount > 0)
    text.append(QString("\\songcolumns{%1}\n").arg(song.columnCount));

  text.append(QString("\\beginsong{%1}\n  [by=%2").arg(song.title).arg(song.artist));

  if (!song.coverName.isEmpty())
    text.append(QString(",cov=%1").arg(song.coverName));

  if (!song.album.isEmpty())
    text.append(QString(",album=%1").arg(song.album));

  text.append(QString("]\n\n"));

  if (!song.coverName.isEmpty())
    text.append(QString("  \\cover\n"));

  if (song.capo > 0)
    text.append(QString("  \\capo{%1}\n").arg(song.capo));

  foreach (QString gtab, song.gtabs)
    {
      text.append(QString("  \\gtab{%1}\n").arg(gtab));
    }

  text.append(QString("\n"));

  foreach (QString lyric, song.lyrics)
    {
      text.append(QString("%1\n").arg(lyric));
    }

  text.append(QString("\\endsong\n"));

  return text;
}

QLocale::Language Song::languageFromString(const QString &languageName)
{
  if (languageName == "french")
    return QLocale::French;
  else if (languageName == "english")
    return QLocale::English;
  else if (languageName == "spanish")
    return QLocale::Spanish;
  else if (languageName == "portuguese")
    return QLocale::Portuguese;

  return QLocale::system().language();
}

QString Song::languageToString(const QLocale::Language language)
{
  switch (language)
    {
    case QLocale::French:
      return "french";
    case QLocale::English:
      return "english";
    case QLocale::Spanish:
      return "spanish";
    case QLocale::Portuguese:
      return "portuguese";
    default:
      return "english";
    }
}
