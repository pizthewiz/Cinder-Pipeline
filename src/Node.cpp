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

void Node::connectOutputNode(const NodeRef& node, const std::string& key, const std::string& outputPortKey) {
    // TODO - host to bool hasInputPortWithKey(const std::string& key) { … }
    auto it = std::find_if(node->getInputPorts().begin(), node->getInputPorts().end(), [key](const NodePortRef& p){ return p->getKey() == key; });
    if (it == node->getInputPorts().end()) {
        cinder::app::console() << "ERROR - attempting to connect to unknown input port '" << key << "'" << std::endl;
        return;
    }

    // check that types match, no cycle
//    mOutputConnectionMap[outputPortKey].push_back(std::make_tuple(node, key));
    node->connectInputNode(shared_from_this(), outputPortKey, key);
}

void Node::connectInputNode(const NodeRef& source, const std::string& sourceKey, const std::string& destinationKey) {
    mInputConnections[destinationKey] = NodePortConnection::create(source, sourceKey, nullptr, destinationKey);
}

#pragma mark - SOURCE

SourceNodeRef SourceNode::create() {
    return SourceNodeRef(new SourceNode());
}

SourceNode::SourceNode() {
    std::vector<NodePortRef> inputPorts = {NodePort::create("texture", NodePortType::Texture)};
    setInputPorts(inputPorts);
//    std::vector<std::string> outputKeys = {"image"};
//    setOutputPortKeys(outputKeys);
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

void EffectorNode::setupShader(const DataSourceRef& vertexShader, const DataSourceRef& fragmentShader) {
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
