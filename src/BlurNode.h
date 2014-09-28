//
//  BlurNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 22 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "EffectorNode.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class BlurNode> BlurNodeRef;

static const std::string BlurNodeInputPortKeyPixelSize = "pixelSize";
static const std::string BlurNodeInputPortKeyMix = "mix";

class BlurNode : public EffectorNode {
public:
    BlurNode();
    ~BlurNode();

    BlurNodeRef getPtr() { return std::static_pointer_cast<BlurNode>(shared_from_this()); }

    std::string getName() const { return "Blur"; }

    void render(const FBOImageRef& outputFBOImage);
};
