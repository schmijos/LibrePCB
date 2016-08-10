/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * http://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "repository.h"
#include "fileio/xmldomelement.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

Repository::Repository(const Repository& other) noexcept :
    mUrl(other.mUrl)
{
}

Repository::Repository(const QUrl& url) throw (Exception)
{
    setUrl(url);
}

Repository::Repository(const XmlDomElement& domElement) throw (Exception)
{
    mUrl = domElement.getAttribute<QUrl>("url", true);
}

Repository::~Repository() noexcept
{
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

void Repository::setUrl(const QUrl& url) throw (Exception)
{
    if (url.isValid()) {
        mUrl = url;
    } else {
        throw RuntimeError(__FILE__, __LINE__, QString(),
            QString(tr("The URL is invalid: %1")).arg(url.errorString()));
    }
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

XmlDomElement* Repository::serializeToXmlDomElement() const throw (Exception)
{
    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);

    QScopedPointer<XmlDomElement> root(new XmlDomElement("repository"));
    root->setAttribute("url", mUrl);
    return root.take();
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

bool Repository::checkAttributesValidity() const noexcept
{
    if (!mUrl.isValid()) return false;
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb
