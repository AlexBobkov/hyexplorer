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
                                                 const RectangleFailCallbackType& rfcb) :
                                                 osgGA::GUIEventHandler(),
                                                 _mapNode(mapNode),                                                
                                                 _rectangleCB(rcb),
                                                 _rectangleFailCB(rfcb),
                                                 _rectangleMode(false),
                                                 _mouseX(0.0),
                                                 _mouseY(0.0)
{
}

void DrawRectangleMouseHandler::setRectangleMode(bool b)
{
    _rectangleMode = b;

    if (_rectangleMode)
    {
        _firstCorner.reset();
        _ring = nullptr;
        _feature = nullptr;
    }
}

void DrawRectangleMouseHandler::setInitialRectangle(const osgEarth::Bounds& b)
{
    _initialBounds = b;
    updateFeature(osgEarth::GeoPoint(_mapNode->getMapSRS(), b.xMin(), b.yMax()),
                  osgEarth::GeoPoint(_mapNode->getMapSRS(), b.xMax(), b.yMin()));
}

bool DrawRectangleMouseHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    if (_rectangleMode)
    {
        return handleRectangle(ea, aa);
    }

    return false;
}

bool DrawRectangleMouseHandler::handleRectangle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = static_cast<osgViewer::View*>(aa.asView());

    if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        osgEarth::GeoPoint mapPoint;
        if (!computeMapPoint(aa.asView(), ea.getX(), ea.getY(), mapPoint))
        {
            resetFeature();
            _rectangleMode = false;
            _rectangleFailCB();
            return false;
        }

        _mouseX = ea.getX();
        _mouseY = ea.getY();
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        osgEarth::GeoPoint mapPoint;
        if (!computeMapPoint(aa.asView(), ea.getX(), ea.getY(), mapPoint))
        {
            resetFeature();
            _rectangleMode = false;
            _rectangleFailCB();
            return false;
        }

        if (fabs(ea.getX() - _mouseX) < 2.0f && fabs(ea.getY() - _mouseY) < 2.0f)
        {
            if (!_firstCorner)
            {
                _firstCorner = mapPoint;
            }
            else
            {
                updateFeature(*_firstCorner, mapPoint);

                osgEarth::Bounds b;
                b.expandBy(_firstCorner->x(), _firstCorner->y());
                b.expandBy(mapPoint.x(), mapPoint.y());
                _initialBounds = b;
                _rectangleCB(b);

                _rectangleMode = false;
            }
        }
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE && ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        resetFeature();
        _rectangleMode = false;
        _rectangleFailCB();
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
            updateFeature(*_firstCorner, mapPoint);
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

void DrawRectangleMouseHandler::updateFeature(const osgEarth::GeoPoint& point1, const osgEarth::GeoPoint& point2)
{
    if (!_ring)
    {
        _ring = new osgEarth::Symbology::Ring();

        osgEarth::Symbology::Style pathStyle;
        pathStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Blue;
        pathStyle.getOrCreate<LineSymbol>()->stroke()->width() = 2.0f;
        pathStyle.getOrCreate<LineSymbol>()->stroke()->stipplePattern() = 0x0F0F;
        pathStyle.getOrCreate<LineSymbol>()->tessellation() = 20;
        pathStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_TO_TERRAIN;
        pathStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_SCENE;

        _feature = new osgEarth::Features::Feature(_ring, _mapNode->getMapSRS(), pathStyle);

        if (!_featureNode.valid())
        {
            _featureNode = new osgEarth::Annotation::FeatureNode(_mapNode.get(), _feature);
            _mapNode->addChild(_featureNode);
        }
    }

    osg::Vec3d p1 = point1.vec3d();
    osg::Vec3d p2 = point2.vec3d();

    _ring->clear();
    _ring->push_back(osg::Vec3d(osg::minimum(p1.x(), p2.x()), osg::maximum(p1.y(), p2.y()), 0.0));
    _ring->push_back(osg::Vec3d(osg::maximum(p1.x(), p2.x()), osg::maximum(p1.y(), p2.y()), 0.0));
    _ring->push_back(osg::Vec3d(osg::maximum(p1.x(), p2.x()), osg::minimum(p1.y(), p2.y()), 0.0));
    _ring->push_back(osg::Vec3d(osg::minimum(p1.x(), p2.x()), osg::minimum(p1.y(), p2.y()), 0.0));

    _featureNode->setFeature(_feature);
}

void DrawRectangleMouseHandler::resetFeature()
{
    updateFeature(osgEarth::GeoPoint(_mapNode->getMapSRS(), _initialBounds.xMin(), _initialBounds.yMax()),
                  osgEarth::GeoPoint(_mapNode->getMapSRS(), _initialBounds.xMax(), _initialBounds.yMin()));
}