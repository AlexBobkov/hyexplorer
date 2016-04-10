/* HyExplorer - hyperspectral images management system
* Copyright (c) 2015-2016 HyExplorer team
* http://virtualglobe.ru/hyexplorer/
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "EventHandlers.hpp"

#include <osgViewer/View>

#include <QDebug>

using namespace osgEarth;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;
using namespace portal;

ReportMoveMouseHandler::ReportMoveMouseHandler(osgEarth::MapNode* mapNode,
                                               const CallbackType &pcb) :
                                               osgGA::GUIEventHandler(),
                                               _mapNode(mapNode),
                                               _cb(pcb)
{

}

bool ReportMoveMouseHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = static_cast<osgViewer::View*>(aa.asView());

    if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
    {
        osg::Vec3d world;
        if (_mapNode->getTerrain()->getWorldCoordsUnderMouse(aa.asView(), ea.getX(), ea.getY(), world))
        {
            osgEarth::GeoPoint mapPoint;
            mapPoint.fromWorld(_mapNode->getMapSRS(), world);

            _cb(mapPoint);
        }
    }

    return false;
}

//====================================================

ReportClickMouseHandler::ReportClickMouseHandler(osgEarth::MapNode* mapNode,
                                                 const SuccessCallbackType& scb,
                                                 const FailureCallbackType& fcb) :
                                                 osgGA::GUIEventHandler(),
                                                 _mapNode(mapNode),
                                                 _scb(scb),
                                                 _fcb(fcb),
                                                 _mouseX(0.0),
                                                 _mouseY(0.0)
{

}

bool ReportClickMouseHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = static_cast<osgViewer::View*>(aa.asView());

    if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        _mouseX = ea.getX();
        _mouseY = ea.getY();
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        if (fabs(ea.getX() - _mouseX) < 2.0f && fabs(ea.getY() - _mouseY) < 2.0f)
        {
            osg::Vec3d world;
            if (_mapNode->getTerrain()->getWorldCoordsUnderMouse(aa.asView(), ea.getX(), ea.getY(), world))
            {
                osgEarth::GeoPoint mapPoint;
                mapPoint.fromWorld(_mapNode->getMapSRS(), world);

                _scb(mapPoint);

                return false;
            }
        }

        _fcb();
    }

    return false;
}

//====================================================

DrawRectangleMouseHandler::DrawRectangleMouseHandler(osgEarth::MapNode* mapNode,
                                                     const RectangleCreateCallbackType& rcb,
                                                     const RectangleDrawCallbackType& dcb,
                                                     const RectangleFailCallbackType& fcb) :
                                                     osgGA::GUIEventHandler(),
                                                     _mapNode(mapNode),
                                                     _createCB(rcb),
                                                     _drawCB(dcb),
                                                     _failCB(fcb),
                                                     _mouseX(0.0),
                                                     _mouseY(0.0)
{
}

void DrawRectangleMouseHandler::reset()
{
    _firstCorner.reset();
}

bool DrawRectangleMouseHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = static_cast<osgViewer::View*>(aa.asView());

    if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        _mouseX = ea.getX();
        _mouseY = ea.getY();
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        if (fabs(ea.getX() - _mouseX) < 2.0f && fabs(ea.getY() - _mouseY) < 2.0f)
        {
            osgEarth::GeoPoint mapPoint;
            if (!computeMapPoint(aa.asView(), ea.getX(), ea.getY(), mapPoint))
            {
                _failCB();
                return false;
            }

            if (!_firstCorner)
            {
                _firstCorner = mapPoint;
            }
            else
            {
                osgEarth::Bounds b;
                b.expandBy(_firstCorner->x(), _firstCorner->y());
                b.expandBy(mapPoint.x(), mapPoint.y());

                _drawCB(b);
                _createCB(b);
            }
        }
        else
        {
            _failCB();
            return false;
        }
    }
    else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON || ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        _failCB();
        return false;
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
    {
        osgEarth::GeoPoint mapPoint;
        if (!computeMapPoint(aa.asView(), ea.getX(), ea.getY(), mapPoint))
        {
            return false;
        }

        if (_firstCorner)
        {
            osgEarth::Bounds b;
            b.expandBy(_firstCorner->x(), _firstCorner->y());
            b.expandBy(mapPoint.x(), mapPoint.y());

            _drawCB(b);
        }
    }

    return false;
}

bool DrawRectangleMouseHandler::computeMapPoint(osg::View* view, float mx, float my, osgEarth::GeoPoint& point)
{
    osg::Vec3d world;
    if (_mapNode->getTerrain()->getWorldCoordsUnderMouse(view, mx, my, world))
    {
        point.fromWorld(_mapNode->getMapSRS(), world);
        return true;
    }

    return false;
}
