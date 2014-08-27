//
//  ISFNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 26 Aug 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "EffectorNode.h"

using namespace Cinder::Pipeline;

typedef std::shared_ptr<class ISFNode> ISFNodeRef;

class ISFNode : public EffectorNode {
public:
    ISFNode();
    ~ISFNode();

    ISFNodeRef getPtr() { return std::static_pointer_cast<ISFNode>(shared_from_this()); }

    std::string getName() const { return "ISF"; }

    void render(const FBOImageRef& outputFBOImage);

private:
    void parseShader(const ci::DataSourceRef& vertexShader, const ci::DataSourceRef& fragmentShader);
    void parseShader(const std::string& vertexShader, const std::string& fragmentShader);
};
