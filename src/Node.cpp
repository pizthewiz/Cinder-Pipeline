//
//  Node.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "Node.h"

namespace Cinder { namespace Pipeline {

void Node::connectOutputNode(const NodeRef& destination, const std::string& destinationKey, const std::string& sourceKey) {
    if (!destination->getInputPortForKey(destinationKey)) {
        cinder::app::console() << "ERROR - attempting to connect to unknown input port '" << destinationKey << "'" << std::endl;
        return;
    }

    // TODO - check that types match, no cycle
    
//    mOutputConnectionMap[outputPortKey].push_back(std::make_tuple(node, key));
    destination->connectInputNode(shared_from_this(), sourceKey, destinationKey);
}

void Node::connectInputNode(const NodeRef& source, const std::string& sourceKey, const std::string& destinationKey) {
    mInputConnections[destinationKey] = NodePortConnection::create(source, sourceKey, nullptr, destinationKey);
}

}}
