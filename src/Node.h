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
typedef std::shared_ptr<class FBOImage> FBOImageRef;

// port types: NodePortTypeFBOImage, NodePortTypeTexture, NodePortTypeFloat, NodePortTypeBool, NodePortTypeInt, NodePortTypeVec2f
// typedef std::tuple<NodeRef, std::string> NodeConnection

class FBOImage : public std::enable_shared_from_this<FBOImage> {
public:
    static FBOImageRef create(const gl::Fbo& fbo, const int attachment) { return FBOImageRef(new FBOImage(fbo, attachment))->shared_from_this(); }
    ~FBOImage() {}

    inline gl::Fbo getFBO() const { return mFBO; }
    inline int getAttachment() const { return mAttachment; }

    gl::Texture& getTexture() { return mFBO.getTexture(mAttachment); }

    void bindTexture(const int textureUnit) { mFBO.bindTexture(textureUnit, mAttachment); }
    void unbindTexture() { mFBO.unbindTexture(); }

private:
    FBOImage(const gl::Fbo& fbo, const int attachment) : mFBO(fbo), mAttachment(attachment) {}

    gl::Fbo mFBO;
    int mAttachment;
};

class Node : public std::enable_shared_from_this<Node>, public boost::noncopyable {
public:
    Node() {}
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
        mInputPortValueMap[key] = value;
    }
    template <typename T>
	inline T getValueForInputPortKey(const std::string& key) {
        return boost::any_cast<T>(mInputPortValueMap[key]);
    }

//    virtual void connectOutputNode(const std::string outputPortKey, const NodeRef& node, const std::string key);

//    std::tuple<NodeRef, std::string>& getNodeConnectionForInputPortKey(const std::string key) { return mInputConnectionMap[key]; }
//    std::vector<std::tuple<NodeRef, std::string>>& getNodeConnectionForOutputPortKey(const std::string key) { return mOutputConnectionMap[key]; }

protected:
    friend class Pipeline;
    template <typename T>
    void setValueForOutputPortKey(T value, const std::string& key) {
        if (std::find(mOutputPortKeys.begin(), mOutputPortKeys.end(), key) == mOutputPortKeys.end()) {
            return;
        }
        mOutputPortValueMap[key] = value;
    }
    template <typename T>
	inline T getValueForOutputPortKey(const std::string& key) {
        return boost::any_cast<T>(mOutputPortValueMap[key]);
    }

//    virtual void connectInputNode(const std::string inputPortKey, const NodeRef& node, const std::string key);

    std::vector<std::string> mInputPortKeys;
    std::vector<std::string> mOutputPortKeys;

    std::map<std::string, boost::any> mInputPortValueMap;
    std::map<std::string, boost::any> mOutputPortValueMap;

//    std::map<std::string, std::tuple<NodeRef, std::string>> mInputConnectionMap;
//    std::map<std::string, std::vector<std::tuple<NodeRef, std::string>>> mOutputConnectionMap;
};

class SourceNode : public Node {
public:
    static SourceNodeRef create();
    virtual ~SourceNode() {}

    virtual std::string getName() const { return "Source"; }

    virtual void render(const FBOImageRef& outputFBOImage);

protected:
    SourceNode();
};

class EffectorNode : public Node {
public:
    virtual ~EffectorNode() {}

    virtual std::string getName() const { return "Effector"; }

    virtual void render(const FBOImageRef& outputFBOImage) {}

protected:
    EffectorNode() {}

    void setupShader(DataSourceRef vertexShader, DataSourceRef fragmentShader);
    void setupShader(const std::string& vertexShader, const std::string& fragmentShader);

    static const std::string sVertexShaderPassThrough;
    gl::GlslProgRef mShader;
};

}}
