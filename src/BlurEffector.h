//
//  BlurEffector.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 22 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Effector.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class BlurEffector> BlurEffectorRef;

class BlurEffector : public Effector, public std::enable_shared_from_this<BlurEffector> {
public:
    static BlurEffectorRef create();
    ~BlurEffector();

    void execute(gl::Fbo& inputFBO, const int inputFBOAttachment, gl::Fbo& outputFBO, const int outputFBOAttachment);

    void setSampleOffset(Vec2f sampleOffset) { mSampleOffset = sampleOffset; }
    Vec2f sampleOffset() const { return mSampleOffset; }

private:
    BlurEffector();

    Vec2f mSampleOffset;
};
