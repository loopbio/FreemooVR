/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef DSOSG_H
#define DSOSG_H

#include <osg/Vec3>
#include <osg/PositionAttitudeTransform>
#include <osg/Uniform>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include "Poco/ClassLoader.h"
#include "Poco/Manifest.h"
#include "Poco/Path.h"

#include "flyvr/StimulusInterface.hpp"
#include "flyvr/CallbackHolder.hpp"

#include "WindowCaptureCallback.h"
#include "GeometryTextureToDisplayImagePass.h"
#include "ProjectCubemapToGeometryPass.h"

namespace dsosg{

    typedef struct {
        osg::Quat rotation;
        osg::Vec3 center;
        double distance;
    } TrackballManipulatorState;

    typedef Poco::ClassLoader<StimulusInterface> StimulusLoader;

    class ObserverPositionCallback: public osg::Uniform::Callback {
    public:
        ObserverPositionCallback() {}
        virtual void operator() ( osg::Uniform* uniform, osg::NodeVisitor* nv );
        virtual void setObserverPosition( osg::Vec3 p );
    private:
        OpenThreads::Mutex      _mutex;
        osg::Vec3 _p;
    };

    class CameraCube; // forward declaration

    class DSOSG {
    public:
        DSOSG(std::string flyvr_basepath, std::string mode, float observer_radius, std::string config_fname, bool two_pass=false,
              bool show_geom_coords=false, bool tethered_mode=false, bool slave=false, unsigned int cubemap_resolution=512);

        std::vector<std::string> get_stimulus_plugin_names();
        std::string get_current_stimulus_plugin_name() { return std::string(_current_stimulus->name()); }
        void set_stimulus_plugin(const std::string& plugin_name);

        std::vector<std::string> stimulus_get_topic_names(const std::string& plugin_name);
        std::string stimulus_get_message_type(const std::string& plugin_name, const std::string& topic_name);

        void topic_receive_json_message(const std::string& topic_name, const std::string& json_message);
        void stimulus_receive_json_message(const std::string& plugin_name, const std::string& topic_name, const std::string& json_message);

        void setup_viewer(const std::string& viewer_window_name, const std::string& json_config, bool pbuffer=false);
        void resized(const int& width, const int& height);

        void update( const double& time, const osg::Vec3& observer_position, const osg::Quat& observer_orientation );
        void frame();
        bool done();

        int getXSize() {return _width;}
        int getYSize() {return _height;}

        float getFrameRate();
        void setCursorVisible(bool visible);
        void setWindowName(std::string name);
        void setGamma(float gamma);
        void setRedMax(bool red_max);

        void setCaptureImageFilename(std::string name);
        void setCaptureOSGFilename(std::string name);

        void loadDisplayCalibrationFile(std::string p2g_filename,
                                        bool show_geom_coords);
        void loadDisplayGeomJSON(std::string geom_json_buf);

        TrackballManipulatorState getTrackballManipulatorState();
        void setTrackballManipulatorState(TrackballManipulatorState s);

        bool is_CUDA_available();

    private:
        StimulusLoader _stimulus_loader;

        std::map<std::string, StimulusInterface*> _stimulus_plugins;
        std::map<std::string, std::vector<std::string> > _stimulus_topics;

        StimulusInterface* _current_stimulus;
        std::string _mode;

        Poco::Path _flyvr_basepath;
        Poco::Path _config_file_path;

        osgViewer::Viewer* _viewer;
        osg::ref_ptr<osg::PositionAttitudeTransform> _observer_pat;
        osg::ref_ptr<osg::PositionAttitudeTransform> _observer_geometry_pat;
        osg::ref_ptr<osg::PositionAttitudeTransform> _observer_marker_pat;
        ObserverPositionCallback* _observer_cb;
        bool _tethered_mode;

        CameraCube* _cubemap_maker;

        osg::ref_ptr<osg::Camera> _hud_cam;
        osg::ref_ptr<osg::Group> _active_3d_world;
        osg::ref_ptr<osg::Group> _active_2d_hud;

        int _width;
        int _height;
        osg::ref_ptr<osgGA::TrackballManipulator> _cameraManipulator;
        WindowCaptureCallback* _wcc;
        GeometryTextureToDisplayImagePass *_g2di;
        OpenThreads::Mutex _osg_capture_mutex;
        std::string        _osg_capture_filename;
        flyvr::BackgroundColorCallback *_bg_callback;
        ProjectCubemapToGeometryPass* _pctcp;
        osg::Camera* _debug_hud_cam;
        osg::ref_ptr<osg::Group> _root;
        osg::Group* _g2d_hud_cam_root;
        bool _two_pass;
    };

}

#endif
