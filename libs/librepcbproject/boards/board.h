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

#ifndef LIBREPCB_PROJECT_BOARD_H
#define LIBREPCB_PROJECT_BOARD_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <QtWidgets>
#include <librepcbcommon/if_attributeprovider.h>
#include <librepcbcommon/fileio/if_xmlserializableobject.h>
#include <librepcbcommon/units/all_length_units.h>
#include <librepcbcommon/fileio/filepath.h>
#include <librepcbcommon/exceptions.h>
#include <librepcbcommon/uuid.h>
#include "../erc/if_ercmsgprovider.h"

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {

class GridProperties;
class GraphicsView;
class GraphicsScene;
class SmartXmlFile;
class BoardLayer;
class BoardDesignRules;

namespace project {

class NetSignal;
class Project;
class BI_Device;
class BI_Base;
class BI_FootprintPad;
class BI_Via;
class BI_NetPoint;
class BI_NetLine;
class BI_Polygon;
class BoardLayerStack;

/*****************************************************************************************
 *  Class Board
 ****************************************************************************************/

/**
 * @brief The Board class represents a PCB of a project and is always part of a circuit
 */
class Board final : public QObject, public IF_AttributeProvider,
                    public IF_ErcMsgProvider, public IF_XmlSerializableObject
{
        Q_OBJECT
        DECLARE_ERC_MSG_CLASS_NAME(Board)

    public:

        // Types

        /**
         * @brief Z Values of all items in a board scene (to define the stacking order)
         *
         * These values are used for QGraphicsItem::setZValue() to define the stacking
         * order of all items in a board QGraphicsScene. We use integer values, even
         * if the z-value of QGraphicsItem is a qreal attribute...
         *
         * Low number = background, high number = foreground
         */
        enum ItemZValue {
            ZValue_Default = 0,         ///< this is the default value (behind all other items)
            ZValue_FootprintsBottom,    ///< Z value for #project#BI_Footprint items
            ZValue_FootprintPadsBottom, ///< Z value for #project#BI_FootprintPad items
            ZValue_CopperBottom,
            ZValue_CopperTop,
            ZValue_FootprintPadsTop,    ///< Z value for #project#BI_FootprintPad items
            ZValue_FootprintsTop,       ///< Z value for #project#BI_Footprint items
            ZValue_Vias,                ///< Z value for #project#BI_Via items
        };

        // Constructors / Destructor
        Board() = delete;
        Board(const Board& other) = delete;
        Board(const Board& other, const FilePath& filepath, const QString& name) throw (Exception);
        Board(Project& project, const FilePath& filepath, bool restore, bool readOnly) throw (Exception) :
            Board(project, filepath, restore, readOnly, false, QString()) {}
        ~Board() noexcept;

        // Getters: General
        Project& getProject() const noexcept {return mProject;}
        const FilePath& getFilePath() const noexcept {return mFilePath;}
        const GridProperties& getGridProperties() const noexcept {return *mGridProperties;}
        BoardLayerStack& getLayerStack() noexcept {return *mLayerStack;}
        BoardDesignRules& getDesignRules() noexcept {return *mDesignRules;}
        const BoardDesignRules& getDesignRules() const noexcept {return *mDesignRules;}
        bool isEmpty() const noexcept;
        QList<BI_Base*> getSelectedItems(bool vias,
                                         bool footprintPads,
                                         bool floatingPoints,
                                         bool attachedPoints,
                                         bool floatingPointsFromFloatingLines,
                                         bool attachedPointsFromFloatingLines,
                                         bool floatingPointsFromAttachedLines,
                                         bool attachedPointsFromAttachedLines,
                                         bool attachedPointsFromFootprints,
                                         bool floatingLines,
                                         bool attachedLines,
                                         bool attachedLinesFromFootprints) const noexcept;
        QList<BI_Base*> getItemsAtScenePos(const Point& pos) const noexcept;
        QList<BI_Via*> getViasAtScenePos(const Point& pos, const NetSignal* netsignal) const noexcept;
        QList<BI_NetPoint*> getNetPointsAtScenePos(const Point& pos, const BoardLayer* layer,
                                                   const NetSignal* netsignal) const noexcept;
        QList<BI_NetLine*> getNetLinesAtScenePos(const Point& pos, const BoardLayer* layer,
                                                 const NetSignal* netsignal) const noexcept;
        QList<BI_FootprintPad*> getPadsAtScenePos(const Point& pos, const BoardLayer* layer,
                                                  const NetSignal* netsignal) const noexcept;
        QList<BI_Base*> getAllItems() const noexcept;

        // Setters: General
        void setGridProperties(const GridProperties& grid) noexcept;

        // Getters: Attributes
        const Uuid& getUuid() const noexcept {return mUuid;}
        const QString& getName() const noexcept {return mName;}
        const QIcon& getIcon() const noexcept {return mIcon;}

        // DeviceInstance Methods
        const QMap<Uuid, BI_Device*>& getDeviceInstances() const noexcept {return mDeviceInstances;}
        BI_Device* getDeviceInstanceByComponentUuid(const Uuid& uuid) const noexcept;
        void addDeviceInstance(BI_Device& instance) throw (Exception);
        void removeDeviceInstance(BI_Device& instance) throw (Exception);

        // Via Methods
        const QList<BI_Via*>& getVias() const noexcept {return mVias;}
        BI_Via* getViaByUuid(const Uuid& uuid) const noexcept;
        void addVia(BI_Via& via) throw (Exception);
        void removeVia(BI_Via& via) throw (Exception);

        // NetPoint Methods
        BI_NetPoint* getNetPointByUuid(const Uuid& uuid) const noexcept;
        void addNetPoint(BI_NetPoint& netpoint) throw (Exception);
        void removeNetPoint(BI_NetPoint& netpoint) throw (Exception);

        // NetLine Methods
        const QList<BI_NetLine*>& getNetLines() const noexcept {return mNetLines;}
        BI_NetLine* getNetLineByUuid(const Uuid& uuid) const noexcept;
        void addNetLine(BI_NetLine& netline) throw (Exception);
        void removeNetLine(BI_NetLine& netline) throw (Exception);

        // Polygon Methods
        const QList<BI_Polygon*>& getPolygons() const noexcept {return mPolygons;}
        void addPolygon(BI_Polygon& polygon) throw (Exception);
        void removePolygon(BI_Polygon& polygon) throw (Exception);

        // General Methods
        void addToProject() throw (Exception);
        void removeFromProject() throw (Exception);
        bool save(bool toOriginal, QStringList& errors) noexcept;
        void showInView(GraphicsView& view) noexcept;
        void saveViewSceneRect(const QRectF& rect) noexcept {mViewRect = rect;}
        const QRectF& restoreViewSceneRect() const noexcept {return mViewRect;}
        void setSelectionRect(const Point& p1, const Point& p2, bool updateItems) noexcept;
        void clearSelection() const noexcept;

        // Helper Methods
        bool getAttributeValue(const QString& attrNS, const QString& attrKey,
                               bool passToParents, QString& value) const noexcept;

        // Operator Overloadings
        Board& operator=(const Board& rhs) = delete;
        bool operator==(const Board& rhs) noexcept {return (this == &rhs);}
        bool operator!=(const Board& rhs) noexcept {return (this != &rhs);}

        // Static Methods
        static Board* create(Project& project, const FilePath& filepath,
                             const QString& name) throw (Exception);


    signals:

        /// @copydoc IF_AttributeProvider#attributesChanged()
        void attributesChanged();

        void deviceAdded(BI_Device& comp);
        void deviceRemoved(BI_Device& comp);


    private:

        Board(Project& project, const FilePath& filepath, bool restore,
              bool readOnly, bool create, const QString& newName) throw (Exception);
        void updateIcon() noexcept;

        /// @copydoc IF_XmlSerializableObject#checkAttributesValidity()
        bool checkAttributesValidity() const noexcept override;

        void updateErcMessages() noexcept;

        /// @copydoc IF_XmlSerializableObject#serializeToXmlDomElement()
        XmlDomElement* serializeToXmlDomElement() const throw (Exception) override;


        // General
        Project& mProject; ///< A reference to the Project object (from the ctor)
        FilePath mFilePath; ///< the filepath of the schematic *.xml file (from the ctor)
        QScopedPointer<SmartXmlFile> mXmlFile;
        bool mIsAddedToProject;

        QScopedPointer<GraphicsScene> mGraphicsScene;
        QScopedPointer<BoardLayerStack> mLayerStack;
        QScopedPointer<GridProperties> mGridProperties;
        QScopedPointer<BoardDesignRules> mDesignRules;
        QRectF mViewRect;

        // Attributes
        Uuid mUuid;
        QString mName;
        QIcon mIcon;

        // items
        QMap<Uuid, BI_Device*> mDeviceInstances;
        QList<BI_Via*> mVias;
        QList<BI_NetPoint*> mNetPoints;
        QList<BI_NetLine*> mNetLines;
        QList<BI_Polygon*> mPolygons;

        // ERC messages
        QHash<Uuid, ErcMsg*> mErcMsgListUnplacedComponentInstances;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb

#endif // LIBREPCB_PROJECT_BOARD_H
