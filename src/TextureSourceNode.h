//
//  TextureSourceNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 06 Jul 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "SourceNode.h"

namespace Cinder { namespace Pipeline {

using namespace ci;

typedef std::shared_ptr<class TextureSourceNode> TextureSourceNodeRef;

static const std::string TextureSourceNodeInputPortKeyTexture = "texture";

class TextureSourceNode : public SourceNode {
public:
    TextureSourceNode() {
        std::vector<NodePortRef> inputPorts = {NodePort::create(TextureSourceNodeInputPortKeyTexture, NodePortType::Texture)};
        setInputPorts(inputPorts);
    }
    ~TextureSourceNode() {}

    TextureSourceNodeRef getPtr() { return std::static_pointer_cast<TextureSourceNode>(shared_from_this()); }

    std::string getName() const { return "Texture Source"; }

    void render(const FBOImageRef& outputFBOImage) {
        gl::TextureRef texture = getValueForInputPortKey<gl::TextureRef>(TextureSourceNodeInputPortKeyTexture);
        gl::draw(texture);
    }
};

}}
