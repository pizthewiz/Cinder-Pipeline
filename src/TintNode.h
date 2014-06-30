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

static const std::string TintNodeInputPortKeyImage = "image";
static const std::string TintNodeInputPortKeyColor = "tintColor";
static const std::string TintNodeInputPortKeyAmount = "amount";

class TintNode : public EffectorNode {
public:
    TintNode();
    ~TintNode();

    virtual std::string getName() const { return "Tint"; }

    void render(const FBOImageRef& outputFBOImage);
};
