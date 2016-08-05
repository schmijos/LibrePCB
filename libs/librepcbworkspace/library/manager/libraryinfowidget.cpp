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
#include "libraryinfowidget.h"
#include "ui_libraryinfowidget.h"
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

LibraryInfoWidget::LibraryInfoWidget(const Workspace& ws, const library::Library& lib) noexcept :
    QWidget(nullptr), mUi(new Ui::LibraryInfoWidget)
{
    mUi->setupUi(this);

    const QStringList& localeOrder = ws.getSettings().getLibLocaleOrder().getLocaleOrder();

    if (!lib.getImage().isNull()) {
        mUi->lblImage->setPixmap(lib.getImage().scaledToHeight(mUi->lblImage->height(),
                                                               Qt::SmoothTransformation));
    } else {
        mUi->lblImage->setVisible(false);
        mUi->line->setVisible(false);
    }
    mUi->lblName->setText(lib.getName(localeOrder));
    mUi->lblDescription->setText(lib.getDescription(localeOrder));
    mUi->lblVersion->setText(lib.getVersion().toStr());
    mUi->lblAuthor->setText(lib.getAuthor());
    mUi->lblUuid->setText(lib.getUuid().toStr());
    mUi->lblDirectory->setText(QString("<a href='%1'>%2</a>").arg(
        lib.getFilePath().toQUrl().toLocalFile(), lib.getFilePath().getFilename()));
    mUi->lblDirectory->setToolTip(lib.getFilePath().toNative());
}

LibraryInfoWidget::~LibraryInfoWidget()
{
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace workspace
} // namespace librepcb
