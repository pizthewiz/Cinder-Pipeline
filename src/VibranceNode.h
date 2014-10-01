//
//  VibranceNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 28 Sept 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "EffectorNode.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class VibranceNode> VibranceNodeRef;

static const std::string VibranceNodeInputPortKeyAmount = "amount";
static const std::string VibranceNodeInputPortKeyMixAmount = "mixAmount";

class VibranceNode : public EffectorNode {
public:
    VibranceNode();
    ~VibranceNode();

    VibranceNodeRef getPtr() { return std::static_pointer_cast<VibranceNode>(shared_from_this()); }

    std::string getName() const { return "Vibrance"; }

    void render(const FBOImageRef& outputFBOImage);
};
