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

static const std::string BlurNodeInputPortKeyImage = "image";
static const std::string BlurNodeInputPortKeyPixelSize = "pixelSize";
static const std::string BlurNodeInputPortKeyAmount = "amount";

class BlurNode : public EffectorNode {
public:
    BlurNode();
    ~BlurNode();

    virtual std::string getName() const { return "Blur"; }

    void render(const FBOImageRef& outputFBOImage);
};
