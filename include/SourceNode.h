//
//  SourceNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 May 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class SourceNode> SourceNodeRef;

class SourceNode : public Node {
public:
    SourceNode() = default;
    virtual ~SourceNode() = default;

    SourceNodeRef getPtr() { return std::static_pointer_cast<SourceNode>(shared_from_this()); }

    virtual std::string getName() = 0;

    virtual void render(const FBOImageRef& outputFBOImage) = 0;
};

}}
