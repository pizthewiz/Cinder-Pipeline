//
//  BlurNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 22 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class BlurNode> BlurNodeRef;

class BlurNode : public EffectorNode {
public:
    static BlurNodeRef create();
    ~BlurNode();

    virtual std::string getName() const { return "Blur"; }

    void render(gl::Fbo& inputFBO, const int inputFBOAttachment, gl::Fbo& outputFBO, const int outputFBOAttachment);

    void setSampleOffset(Vec2f sampleOffset) { mSampleOffset = sampleOffset; }
    Vec2f sampleOffset() const { return mSampleOffset; }

private:
    BlurNode();

    Vec2f mSampleOffset;
};
