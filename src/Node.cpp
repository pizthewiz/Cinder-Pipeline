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
    mInputPortValueMap[key] = value;

    // value changed handler
    if (mInputPortValueChangedHandlerMap.find(key) != mInputPortValueChangedHandlerMap.end()) {
        mInputPortValueChangedHandlerMap[key](key);
    }
}

}}
