//
//  SourceNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 May 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"

namespace Cinder { namespace Pipeline {

using namespace ci;

typedef std::shared_ptr<class SourceNode> SourceNodeRef;

class SourceNode : public Node {
public:
    SourceNode() {
        std::vector<NodePortRef> inputPorts = {NodePort::create("texture", NodePortType::Texture)};
        setInputPorts(inputPorts);
    }
    virtual ~SourceNode() {}

    virtual std::string getName() const { return "Source"; }

    virtual void render(const FBOImageRef& outputFBOImage) {
        gl::TextureRef texture = getValueForInputPortKey<gl::TextureRef>("texture");
        gl::draw(texture);
    }
    
protected:
};

}}
