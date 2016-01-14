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
        typedef std::function<void()> RectangleFailCallbackType;

        DrawRectangleMouseHandler(osgEarth::MapNode* mapNode,
                                const RectangleCreateCallbackType& rcb,
                                const RectangleFailCallbackType& rfcb);

        void setRectangleMode(bool b);

        void setInitialRectangle(const osgEarth::Bounds& b);

        bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

        bool handleRectangle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

    protected:
        bool computeMapPoint(osg::View* view, float mx, float my, osgEarth::GeoPoint& point);
        void updateFeature(const osgEarth::GeoPoint& point1, const osgEarth::GeoPoint& point2);
        void resetFeature();

        osg::observer_ptr<osgEarth::MapNode>  _mapNode;

        RectangleCreateCallbackType _rectangleCB;
        RectangleFailCallbackType _rectangleFailCB;

        bool _rectangleMode;
        boost::optional<osgEarth::GeoPoint> _firstCorner;

        osg::ref_ptr<osgEarth::Symbology::Ring> _ring;
        osg::ref_ptr<osgEarth::Features::Feature> _feature;
        osg::ref_ptr<osgEarth::Annotation::FeatureNode> _featureNode;

        float _mouseX;
        float _mouseY;

        osgEarth::Bounds _initialBounds;
    };
}
