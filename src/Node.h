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

enum class NodePortType {FBOImage, Texture, Bool, Float, Int, Vec2f, Vec4f, FilePath};

// TODO - min, max
class NodePort : public std::enable_shared_from_this<NodePort> {
public:
    static NodePortRef create(const std::string& key, const NodePortType type = NodePortType::FBOImage) {
        return NodePortRef(new NodePort(key, type, boost::any()))->shared_from_this();
    }
    static NodePortRef create(const std::string& key, const NodePortType type, boost::any valueDefault) {
        return NodePortRef(new NodePort(key, type, valueDefault))->shared_from_this();
    }

    inline std::string& getKey() { return mKey; }
    inline NodePortType getType() { return mType; }
    inline bool hasValueDefault() { return !mDefault.empty(); }
    inline boost::any getValueDefault() { return mDefault; }

private:
    NodePort(const std::string& key, const NodePortType type, boost::any valueDefault) : mKey(key), mType(type), mDefault(valueDefault) {}

    std::string mKey;
    NodePortType mType;
    boost::any mDefault;
};

static const std::string NodeInputPortKeyImage = "image";
static const std::string NodeOutputPortKeyImage = "image";

class Node : public std::enable_shared_from_this<Node>, public boost::noncopyable {
public:
    Node() {
        mOutputPorts = {
            NodePort::create(NodeOutputPortKeyImage, NodePortType::FBOImage),
        };
    }
    virtual ~Node() {}

    virtual std::string getName() const = 0;

    void setInputPorts(std::vector<NodePortRef>& ports) {
        mInputPorts = ports;

        // set default values
        for (const NodePortRef& port : mInputPorts) {
            if (!port->hasValueDefault()) {
                continue;
            }
            setValueForInputPortKey(port->getValueDefault(), port->getKey());
        }
    }
    inline std::vector<NodePortRef>& getInputPorts() { return mInputPorts; }
    inline std::vector<std::string> getInputPortKeys() {
        // TODO - replace with some sort of collect
        std::vector<std::string> keys;
        for (const NodePortRef& port : mInputPorts) {
            keys.push_back(port->getKey());
        }
        return keys;
    }

    inline std::vector<NodePortRef>& getOutputPorts() { return mOutputPorts; }

    inline std::vector<std::string> getInputPortKeysWithType(NodePortType type) {
        std::vector<std::string> filteredKeys;
        for (const NodePortRef& port : mInputPorts) {
            if (port->getType() != type) {
                continue;
            }
            filteredKeys.push_back(port->getKey());
        }
        return filteredKeys;
    }
    std::vector<std::string> getImageInputPortKeys() {
        return getInputPortKeysWithType(NodePortType::FBOImage);
    }

    NodePortRef getInputPortForKey(const std::string& key) {
        NodePortRef port = nullptr;
        auto it = std::find_if(mInputPorts.begin(), mInputPorts.end(), [key](const NodePortRef& p){ return p->getKey() == key; });
        if (it != mInputPorts.end()) {
            port = *it;
        }
        return port;
    }
    NodePortRef getOutputPortForKey(const std::string& key) {
        NodePortRef port = nullptr;
        auto it = std::find_if(mOutputPorts.begin(), mOutputPorts.end(), [key](const NodePortRef& p){ return p->getKey() == key; });
        if (it != mOutputPorts.end()) {
            port = *it;
        }
        return port;
    }

    template <typename T>
    void setValueForInputPortKey(T value, const std::string& key) {
        // TODO - bail if unchanged, boost::any doesn't implement ==

        mInputPortValueMap[key] = value;

        // value changed handler
        if (mInputPortValueChangedHandlerMap.find(key) != mInputPortValueChangedHandlerMap.end()) {
            mInputPortValueChangedHandlerMap[key](key);
        }
    }
    template <typename T>
	inline T getValueForInputPortKey(const std::string& key) {
        return boost::any_cast<T>(mInputPortValueMap[key]);
    }
    inline bool hasValueForInputPortKey(const std::string& key) {
        return mInputPortValueMap.find(key) != mInputPortValueMap.end();
    }

    template<typename T, typename Y>
    inline void connectValueForInputPortKeyChangedHandler(const std::string& key, T handler, Y* obj) {
        connectValueForInputPortKeyChangedHandler(key, std::bind(handler, obj, std::placeholders::_1));
    }
    void connectValueForInputPortKeyChangedHandler(const std::string& key, const std::function<void(const std::string&)>& handler) {
        mInputPortValueChangedHandlerMap[key] = handler;
    }

protected:
    std::vector<NodePortRef> mInputPorts;
    std::vector<NodePortRef> mOutputPorts;
    std::map<std::string, boost::any> mInputPortValueMap;
    std::map<std::string, std::function<void (const std::string&)>> mInputPortValueChangedHandlerMap;
};

}}
