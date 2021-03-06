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
#include "cmdremoveviafromboard.h"
#include <librepcbcommon/scopeguard.h>
#include <librepcbproject/boards/board.h>
#include <librepcbproject/boards/items/bi_via.h>
#include <librepcbproject/boards/cmd/cmdboardviaremove.h>
#include "cmddetachboardnetpointfromviaorpad.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

CmdRemoveViaFromBoard::CmdRemoveViaFromBoard(BI_Via& via) noexcept :
    UndoCommandGroup(tr("Remove via from board")),
    mVia(via)
{
}

CmdRemoveViaFromBoard::~CmdRemoveViaFromBoard() noexcept
{
}

/*****************************************************************************************
 *  Inherited from UndoCommand
 ****************************************************************************************/

bool CmdRemoveViaFromBoard::performExecute() throw (Exception)
{
    // if an error occurs, undo all already executed child commands
    auto undoScopeGuard = scopeGuard([&](){performUndo();});

    // detach all used netpoints && remove all unused netpoints + netlines
    foreach (BI_NetPoint* netpoint, mVia.getNetPoints()) { Q_ASSERT(netpoint);
        execNewChildCmd(new CmdDetachBoardNetPointFromViaOrPad(*netpoint)); // can throw
    }

    // remove the via itself
    execNewChildCmd(new CmdBoardViaRemove(mVia)); // can throw

    undoScopeGuard.dismiss(); // no undo required
    return (getChildCount() > 0);
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
