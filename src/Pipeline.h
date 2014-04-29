//
//  Pipeline.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Effector.h"
#include "cinder/gl/Fbo.h"

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class Pipeline> PipelineRef;

class Pipeline : public std::enable_shared_from_this<Pipeline> {
public:
    static PipelineRef create();
    ~Pipeline();

private:
    Pipeline();

    gl::Fbo mFBO;
};

}}
