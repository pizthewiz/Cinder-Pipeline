//
//  BlendNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 24 Apr 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#pragma once

#include "EffectorNode.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class BlendNode> BlendNodeRef;

static const std::string BlendNodeInputPortKeyBlendImage = "blendImage";
static const std::string BlendNodeInputPortKeyBlendMode = "blendMode";

class BlendNode : public EffectorNode {
public:
    enum class BlendMode { Subtract, Over, Multiply };

    BlendNode();
    ~BlendNode();

    BlendNodeRef getPtr() { return std::static_pointer_cast<BlendNode>(shared_from_this()); }

    std::string getName() override { return "Blend"; }

    void render(const FBOImageRef& outputFBOImage) override;

private:
    void setupShaderForBlendMode(const BlendMode mode);
};

static const BlendNode::BlendMode BlendNodeBlendModeDefault = BlendNode::BlendMode::Over;
