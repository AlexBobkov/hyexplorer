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
