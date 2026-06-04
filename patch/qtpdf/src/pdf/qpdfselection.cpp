/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qpdfselection.h"
#include "qpdfselection_p.h"
#include <QGuiApplication>

QT_BEGIN_NAMESPACE

QPdfSelection::QPdfSelection()
  : d(new QPdfSelectionPrivate())
{
}

QPdfSelection::QPdfSelection(const QString &text, QVector<QPolygonF> bounds, QRectF boundingRect, int startIndex, int endIndex)
  : d(new QPdfSelectionPrivate(text, bounds, boundingRect, startIndex, endIndex))
{
}

QPdfSelection::QPdfSelection(QPdfSelectionPrivate *d)
  : d(d)
{
}

QPdfSelection::QPdfSelection(const QPdfSelection &other)
  : d(other.d)
{
}

QPdfSelection::QPdfSelection(QPdfSelection &&other) noexcept
  : d(std::move(other.d))
{
}

QPdfSelection::~QPdfSelection()
{
}

QPdfSelection &QPdfSelection::operator=(const QPdfSelection &other)
{
    d = other.d;
    return *this;
}

bool QPdfSelection::isValid() const
{
    return !d->bounds.isEmpty();
}

QVector<QPolygonF> QPdfSelection::bounds() const
{
    return d->bounds;
}

QString QPdfSelection::text() const
{
    return d->text;
}

QRectF QPdfSelection::boundingRectangle() const
{
    return d->boundingRect;
}

int QPdfSelection::startIndex() const
{
    return d->startIndex;
}

int QPdfSelection::endIndex() const
{
    return d->endIndex;
}

#if QT_CONFIG(clipboard)
void QPdfSelection::copyToClipboard(QClipboard::Mode mode) const
{
    QGuiApplication::clipboard()->setText(d->text, mode);
}
#endif

QT_END_NAMESPACE

#include "moc_qpdfselection.cpp"

