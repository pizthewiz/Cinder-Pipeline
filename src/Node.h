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
#include <boost/any.hpp>

namespace Cinder { namespace Pipeline {

using namespace ci;

typedef std::shared_ptr<class Node> NodeRef;
typedef std::shared_ptr<class SourceNode> SourceNodeRef;
typedef std::shared_ptr<class EffectorNode> EffectorNodeRef;

// types: FBOImage, texture, float, Vec2f, bool, int
// typedef std::tuple<NodeRef, std::string> NodeConnection

class Node : public std::enable_shared_from_this<Node>, public boost::noncopyable {
public:
    virtual ~Node() {}

    virtual std::string getName() const = 0;

    void setInputPortKeys(const std::vector<std::string> keys) { mInputPortKeys = keys; }
    std::vector<std::string> getInputPortKeys() const { return mInputPortKeys; }
    void setOutputPortKeys(const std::vector<std::string> keys) { mOutputPortKeys = keys; }
    std::vector<std::string> getOutputPortKeys() const { return mOutputPortKeys; }

    template <typename T>
    void setValueForInputPortKey(T value, const std::string& key) {
        if (std::find(mInputPortKeys.begin(), mInputPortKeys.end(), key) == mInputPortKeys.end()) {
            return;
        }
        mPortValueMap[key] = value;
    }
    template <typename T>
	inline T getValueForOutputPortKey(const std::string& key) {
        return boost::any_cast<T>(mPortValueMap[key]);
    }

    virtual void connectOutputNode(const std::string outputPortKey, const NodeRef& node, const std::string key);

    std::tuple<NodeRef, std::string>& getNodeConnectionForInputPortKey(const std::string key) { return mInputConnectionMap[key]; }
    std::vector<std::tuple<NodeRef, std::string>>& getNodeConnectionForOutputPortKey(const std::string key) { return mOutputConnectionMap[key]; }

protected:
    Node() {}

    virtual void connectInputNode(const std::string inputPortKey, const NodeRef& node, const std::string key);

    std::vector<std::string> mInputPortKeys;
    std::vector<std::string> mOutputPortKeys;

    std::map<std::string, boost::any> mPortValueMap;

    std::map<std::string, std::tuple<NodeRef, std::string>> mInputConnectionMap;
    std::map<std::string, std::vector<std::tuple<NodeRef, std::string>>> mOutputConnectionMap;
};

class SourceNode : public Node {
public:
    static SourceNodeRef create();
    static SourceNodeRef create(const gl::TextureRef& texture);
    virtual ~SourceNode() {}

    virtual std::string getName() const { return "Source"; }

    virtual void render(gl::Fbo& outFBO, const int outAttachment);

    void setTexture(gl::TextureRef& texture) { mTexture = texture; }
    gl::TextureRef& getTexture() { return mTexture; }

protected:
    SourceNode() {}
    SourceNode(const gl::TextureRef& texture) : mTexture(texture) {}

    virtual void connectInputNode(const std::string inputPortKey, const NodeRef& node, const std::string key) final {}

    gl::TextureRef mTexture;
};

class EffectorNode : public Node {
public:
    virtual ~EffectorNode() {}

    virtual std::string getName() const { return "Effector"; }

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
