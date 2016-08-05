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
#include <QtWidgets>
#include "librarylistwidgetitem.h"
#include "ui_librarylistwidgetitem.h"
#include <librepcblibrary/library.h>
#include "../../workspace.h"
#include "../../settings/workspacesettings.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace workspace {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

LibraryListWidgetItem::LibraryListWidgetItem(const Workspace& ws,
                                             const library::Library* lib) noexcept :
    QWidget(nullptr), mUi(new Ui::LibraryListWidgetItem)
{
    mUi->setupUi(this);

    if (lib) {
        const QStringList& localeOrder = ws.getSettings().getLibLocaleOrder().getLocaleOrder();
        if (!lib->getImage().isNull()) {
            mUi->lblImage->setPixmap(lib->getImage().scaled(mUi->lblImage->size(),
                Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        mUi->lblLibraryName->setText(lib->getName(localeOrder));
        mUi->lblLibraryDescription->setText(lib->getDescription(localeOrder));
        mUi->lblLibraryUrl->setText(lib->getFilePath().getFilename());
    } else {
        QPixmap image(":/img/actions/add.png");
        mUi->lblImage->setPixmap(image.scaled(mUi->lblImage->size(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
        mUi->lblLibraryName->setText(tr("Add a new library"));
        mUi->lblLibraryDescription->setText(tr("Click here to add a new library."));
        mUi->lblLibraryUrl->setText("");
    }

    if (mUi->lblLibraryDescription->text().isEmpty()) {
        mUi->lblLibraryDescription->setVisible(false);
    }
}

LibraryListWidgetItem::~LibraryListWidgetItem() noexcept
{
}

/*****************************************************************************************
 *  Getters
 ****************************************************************************************/

QString LibraryListWidgetItem::getName() const noexcept
{
    return mUi->lblLibraryName->text();
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace workspace
} // namespace librepcb
