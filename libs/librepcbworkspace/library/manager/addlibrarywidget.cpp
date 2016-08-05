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
#include "addlibrarywidget.h"
#include "ui_addlibrarywidget.h"
#include <librepcbcommon/systeminfo.h>
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

AddLibraryWidget::AddLibraryWidget(Workspace& ws) noexcept :
    QWidget(nullptr), mWorkspace(ws), mUi(new Ui::AddLibraryWidget)
{
    mUi->setupUi(this);
    connect(mUi->btnLocalCreate, &QPushButton::clicked,
            this, &AddLibraryWidget::createLocalLibraryButtonClicked);
    connect(mUi->edtLocalName, &QLineEdit::textChanged,
            this, &AddLibraryWidget::localLibraryNameLineEditTextChanged);

    // set placeholder texts
    mUi->edtLocalName->setPlaceholderText("My Library");
    mUi->edtLocalAuthor->setPlaceholderText(SystemInfo::getFullUsername());
    mUi->edtLocalVersion->setPlaceholderText("0.1");
    localLibraryNameLineEditTextChanged(mUi->edtLocalName->text());
}

AddLibraryWidget::~AddLibraryWidget() noexcept
{
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void AddLibraryWidget::localLibraryNameLineEditTextChanged(QString name) noexcept
{
    // TODO: allow only valid directory names!
    if (name.isEmpty()) name = mUi->edtLocalName->placeholderText();
    QString dirname = name.replace(" ", "_").append(".lplib");
    mUi->edtLocalDirectory->setPlaceholderText(dirname);
}

void AddLibraryWidget::createLocalLibraryButtonClicked() noexcept
{
    // get attributes
    QString name = getTextOrPlaceholderFromQLineEdit(mUi->edtLocalName);
    QString desc = getTextOrPlaceholderFromQLineEdit(mUi->edtLocalDescription);
    QString author = getTextOrPlaceholderFromQLineEdit(mUi->edtLocalAuthor);
    QString versionStr = getTextOrPlaceholderFromQLineEdit(mUi->edtLocalVersion);
    Version version(versionStr);
    QString directoryStr = getTextOrPlaceholderFromQLineEdit(mUi->edtLocalDirectory);
    if ((!directoryStr.isEmpty()) && (!directoryStr.endsWith(".lplib"))) {
        directoryStr.append(".lplib");
    }
    FilePath directory = mWorkspace.getLibraryPath().getPathTo(directoryStr);
    bool gitInitRepo = mUi->cbxLocalGitInit->isChecked();

    // check attributes validity
    if (name.isEmpty()) {
        QMessageBox::critical(this, tr("Invalid Input"), tr("Please enter a name."));
        return;
    }
    if (author.isEmpty()) {
        QMessageBox::critical(this, tr("Invalid Input"), tr("Please enter an author."));
        return;
    }
    if (!version.isValid()) {
        QMessageBox::critical(this, tr("Invalid Input"), tr("Please enter a valid version number."));
        return;
    }
    if (directoryStr.isEmpty()) {
        QMessageBox::critical(this, tr("Invalid Input"), tr("Please enter a directory name."));
        return;
    }
    if (directory.isExistingFile() || directory.isExistingDir()) {
        QMessageBox::critical(this, tr("Invalid Input"), tr("The specified directory exists already."));
        return;
    }

    try {
        // create the new library
        QScopedPointer<library::Library> lib(new library::Library(
            Uuid::createRandom(), version, author, name, desc, QString("")));
        lib->saveTo(directory);

        // create git repository if required
        if (gitInitRepo) {
            // TODO
        }

        emit libraryAdded(directory);
    } catch (Exception& e) {
        QMessageBox::critical(this, tr("Error"), e.getUserMsg());
    }
}

/*****************************************************************************************
 *  Private Static Methods
 ****************************************************************************************/

QString AddLibraryWidget::getTextOrPlaceholderFromQLineEdit(QLineEdit* edit) noexcept
{
    if (edit) {
        QString text = edit->text().trimmed();
        QString placeholder = edit->placeholderText().trimmed();
        return (text.length() > 0) ? text : placeholder;
    } else {
        return QString();
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace workspace
} // namespace librepcb
