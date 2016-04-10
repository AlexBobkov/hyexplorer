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

#pragma once

#include <osgEarth/GeoData>
#include <osgEarth/Bounds>
#include <osgEarth/MapNode>
#include <osgEarth/Terrain>
#include <osgEarthAnnotation/CircleNode>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthFeatures/Feature>
#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/Style>
#include <osgEarthSymbology/LineSymbol>

#include <osgGA/GUIEventHandler>

#include <boost/optional.hpp>

#include <functional>

namespace portal
{
    /**
    Определяет координаты мыши при движении по глобусу
    */
    class ReportMoveMouseHandler : public osgGA::GUIEventHandler
    {
    public:
        typedef std::function<void(const osgEarth::GeoPoint&)> CallbackType;

        ReportMoveMouseHandler(osgEarth::MapNode* mapNode,
                               const CallbackType& pcb);

        bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

    protected:
        osg::observer_ptr<osgEarth::MapNode>  _mapNode;

        CallbackType _cb;
    };

    /**
    Определяет координаты мыши при клике по глобусу
    */
    class ReportClickMouseHandler : public osgGA::GUIEventHandler
    {
    public:
        typedef std::function<void(const osgEarth::GeoPoint&)> SuccessCallbackType;
        typedef std::function<void()> FailureCallbackType;

        ReportClickMouseHandler(osgEarth::MapNode* mapNode,
                                const SuccessCallbackType& scb,
                                const FailureCallbackType& fcb);

        bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

    protected:
        osg::observer_ptr<osgEarth::MapNode>  _mapNode;

        SuccessCallbackType _scb;
        FailureCallbackType _fcb;

        float _mouseX;
        float _mouseY;
    };

    /**
    Выделяет прямоугольник на глобусе
    */
    class DrawRectangleMouseHandler : public osgGA::GUIEventHandler
    {
    public:
        typedef std::function<void(const osgEarth::Bounds&)> RectangleCreateCallbackType;
        typedef std::function<void(const osgEarth::Bounds&)> RectangleDrawCallbackType;
        typedef std::function<void()> RectangleFailCallbackType;

        DrawRectangleMouseHandler(osgEarth::MapNode* mapNode,
                                const RectangleCreateCallbackType& rcb,
                                const RectangleDrawCallbackType& dcb,
                                const RectangleFailCallbackType& fcb);

        void reset();

        bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

    protected:
        bool computeMapPoint(osg::View* view, float mx, float my, osgEarth::GeoPoint& point);

        osg::observer_ptr<osgEarth::MapNode>  _mapNode;

        RectangleCreateCallbackType _createCB;
        RectangleDrawCallbackType _drawCB;
        RectangleFailCallbackType _failCB;

        boost::optional<osgEarth::GeoPoint> _firstCorner;                

        float _mouseX;
        float _mouseY;
    };
}
