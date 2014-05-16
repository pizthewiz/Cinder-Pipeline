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

//typedef std::shared_ptr<class NodePort> NodePortRef;
typedef std::shared_ptr<class NodeConnection> NodeConnectionRef;
typedef std::shared_ptr<class FBOImage> FBOImageRef;

// port: {key: K, type: T, minimum: N, maximum: X, default: D}
// types: PortTypeFBOImage, PortTypeTexture, PortTypeFloat, PortTypeInt, PortTypeBool, PortTypeVec2f, PortTypeVec3f, PortTypeVec4f, PortTypeColor

class NodeConnection : public std::enable_shared_from_this<NodeConnection> {
public:
    static NodeConnectionRef create(const NodeRef& source, const std::string& sourceKey, const NodeRef& destination, const std::string& destinationKey) {
        return NodeConnectionRef(new NodeConnection(source, sourceKey, destination, destinationKey))->shared_from_this();
    }

    NodeRef& getSourceNode() {
        return mSourceNode;
    }
    std::string& getSourcePortKey() {
        return mSourcePortKey;
    }
    NodeRef& getDestinationNode() {
        return mDestinationNode;
    }
    std::string& getDestinationPortKey() {
        return mDestinationPortKey;
    }

private:
    NodeConnection(const NodeRef& source, const std::string& sourceKey, const NodeRef& destination, const std::string& destinationKey) : mSourceNode(source), mSourcePortKey(sourceKey), mDestinationNode(destination), mDestinationPortKey(destinationKey) {}

    NodeRef mSourceNode;
    std::string mSourcePortKey;
    NodeRef mDestinationNode;
    std::string mDestinationPortKey;
};

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

    void setInputPortKeys(std::vector<std::string>& keys) { mInputPortKeys = keys; }
    std::vector<std::string>& getInputPortKeys() { return mInputPortKeys; }
//    void setOutputPortKeys(std::vector<std::string> keys) { mOutputPortKeys = keys; }
//    std::vector<std::string> getOutputPortKeys() const { return mOutputPortKeys; }

    std::vector<std::string> getImageInputPortKeys() {
        std::vector<std::string> filteredKeys;
        for (const std::string& key : mInputPortKeys) {
            // TODO - filter on type, port.type == PortTypeFBOImage
            if (key.rfind("image", 0) != 0) {
                continue;
            }
            filteredKeys.push_back(key);
        }
        return filteredKeys;
    }

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

    virtual void connectOutputNode(const NodeRef& node, const std::string& key = "image", const std::string& outputPortKey = "image");

    NodeConnectionRef& getConnectionForInputPortKey(const std::string& key) { return mInputConnections[key]; }
//    std::vector<std::tuple<NodeRef, std::string>>& getConnectionForOutputPortKey(const std::string key) { return mOutputConnectionMap[key]; }

protected:
//    friend class Pipeline;
//    template <typename T>
//    void setValueForOutputPortKey(T value, const std::string& key) {
//        if (std::find(mOutputPortKeys.begin(), mOutputPortKeys.end(), key) == mOutputPortKeys.end()) {
//            return;
//        }
//        mOutputPortValueMap[key] = value;
//    }
//    template <typename T>
//	inline T getValueForOutputPortKey(const std::string& key) {
//        return boost::any_cast<T>(mOutputPortValueMap[key]);
//    }

    virtual void connectInputNode(const NodeRef& node, const std::string& key = "image", const std::string& inputPortKey = "image");

    std::vector<std::string> mInputPortKeys;
//    std::vector<std::string> mOutputPortKeys;

    std::map<std::string, boost::any> mInputPortValueMap;
//    std::map<std::string, boost::any> mOutputPortValueMap;

    std::map<std::string, NodeConnectionRef> mInputConnections;
//    std::map<std::string, std::vector<std::tuple<NodeRef, std::string>>> mOutputConnectionMap;
};

inline const NodeRef& operator>>(const NodeRef& source, const NodeRef& destination) {
    source->connectOutputNode(destination);
    return destination;
}

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

    void setupShader(const DataSourceRef& vertexShader, const DataSourceRef& fragmentShader);
    void setupShader(const std::string& vertexShader, const std::string& fragmentShader);

    static const std::string sVertexShaderPassThrough;
    gl::GlslProgRef mShader;
};

}}
