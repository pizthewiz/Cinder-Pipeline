//
//  GaussianBlurNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 22 Apr 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#pragma once

#include "EffectorNode.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class GaussianBlurNode> GaussianBlurNodeRef;

static const std::string GaussianBlurNodeInputPortKeyPixelSize = "pixelSize";
static const std::string GaussianBlurNodeInputPortKeyMixAmount = "mixAmount";

class GaussianBlurNode : public EffectorNode {
public:
    GaussianBlurNode();
    ~GaussianBlurNode();

    GaussianBlurNodeRef getPtr() { return std::static_pointer_cast<GaussianBlurNode>(shared_from_this()); }

    std::string getName() const { return "Gaussian Blur"; }

    void render(const FBOImageRef& outputFBOImage);
};
