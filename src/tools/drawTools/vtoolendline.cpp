/************************************************************************
 **
 **  @file   vtoolendline.cpp
 **  @author Roman Telezhinsky <dismine@gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vtoolendline.h"
#include "../../widgets/vmaingraphicsscene.h"
#include "../../container/calculator.h"
#include "../../dialogs/tools/dialogendline.h"

const QString VToolEndLine::ToolType = QStringLiteral("endLine");

VToolEndLine::VToolEndLine(VDomDocument *doc, VContainer *data, const qint64 &id,  const QString &typeLine,
                           const QString &formula, const qreal &angle, const qint64 &basePointId,
                           const Tool::Sources &typeCreation, QGraphicsItem *parent)
    :VToolLinePoint(doc, data, id, typeLine, formula, basePointId, angle, parent)
{

    if (typeCreation == Tool::FromGui)
    {
        AddToFile();
    }
    else
    {
        RefreshDataInFile();
    }
}

void VToolEndLine::setDialog()
{
    Q_CHECK_PTR(dialog);
    DialogEndLine *dialogTool = qobject_cast<DialogEndLine*>(dialog);
    Q_CHECK_PTR(dialogTool);
    const VPointF *p = VAbstractTool::data.GeometricObject<const VPointF *>(id);
    dialogTool->setTypeLine(typeLine);
    dialogTool->setFormula(formula);
    dialogTool->setAngle(angle);
    dialogTool->setBasePointId(basePointId, id);
    dialogTool->setPointName(p->name());
}

void VToolEndLine::Create(DialogTool *dialog, VMainGraphicsScene *scene, VDomDocument *doc,
                          VContainer *data)
{
    Q_CHECK_PTR(dialog);
    DialogEndLine *dialogTool = qobject_cast<DialogEndLine*>(dialog);
    Q_CHECK_PTR(dialogTool);
    QString pointName = dialogTool->getPointName();
    QString typeLine = dialogTool->getTypeLine();
    QString formula = dialogTool->getFormula();
    qreal angle = dialogTool->getAngle();
    qint64 basePointId = dialogTool->getBasePointId();
    Create(0, pointName, typeLine, formula, angle, basePointId, 5, 10, scene, doc, data, Document::FullParse,
           Tool::FromGui);
}

void VToolEndLine::Create(const qint64 _id, const QString &pointName, const QString &typeLine,
                          const QString &formula, const qreal &angle, const qint64 &basePointId,
                          const qreal &mx, const qreal &my, VMainGraphicsScene *scene, VDomDocument *doc,
                          VContainer *data, const Document::Documents &parse, const Tool::Sources &typeCreation)
{
    const VPointF *basePoint = data->GeometricObject<const VPointF *>(basePointId);
    QLineF line = QLineF(basePoint->toQPointF(), QPointF(basePoint->x()+100, basePoint->y()));
    Calculator cal(data);
    QString errorMsg;
    qreal result = cal.eval(formula, &errorMsg);
    if (errorMsg.isEmpty())
    {
        line.setLength(toPixel(result));
        line.setAngle(angle);
        qint64 id = _id;
        if (typeCreation == Tool::FromGui)
        {
            id = data->AddGObject(new VPointF(line.p2().x(), line.p2().y(), pointName, mx, my));
            data->AddLine(basePointId, id);
        }
        else
        {
            data->UpdateGObject(id, new VPointF(line.p2().x(), line.p2().y(), pointName, mx, my));
            data->AddLine(basePointId, id);
            if (parse != Document::FullParse)
            {
                doc->UpdateToolData(id, data);
            }
        }
        VDrawTool::AddRecord(id, Tool::EndLineTool, doc);
        if (parse == Document::FullParse)
        {
            VToolEndLine *point = new VToolEndLine(doc, data, id, typeLine, formula, angle,
                                                   basePointId, typeCreation);
            scene->addItem(point);
            connect(point, &VToolPoint::ChoosedTool, scene, &VMainGraphicsScene::ChoosedItem);
            connect(scene, &VMainGraphicsScene::NewFactor, point, &VToolPoint::SetFactor);
            doc->AddTool(id, point);
            doc->IncrementReferens(basePointId);
        }
    }
}

void VToolEndLine::FullUpdateFromFile()
{
    QDomElement domElement = doc->elementById(QString().setNum(id));
    if (domElement.isElement())
    {
        typeLine = domElement.attribute(AttrTypeLine, "");
        formula = domElement.attribute(AttrLength, "");
        basePointId = domElement.attribute(AttrBasePoint, "").toLongLong();
        angle = domElement.attribute(AttrAngle, "").toInt();
    }
    RefreshGeometry();
}

void VToolEndLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    ContextMenu<DialogEndLine>(this, event);
}

void VToolEndLine::ShowContextMenu(QGraphicsSceneContextMenuEvent *event)
{
    ContextMenu<DialogEndLine>(this, event);
}

void VToolEndLine::AddToFile()
{
    const VPointF *point = VAbstractTool::data.GeometricObject<const VPointF *>(id);
    QDomElement domElement = doc->createElement(TagName);

    SetAttribute(domElement, AttrId, id);
    SetAttribute(domElement, AttrType, ToolType);
    SetAttribute(domElement, AttrName, point->name());
    SetAttribute(domElement, AttrMx, toMM(point->mx()));
    SetAttribute(domElement, AttrMy, toMM(point->my()));

    SetAttribute(domElement, AttrTypeLine, typeLine);
    SetAttribute(domElement, AttrLength, formula);
    SetAttribute(domElement, AttrAngle, angle);
    SetAttribute(domElement, AttrBasePoint, basePointId);

    AddToCalculation(domElement);
}

void VToolEndLine::RefreshDataInFile()
{
    const VPointF *point = VAbstractTool::data.GeometricObject<const VPointF *>(id);
    QDomElement domElement = doc->elementById(QString().setNum(id));
    if (domElement.isElement())
    {
        SetAttribute(domElement, AttrName, point->name());
        SetAttribute(domElement, AttrMx, toMM(point->mx()));
        SetAttribute(domElement, AttrMy, toMM(point->my()));
        SetAttribute(domElement, AttrTypeLine, typeLine);
        SetAttribute(domElement, AttrLength, formula);
        SetAttribute(domElement, AttrAngle, angle);
        SetAttribute(domElement, AttrBasePoint, basePointId);
    }
}

void VToolEndLine::SaveDialog(QDomElement &domElement)
{
    Q_CHECK_PTR(dialog);
    DialogEndLine *dialogTool = qobject_cast<DialogEndLine*>(dialog);
    Q_CHECK_PTR(dialogTool);
    SetAttribute(domElement, AttrName, dialogTool->getPointName());
    SetAttribute(domElement, AttrTypeLine, dialogTool->getTypeLine());
    SetAttribute(domElement, AttrLength, dialogTool->getFormula());
    SetAttribute(domElement, AttrAngle, QString().setNum(dialogTool->getAngle()));
    SetAttribute(domElement, AttrBasePoint, QString().setNum(dialogTool->getBasePointId()));
}
