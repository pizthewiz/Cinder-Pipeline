//
//  Effector.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"

namespace Cinder { namespace Pipeline {

using namespace ci;

typedef std::shared_ptr<class Effector> EffectorRef;

class Effector {
public:
    ~Effector() {}

    virtual void execute(gl::Fbo& inputFBO, int inputFBOAttachment, gl::Fbo& outputFBO, int outputFBOAttachment) {}
    virtual void execute(gl::Fbo& inputFBO, int inputFBOAttachment, gl::Fbo& inputAltFBO, int inputAltFBOAttachment, gl::Fbo& outputFBO, int outputFBOAttachment) {}
    // properties
    //  inputs
    //  mask input
    //  output
    //  ? number of passes
    //  ? needs intermediate FBO (for multi-pass)

protected:
    Effector() {}
    Effector(DataSourceRef vertexShader, DataSourceRef fragmentShader);
    Effector(const std::string& vertexShader, const std::string& fragmentShader);

    void setupShader(const std::string& vertexShader, const std::string& fragmentShader);

    static const std::string sVertexShaderPassThrough;
    gl::GlslProgRef mShader;
};

}}
