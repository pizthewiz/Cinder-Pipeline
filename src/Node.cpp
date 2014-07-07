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
    NodePortRef port = getInputPortForKey(key);

    switch (port->getType()) {
        case NodePortType::Float:
            val = clampValue<float>(value, port->getValueMinimum(), port->getValueMaximum());
            break;
        case NodePortType::Int:
            val = clampValue<int>(value, port->getValueMinimum(), port->getValueMaximum());
            break;
        case NodePortType::Vec2f:
        case NodePortType::Vec4f:
            // TODO - ?
            break;
        case NodePortType::FBOImage:
        case NodePortType::Texture:
        case NodePortType::Bool:
        case NodePortType::FilePath:
        default:
            break;
    }

    // TODO - bail if value is unchanged http://stackoverflow.com/questions/6029092/compare-boostany-contents/6029595#6029595

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
