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
typedef std::shared_ptr<class NodePort> NodePortRef;
typedef std::shared_ptr<class NodePortConnection> NodePortConnectionRef;
typedef std::shared_ptr<class FBOImage> FBOImageRef;

class FBOImage : public std::enable_shared_from_this<FBOImage> {
public:
    static FBOImageRef create(const gl::Fbo& fbo, const int attachment) {
        return FBOImageRef(new FBOImage(fbo, attachment))->shared_from_this();
    }

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

enum class NodePortType {FBOImage, Texture, Bool, Float, Int, Vec2f};

// TODO - default, min, max
class NodePort : public std::enable_shared_from_this<NodePort> {
public:
    static NodePortRef create(const std::string& key, const NodePortType type = NodePortType::FBOImage) {
        return NodePortRef(new NodePort(key, type))->shared_from_this();
    }

    inline std::string& getKey() { return mKey; }
    inline NodePortType getType() { return mType; }

private:
    NodePort(const std::string& key, const NodePortType type) : mKey(key), mType(type) {}

    std::string mKey;
    NodePortType mType;
};

class NodePortConnection : public std::enable_shared_from_this<NodePortConnection> {
public:
    static NodePortConnectionRef create(const NodeRef& source, const std::string& sourceKey, const NodeRef& destination, const std::string& destinationKey) {
        return NodePortConnectionRef(new NodePortConnection(source, sourceKey, destination, destinationKey))->shared_from_this();
    }

    ~NodePortConnection() {
        mSourceNode = nullptr;
        mDestinationNode = nullptr;
    }

    inline NodeRef& getSourceNode() {
        return mSourceNode;
    }
    inline std::string& getSourcePortKey() {
        return mSourcePortKey;
    }
    inline NodeRef& getDestinationNode() {
        return mDestinationNode;
    }
    inline std::string& getDestinationPortKey() {
        return mDestinationPortKey;
    }

private:
    NodePortConnection(const NodeRef& source, const std::string& sourceKey, const NodeRef& destination, const std::string& destinationKey) : mSourceNode(source), mSourcePortKey(sourceKey), mDestinationNode(destination), mDestinationPortKey(destinationKey) {}

    NodeRef mSourceNode;
    std::string mSourcePortKey;
    NodeRef mDestinationNode;
    std::string mDestinationPortKey;
};

class Node : public std::enable_shared_from_this<Node>, public boost::noncopyable {
public:
    Node() {}
    virtual ~Node() {}

    virtual std::string getName() const = 0;

    void setInputPorts(std::vector<NodePortRef>& ports) { mInputPorts = ports; }
    inline std::vector<NodePortRef>& getInputPorts() { return mInputPorts; }

    std::vector<std::string> getImageInputPortKeys() {
        std::vector<std::string> filteredKeys;
        for (const NodePortRef& port : mInputPorts) {
            if (port->getType() != NodePortType::FBOImage) {
                continue;
            }
            filteredKeys.push_back(port->getKey());
        }
        return filteredKeys;
    }

    NodePortRef getInputPortForKey(const std::string& key) {
        NodePortRef port = nullptr;
        auto it = std::find_if(getInputPorts().begin(), getInputPorts().end(), [key](const NodePortRef& p){ return p->getKey() == key; });
        if (it != getInputPorts().end()) {
            port = *it;
        }
        return port;
    }

    template <typename T>
    void setValueForInputPortKey(T value, const std::string& key) {
        mInputPortValueMap[key] = value;
    }
    template <typename T>
	inline T getValueForInputPortKey(const std::string& key) {
        return boost::any_cast<T>(mInputPortValueMap[key]);
    }

    virtual void connectOutputNode(const NodeRef& node, const std::string& key = "image", const std::string& outputPortKey = "image");

    inline NodePortConnectionRef& getConnectionForInputPortKey(const std::string& key) { return mInputConnections[key]; }

protected:

    virtual void connectInputNode(const NodeRef& node, const std::string& key = "image", const std::string& inputPortKey = "image");

    std::vector<NodePortRef> mInputPorts;
    std::map<std::string, NodePortConnectionRef> mInputConnections;
    std::map<std::string, boost::any> mInputPortValueMap;
};

inline const NodeRef& operator>>(const NodeRef& source, const NodeRef& destination) {
    source->connectOutputNode(destination);
    return destination;
}

}}
