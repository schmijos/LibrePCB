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
#include "librarymanager.h"
#include "ui_librarymanager.h"
#include <librepcblibrary/library.h>
#include "../../workspace.h"
#include "../../settings/workspacesettings.h"
#include "../workspacelibrary.h"
#include "librarylistwidgetitem.h"
#include "libraryinfowidget.h"
#include "addlibrarywidget.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace workspace {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

LibraryManager::LibraryManager(Workspace& ws, QWidget* parent) noexcept :
    QMainWindow(parent), mWorkspace(ws), mUi(new Ui::LibraryManager),
    mCurrentWidget(nullptr)
{
    mUi->setupUi(this);
    connect(mUi->btnScanLibrary, &QPushButton::clicked,
            this, &LibraryManager::rescanLibraryButtonClicked);
    connect(mUi->btnClose,&QPushButton::clicked,
            this, &QMainWindow::close);
    connect(mUi->lstLibraries, &QListWidget::currentItemChanged,
            this, &LibraryManager::currentListItemChanged);

    mAddLibraryWidget.reset(new AddLibraryWidget(mWorkspace));
    mUi->horizontalLayout->addWidget(mAddLibraryWidget.data());
    connect(mAddLibraryWidget.data(), &AddLibraryWidget::libraryAdded,
            this, &LibraryManager::libraryAddedSlot);

    loadLibraries();
}

LibraryManager::~LibraryManager() noexcept
{
    clearLibraryList();
    mAddLibraryWidget.reset();
    mUi.reset();
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void LibraryManager::rescanLibraryButtonClicked() noexcept
{
    try {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        int count = mWorkspace.getLibrary().rescan();
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this, tr("Rescan Library"),
            QString("Successfully scanned %1 library elements.").arg(count));
    } catch (Exception& e) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, tr("Error"), e.getUserMsg());
    }

    // update library list
    clearLibraryList();
    loadLibraries();
}

void LibraryManager::clearLibraryList() noexcept
{
    mUi->lstLibraries->clear();
    qDeleteAll(mLibraryListWidgets);    mLibraryListWidgets.clear();
    qDeleteAll(mLibraries);             mLibraries.clear();
}

void LibraryManager::loadLibraries() noexcept
{
    try {
        // add the "Add new library" item
        mLibraries.insert(QString(), nullptr);
        mLibraryListWidgets.insert(nullptr, new LibraryListWidgetItem(mWorkspace, nullptr));

        // add all existing libraries
        QHash<FilePath, Uuid> libs = mWorkspace.getLibrary().getAllLibraries();
        foreach (const FilePath& fp, libs.keys()) {
            QScopedPointer<library::Library> lib(new library::Library(fp, true));
            QScopedPointer<LibraryListWidgetItem> widget(new LibraryListWidgetItem(mWorkspace, lib.data()));
            mLibraries.insert(widget->getName(), lib.data());
            mLibraryListWidgets.insert(lib.take(), widget.take());
        }
        foreach (library::Library* lib, mLibraries) {
            LibraryListWidgetItem* widget = mLibraryListWidgets.value(lib); Q_ASSERT(widget);
            QListWidgetItem* item = new QListWidgetItem(mUi->lstLibraries);
            item->setSizeHint(widget->sizeHint());
            item->setData(Qt::UserRole, QVariant::fromValue(lib));
            mUi->lstLibraries->setItemWidget(item, widget);
        }

        // select the first item in the list
        mUi->lstLibraries->setCurrentRow(0);
    } catch (Exception& e) {
        QMessageBox::critical(this, tr("Error"), e.getUserMsg());
    }
}

void LibraryManager::currentListItemChanged(QListWidgetItem* current, QListWidgetItem* previous) noexcept
{
    Q_UNUSED(previous);

    if (mCurrentWidget) {
        delete mCurrentWidget;
        mCurrentWidget = nullptr;
    }

    if (current) {
        library::Library* lib = current->data(Qt::UserRole).value<library::Library*>();
        if (lib) {
            mCurrentWidget = new LibraryInfoWidget(mWorkspace, *lib);
            mUi->horizontalLayout->addWidget(mCurrentWidget);
        }
    } else {
        mCurrentWidget = new QWidget();
        mUi->horizontalLayout->addWidget(mCurrentWidget);
    }

    mAddLibraryWidget->setVisible(mCurrentWidget ? false : true);
}

void LibraryManager::libraryAddedSlot(const FilePath& filepath) noexcept
{
    try {
        QScopedPointer<library::Library> lib(new library::Library(filepath, true));
        QScopedPointer<LibraryListWidgetItem> widget(new LibraryListWidgetItem(mWorkspace, lib.data()));
        mLibraries.insert(widget->getName(), lib.data());
        int itemindex = mLibraries.keys().indexOf(widget->getName());

        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(widget->sizeHint());
        item->setData(Qt::UserRole, QVariant::fromValue(lib.data()));
        mUi->lstLibraries->insertItem(itemindex, item);
        mUi->lstLibraries->setItemWidget(item, widget.data());
        mUi->lstLibraries->setCurrentRow(itemindex);
        mLibraryListWidgets.insert(lib.take(), widget.take());
    } catch (Exception& e) {
        QMessageBox::critical(this, tr("Error"), e.getUserMsg());
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace workspace
} // namespace librepcb
