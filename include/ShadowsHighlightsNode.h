//
//  ShadowsHighlightsNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 12 Apr 2015.
//  Copyright 2015 Chorded Constructions. All rights reserved.
//

#pragma once

#include "EffectorNode.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class ShadowsHighlightsNode> ShadowsHighlightsNodeRef;

static const std::string ShadowsHighlightsNodeInputPortKeyShadows = "shadows";
static const std::string ShadowsHighlightsNodeInputPortKeyHighlights = "highlights";
static const std::string ShadowsHighlightsNodeInputPortKeyMixAmount = "mixAmount";

class ShadowsHighlightsNode : public EffectorNode {
public:
    ShadowsHighlightsNode();
    ~ShadowsHighlightsNode();

    ShadowsHighlightsNodeRef getPtr() { return std::static_pointer_cast<ShadowsHighlightsNode>(shared_from_this()); }

    std::string getName() override { return "Shadows/Highlights"; }

    void render(const FBOImageRef& outputFBOImage) override;
};
