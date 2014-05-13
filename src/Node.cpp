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

//void Node::connectOutputNode(const std::string outputPortKey, const NodeRef& node, const std::string key) {
//    // check key exists, types match, no cycle
//    mOutputConnectionMap[outputPortKey].push_back(std::make_tuple(node, key));
//    node->connectInputNode(key, shared_from_this(), outputPortKey);
//}
//
//void Node::connectInputNode(const std::string inputPortKey, const NodeRef& node, const std::string key) {
//    mInputConnectionMap[inputPortKey] = std::make_tuple(node, key);
//}

#pragma mark - SOURCE

SourceNodeRef SourceNode::create() {
    return SourceNodeRef(new SourceNode());
}

SourceNode::SourceNode() {
    std::vector<std::string> inputKeys = {"texture"};
    setInputPortKeys(inputKeys);
    std::vector<std::string> outputKeys = {"image"};
    setOutputPortKeys(outputKeys);
}

void SourceNode::render(const FBOImageRef& outputFBOImage) {
    gl::TextureRef texture = getValueForInputPortKey<gl::TextureRef>("texture");

    gl::draw(texture);
}

#pragma mark - EFFECTOR

const std::string EffectorNode::sVertexShaderPassThrough = R"(
    #version 120
    void main() {
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = ftransform();
    }
)";

void EffectorNode::setupShader(DataSourceRef vertexShader, DataSourceRef fragmentShader) {
    std::string vert = vertexShader ? loadString(vertexShader) : sVertexShaderPassThrough;
    std::string frag = loadString(fragmentShader);
    setupShader(vert, frag);
}

void EffectorNode::setupShader(const std::string& vertexShader, const std::string& fragmentShader) {
    const char* vert = !vertexShader.empty() ? vertexShader.c_str() : sVertexShaderPassThrough.c_str();
    const char* frag = fragmentShader.c_str();
    mShader = gl::GlslProg::create(vert, frag);
}

}}
