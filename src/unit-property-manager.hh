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
#ifndef __UNIT_PROPERTY_MANAGER_HH__
#define __UNIT_PROPERTY_MANAGER_HH__

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtIntPropertyManager>
#include <QtVariantProperty>
#include <QtGroupBoxPropertyBrowser>


class CUnitPropertyManager : public QtIntPropertyManager
{
  Q_OBJECT
  public:
  CUnitPropertyManager(QObject *parent = 0);
  ~CUnitPropertyManager();

  QString suffix(const QtProperty *property) const;
  QString valueText(const QtProperty *property) const;
  static int id(){return 127;};

public slots:
  void setSuffix(QtProperty *property, const QString &suffix);

signals:
  void suffixChanged(QtProperty *property, const QString &suffix);

protected:
  virtual void initializeProperty(QtProperty *property);
  virtual void uninitializeProperty(QtProperty *property);

private:
  struct Data {
    QString suffix;
  };
  QMap<const QtProperty *, Data> propertyToData;
};

#endif // __UNIT_PROPERTY_MANAGER_HH__
