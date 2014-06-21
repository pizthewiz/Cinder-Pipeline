//
//  BlendNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 24 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "EffectorNode.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class BlendNode> BlendNodeRef;

class BlendNode : public EffectorNode {
public:
    enum class BlendOperation {Subtract, Over};

    BlendNode();
    ~BlendNode();

    virtual std::string getName() const { return "Blend"; }

    void render(const FBOImageRef& outputFBOImage);
};
