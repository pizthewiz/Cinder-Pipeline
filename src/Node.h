//
//  Node.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"

namespace Cinder { namespace Pipeline {

using namespace ci;

typedef std::shared_ptr<class Node> NodeRef;
typedef std::shared_ptr<class SourceNode> SourceNodeRef;
typedef std::shared_ptr<class EffectorNode> EffectorNodeRef;

class Node : public std::enable_shared_from_this<Node>, public boost::noncopyable {
public:
    virtual ~Node() {}

    virtual void connectOutputNode(const NodeRef& node);
    virtual void disconnectOutputNode(const NodeRef& node);

    // TODO std::vector<NodeInputDescriptor> getInputDescriptors() const { return mInputDescriptors; }
    std::vector<NodeRef> getInputNodes() const { return mInputNodes; }
    std::vector<NodeRef> getOutputNodes() const { return mOutputNodes; }

protected:
    Node() {}

    // TODO - replace with virtual void connectInputNode(const NodeRef& node, NodeInputIdentifier);
    virtual void connectInputNode(const NodeRef& node);
    virtual void disconnectInputNode(const NodeRef& node);

    // TODO - replace with std::map<NodeInputIdentifier, NodeRef> mInputsMap
    std::vector<NodeRef> mInputNodes;
    std::vector<NodeRef> mOutputNodes;
};

inline const NodeRef& operator>>(const NodeRef &node, const NodeRef& childNode) {
    node->connectOutputNode(childNode);
    return childNode;
}

class SourceNode : public Node {
public:
    static SourceNodeRef create();
    static SourceNodeRef create(gl::TextureRef& texture);
    virtual ~SourceNode() {}

    void setTexture(gl::TextureRef& texture) { mTexture = texture; }
    gl::TextureRef& getTexture() { return mTexture; }

    void render(gl::Fbo& fbo, const int attachment);

protected:
    SourceNode() {}
    SourceNode(gl::TextureRef& texture) : mTexture(texture) {}

    virtual void connectInput(const NodeRef& node) final {}
    virtual void disconnectInput(const NodeRef& node) final {}

    gl::TextureRef mTexture;
};

class EffectorNode : public Node {
public:
    virtual ~EffectorNode() {}

    virtual void render(gl::Fbo& inFBO, const int inAttachment, gl::Fbo& outFBO, const int outAttachment) {}
    virtual void render(gl::Fbo& inFBO, const int inAttachment, gl::Fbo& inAltFBO, const int inAltAttachment, gl::Fbo& outFBO, const int outAttachment) {}

protected:
    EffectorNode() {}
    EffectorNode(DataSourceRef vertexShader, DataSourceRef fragmentShader);
    EffectorNode(const std::string& vertexShader, const std::string& fragmentShader);

    void setupShader(const std::string& vertexShader, const std::string& fragmentShader);

    static const std::string sVertexShaderPassThrough;
    gl::GlslProgRef mShader;
};

}}
