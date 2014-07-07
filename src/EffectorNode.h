//
//  EffectorNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 May 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"
#include "cinder/Utilities.h"

namespace Cinder { namespace Pipeline {

using namespace ci;

typedef std::shared_ptr<class EffectorNode> EffectorNodeRef;

class EffectorNode : public Node {
public:
    virtual ~EffectorNode() {}

    EffectorNodeRef getPtr() { return std::static_pointer_cast<EffectorNode>(shared_from_this()); }

    virtual std::string getName() const = 0;

    virtual void render(const FBOImageRef& outputFBOImage) {}

protected:
    EffectorNode() {}

    void setupShader(const DataSourceRef& vertexShader, const DataSourceRef& fragmentShader);
    void setupShader(const std::string& vertexShader, const std::string& fragmentShader);

    static const std::string sVertexShaderPassThrough;
    gl::GlslProgRef mShader;
};

}}
