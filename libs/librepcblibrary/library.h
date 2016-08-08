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

#ifndef LIBREPCB_LIBRARY_LIBRARY_H
#define LIBREPCB_LIBRARY_LIBRARY_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <QtWidgets>
#include "librarybaseelement.h"

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {
namespace library {

/*****************************************************************************************
 *  Class Library
 ****************************************************************************************/

/**
 * @brief   The Library class represents a library directory (*.lplib)
 *
 * @author  ubruhin
 * @date    2016-08-03
 */
class Library final : public LibraryBaseElement
{
        Q_OBJECT

    public:

        // Constructors / Destructor
        Library() = delete;
        Library(const Library& other) = delete;
        Library(const Uuid& uuid, const Version& version, const QString& author,
                const QString& name_en_US, const QString& description_en_US,
                const QString& keywords_en_US) throw (Exception);
        Library(const FilePath& libDir, bool readOnly) throw (Exception);
        ~Library() noexcept;

        // Getters
        const QPixmap& getImage() const noexcept {return mImage;}

        // Operator Overloadings
        Library& operator=(const Library& rhs) = delete;


    private: // Data

        QPixmap mImage;

};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace library
} // namespace librepcb

#endif // LIBREPCB_LIBRARY_LIBRARY_H
