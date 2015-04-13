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
    // copy to allow manipulation via clamp
    boost::any valueShadow = value;

    // access will add an entry if the key is not present 👎
    bool hasOldValue = hasValueForInputPortKey(key);
    boost::any oldValue;
    if (hasOldValue) {
        oldValue = mInputPortValueMap[key];
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
            valueDidChange = hasOldValue ? boost::any_cast<bool>(valueShadow) != boost::any_cast<bool>(oldValue) : true;
            break;
        case NodePortType::Float:
            valueShadow = clampValue<float>(value, port->getValueMinimum(), port->getValueMaximum());
            valueDidChange = hasOldValue ? boost::any_cast<float>(valueShadow) != boost::any_cast<float>(oldValue) : true;
            break;
        case NodePortType::Int:
            valueShadow = clampValue<int>(value, port->getValueMinimum(), port->getValueMaximum());
            valueDidChange = hasOldValue ? boost::any_cast<int>(valueShadow) != boost::any_cast<int>(oldValue) : true;
            break;
        case NodePortType::Vec2:
            // TODO: clamp
            valueDidChange = hasOldValue ? boost::any_cast<ci::vec2>(valueShadow) != boost::any_cast<ci::vec2>(oldValue) : true;
            break;
        case NodePortType::Color:
            // TODO: clamp
            valueDidChange = hasOldValue ? boost::any_cast<ci::ColorA>(valueShadow) != boost::any_cast<ci::ColorA>(oldValue) : true;
            break;
        case NodePortType::Index:
            valueShadow = clampValue<int>(value, 0, static_cast<int>(port->getValues().size()) - 1);
            valueDidChange = hasOldValue ? boost::any_cast<int>(valueShadow) != boost::any_cast<int>(oldValue) : true;
            break;
        case NodePortType::FilePath:
            valueDidChange = hasOldValue ? boost::any_cast<ci::fs::path>(valueShadow) != boost::any_cast<ci::fs::path>(oldValue) : true;
            break;
        default:
            break;
    }

    // bail if the value is unchanged
    if (!valueDidChange) {
        return;
    }

    mInputPortValueMap[key] = valueShadow;

    // value changed handler
    if (mInputPortValueChangedHandlerMap.find(key) != mInputPortValueChangedHandlerMap.end()) {
        mInputPortValueChangedHandlerMap[key](key, valueShadow, oldValue);
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
