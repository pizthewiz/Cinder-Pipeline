//
//  Pipeline.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class Pipeline> PipelineRef;

class Pipeline : public std::enable_shared_from_this<Pipeline> {
public:
    static PipelineRef create();
    ~Pipeline();

    void setup(const Vec2i size);

    gl::Texture& evaluate(const NodeRef& node);

private:
    Pipeline();

    gl::Fbo mFBO;
};

}}
