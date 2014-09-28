//
//  TintNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Jun 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "EffectorNode.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class TintNode> TintNodeRef;

static const std::string TintNodeInputPortKeyColor = "tintColor";
static const std::string TintNodeInputPortKeyMix = "mix";

class TintNode : public EffectorNode {
public:
    TintNode();
    ~TintNode();

    TintNodeRef getPtr() { return std::static_pointer_cast<TintNode>(shared_from_this()); }

    std::string getName() const { return "Tint"; }

    void render(const FBOImageRef& outputFBOImage);
};
