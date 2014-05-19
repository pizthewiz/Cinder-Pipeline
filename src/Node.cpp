//
//  Node.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "Node.h"

namespace Cinder { namespace Pipeline {

void Node::connectOutputNode(const NodeRef& node, const std::string& key, const std::string& outputPortKey) {
    if (!node->inputPortForKey(key)) {
        cinder::app::console() << "ERROR - attempting to connect to unknown input port '" << key << "'" << std::endl;
        return;
    }

    // TODO - check that types match, no cycle
    
//    mOutputConnectionMap[outputPortKey].push_back(std::make_tuple(node, key));
    node->connectInputNode(shared_from_this(), outputPortKey, key);
}

void Node::connectInputNode(const NodeRef& source, const std::string& sourceKey, const std::string& destinationKey) {
    mInputConnections[destinationKey] = NodePortConnection::create(source, sourceKey, nullptr, destinationKey);
}

}}
