//
//  Node.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "Node.h"
#include "cinder/Utilities.h"

namespace Cinder { namespace Pipeline {

void Node::connectOutputNode(const NodeRef& node) {
    mOutputNodes.push_back(node);
    node->connectInputNode(shared_from_this());
}

void Node::disconnectOutputNode(const NodeRef& node) {
    mOutputNodes.erase(std::find(mOutputNodes.begin(), mOutputNodes.end(), node));
    node->disconnectInputNode(shared_from_this());
}

void Node::connectInputNode(const NodeRef& node) {
    mInputNodes.push_back(node);
}

void Node::disconnectInputNode(const NodeRef& node) {
    mInputNodes.erase(std::find(mInputNodes.begin(), mInputNodes.end(), node));
}

#pragma mark - INPUT

SourceNodeRef SourceNode::create() {
    return SourceNodeRef(new SourceNode());
}

SourceNodeRef SourceNode::create(const gl::TextureRef& texture) {
    return SourceNodeRef(new SourceNode(texture));
}

void SourceNode::render(gl::Fbo &fbo, const int attachment) {
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);
    gl::draw(mTexture);
}

#pragma mark - EFFECTOR

const std::string EffectorNode::sVertexShaderPassThrough = R"(
    #version 120
    void main() {
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = ftransform();
    }
)";

EffectorNode::EffectorNode(DataSourceRef vertexShader, DataSourceRef fragmentShader) {
    std::string vert = vertexShader ? loadString(vertexShader) : sVertexShaderPassThrough;
    std::string frag = loadString(fragmentShader);
    EffectorNode(vert, frag);
}

EffectorNode::EffectorNode(const std::string& vertexShader, const std::string& fragmentShader) {
    setupShader(vertexShader, fragmentShader);
}

void EffectorNode::setupShader(const std::string& vertexShader, const std::string& fragmentShader) {
    const char* vert = !vertexShader.empty() ? vertexShader.c_str() : sVertexShaderPassThrough.c_str();
    const char* frag = fragmentShader.c_str();
    mShader = gl::GlslProg::create(vert, frag);
}

}}
