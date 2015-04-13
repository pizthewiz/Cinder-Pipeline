//
//  Node.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 06 Jul 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "Node.h"

namespace Cinder { namespace Pipeline {

void Node::setInputPorts(std::vector<NodePortRef>& ports) {
    mInputPorts = ports;

    // set default values
    for (const NodePortRef& port : mInputPorts) {
        if (!port->hasValueDefault()) {
            continue;
        }
        setValueForInputPortKey(port->getValueDefault(), port->getKey());
    }
}

std::vector<std::string> Node::getInputPortKeys() {
    // TODO - replace with some sort of collect
    std::vector<std::string> keys;
    for (const NodePortRef& port : mInputPorts) {
        keys.push_back(port->getKey());
    }
    return keys;
}

std::vector<std::string> Node::getInputPortKeysWithType(NodePortType type) {
    std::vector<std::string> filteredKeys;
    for (const NodePortRef& port : mInputPorts) {
        if (port->getType() != type) {
            continue;
        }
        filteredKeys.push_back(port->getKey());
    }
    return filteredKeys;
}

NodePortRef Node::getInputPortForKey(const std::string& key) {
    NodePortRef port;
    auto it = std::find_if(mInputPorts.begin(), mInputPorts.end(), [key](const NodePortRef& p){ return p->getKey() == key; });
    if (it != mInputPorts.end()) {
        port = *it;
    }
    return port;
}

NodePortRef Node::getOutputPortForKey(const std::string& key) {
    NodePortRef port;
    auto it = std::find_if(mOutputPorts.begin(), mOutputPorts.end(), [key](const NodePortRef& p){ return p->getKey() == key; });
    if (it != mOutputPorts.end()) {
        port = *it;
    }
    return port;
}

void Node::setValueForInputPortKey(const boost::any& value, const std::string& key) {
    boost::any val = value;

    // access will add an entry if the key is not present 👎
    bool hasOldVal = hasValueForInputPortKey(key);
    boost::any oldVal;
    if (hasOldVal) {
        oldVal = mInputPortValueMap[key];
    }

    NodePortRef port = getInputPortForKey(key);
    bool valueDidChange = false;

    switch (port->getType()) {
        case NodePortType::FBOImage:
        case NodePortType::Texture:
            // TODO: pointer comparison?
            valueDidChange = true;
            break;
        case NodePortType::Bool:
            valueDidChange = hasOldVal ? boost::any_cast<bool>(val) != boost::any_cast<bool>(oldVal) : true;
            break;
        case NodePortType::Float:
            val = clampValue<float>(value, port->getValueMinimum(), port->getValueMaximum());
            valueDidChange = hasOldVal ? boost::any_cast<float>(val) != boost::any_cast<float>(oldVal) : true;
            break;
        case NodePortType::Int:
            val = clampValue<int>(value, port->getValueMinimum(), port->getValueMaximum());
            valueDidChange = hasOldVal ? boost::any_cast<int>(val) != boost::any_cast<int>(oldVal) : true;
            break;
        case NodePortType::Vec2:
            // TODO: clamp
            valueDidChange = hasOldVal ? boost::any_cast<ci::vec2>(val) != boost::any_cast<ci::vec2>(oldVal) : true;
            break;
        case NodePortType::Color:
            // TODO: clamp
            valueDidChange = hasOldVal ? boost::any_cast<ci::ColorA>(val) != boost::any_cast<ci::ColorA>(oldVal) : true;
            break;
        case NodePortType::Index:
            val = clampValue<int>(value, 0, static_cast<int>(port->getValues().size()) - 1);
            valueDidChange = hasOldVal ? boost::any_cast<int>(val) != boost::any_cast<int>(oldVal) : true;
            break;
        case NodePortType::FilePath:
            valueDidChange = hasOldVal ? boost::any_cast<ci::fs::path>(val) != boost::any_cast<ci::fs::path>(oldVal) : true;
            break;
        default:
            break;
    }

    // bail if the value is unchanged
    if (!valueDidChange) {
        return;
    }

    mInputPortValueMap[key] = val;

    // value changed handler
    if (mInputPortValueChangedHandlerMap.find(key) != mInputPortValueChangedHandlerMap.end()) {
        mInputPortValueChangedHandlerMap[key](key);
    }
}

#pragma mark -

template <typename T>
boost::any Node::clampValue(const boost::any& value, const boost::any& minimum, const boost::any& maximum) {
    T val = boost::any_cast<T>(value);
    if (!minimum.empty()) {
        T m = boost::any_cast<T>(minimum);
        val = val < m ? m : val;
    }
    if (!maximum.empty()) {
        T m = boost::any_cast<T>(maximum);
        val = val > m ? m : val;
    }
    return val;
}

}}
