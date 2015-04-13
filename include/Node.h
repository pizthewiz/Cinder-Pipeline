//
//  Node.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#pragma once

#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include <boost/any.hpp>

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class Node> NodeRef;
typedef std::shared_ptr<class NodePort> NodePortRef;
typedef std::shared_ptr<class FBOImage> FBOImageRef;

class FBOImage : public std::enable_shared_from_this<FBOImage> {
public:
    static FBOImageRef create(const ci::gl::FboRef& fbo, GLenum attachment) {
        return FBOImageRef(new FBOImage(fbo, attachment))->shared_from_this();
    }

    inline ci::gl::FboRef getFBO() const { return mFBO; }
    inline GLenum getAttachment() const { return mAttachment; }

    ci::gl::TextureBaseRef getTexture() { return mFBO->getTexture(mAttachment); }

private:
    FBOImage(const ci::gl::FboRef& fbo, GLenum attachment) : mFBO(fbo), mAttachment(attachment) {}

    ci::gl::FboRef mFBO;
    GLenum mAttachment;
};

enum class NodePortType { FBOImage, Texture, Bool, Float, Int, Vec2, Color, Index, FilePath };

class NodePort : public std::enable_shared_from_this<NodePort> {
public:
    static NodePortRef create(const std::string& key, const NodePortType type, const std::string& label = "", const boost::any& valueDefault = boost::any(), const boost::any& valueMinimum = boost::any(), const boost::any& valueMaximum = boost::any()) {
        return NodePortRef(new NodePort(key, type, label, valueDefault, valueMinimum, valueMaximum))->shared_from_this();
    }
    static NodePortRef create(const std::string& key, const NodePortType type, const std::string& label, const int defaultValue, const std::vector<int>& values, const std::vector<std::string>& labels) {
        return NodePortRef(new NodePort(key, type, label, defaultValue, values, labels))->shared_from_this();
    }

    inline std::string& getKey() { return mKey; }
    inline NodePortType getType() { return mType; }

    inline bool hasLabel() { return !mLabel.empty(); }
    inline std::string& getLabel() { return mLabel; }

    inline bool hasValueDefault() { return !mDefault.empty(); }
    inline boost::any getValueDefault() { return mDefault; }
    inline bool hasValueMinimum() { return !mMinimum.empty(); }
    inline boost::any getValueMinimum() { return mMinimum; }
    inline bool hasValueMaximum() { return !mMaximum.empty(); }
    inline boost::any getValueMaximum() { return mMaximum; }

    inline std::vector<int> getValues() { return mValues; }
    inline bool hasLabels() { return !mValues.empty(); }
    inline std::vector<std::string> getLabels() { return mLabels; }

private:
    NodePort(const std::string& key, const NodePortType type, const std::string& label, const boost::any& def, const boost::any& min, const boost::any& max) : mKey(key), mType(type), mLabel(label), mDefault(def), mMinimum(min), mMaximum(max) {}
    NodePort(const std::string& key, const NodePortType type, const std::string& label, const int def, const std::vector<int>& values, const std::vector<std::string>& labels) : mKey(key), mType(type), mLabel(label), mDefault(def), mValues(values), mLabels(labels) {}

    std::string mKey;
    NodePortType mType;

    std::string mLabel;

    boost::any mDefault;
    boost::any mMinimum;
    boost::any mMaximum;

    std::vector<int> mValues;
    std::vector<std::string> mLabels;
};

static const std::string NodeInputPortKeyImage = "image";
static const std::string NodeOutputPortKeyImage = "image";

class Node : public std::enable_shared_from_this<Node>, public boost::noncopyable {
public:
    Node() { mOutputPorts = { NodePort::create(NodeOutputPortKeyImage, NodePortType::FBOImage) }; }
    virtual ~Node() {}

    NodeRef getPtr() { return shared_from_this(); }

    virtual std::string getName() const = 0;

    void setInputPorts(std::vector<NodePortRef>& ports);
    inline std::vector<NodePortRef>& getInputPorts() { return mInputPorts; }
    inline std::vector<NodePortRef>& getOutputPorts() { return mOutputPorts; }

    std::vector<std::string> getInputPortKeys();
    std::vector<std::string> getInputPortKeysWithType(const NodePortType type);
    std::vector<std::string> getImageInputPortKeys() { return getInputPortKeysWithType(NodePortType::FBOImage); }

    NodePortRef getInputPortForKey(const std::string& key);
    NodePortRef getOutputPortForKey(const std::string& key);

    void setValueForInputPortKey(const boost::any& value, const std::string& key);
    template <typename T>
	inline T getValueForInputPortKey(const std::string& key) { return boost::any_cast<T>(mInputPortValueMap[key]); }
    inline bool hasValueForInputPortKey(const std::string& key) { return mInputPortValueMap.find(key) != mInputPortValueMap.end(); }

    template<typename T, typename Y>
    inline void connectValueForInputPortKeyChangedHandler(const std::string& key, T handler, Y* obj) {
        connectValueForInputPortKeyChangedHandler(key, std::bind(handler, obj, std::placeholders::_1));
    }
    void connectValueForInputPortKeyChangedHandler(const std::string& key, const std::function<void (const std::string&)>& handler) {
        mInputPortValueChangedHandlerMap[key] = handler;
    }

protected:
    template <typename T>
    boost::any clampValue(const boost::any& value, const boost::any& minimum, const boost::any& maximum);

    std::vector<NodePortRef> mInputPorts;
    std::vector<NodePortRef> mOutputPorts;
    std::map<std::string, boost::any> mInputPortValueMap;
    std::map<std::string, std::function<void (const std::string&)>> mInputPortValueChangedHandlerMap;
};

}}
