//
//  Context.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"
#include "Branch.h"
#include "cinder/gl/Fbo.h"

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class Context> ContextRef;
typedef std::shared_ptr<class NodePortConnection> NodePortConnectionRef;

class NodePortConnection : public std::enable_shared_from_this<NodePortConnection> {
public:
    static NodePortConnectionRef create(const NodeRef& source, const std::string& sourceKey, const NodeRef& destination, const std::string& destinationKey) {
        return NodePortConnectionRef(new NodePortConnection(source, sourceKey, destination, destinationKey))->shared_from_this();
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


class Context : public std::enable_shared_from_this<Context> {
public:
    static ContextRef create();
    ~Context();

    // NB - attachments = max FBOImage input ports in any node + 1
    void setup(const Vec2i size, int attachments = 3);

    template<typename NodeT>
    std::shared_ptr<NodeT> makeNode(NodeT* node) {
        std::shared_ptr<NodeT> result(node);
//        result->setContext(shared_from_this());
        mNodes.push_back(result);
        return result;
    }

    void connectNodes(const NodeRef& sourceNode, const std::string& sourceNodePortKey, const NodeRef& destinationNode, const std::string& destinationNodePortKey);
    void connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode);

    inline NodePortConnectionRef getConnectionForNodeWithInputPortKey(const NodeRef& node, const std::string& portKey) {
        return mInputConnections[node][portKey];
    }

    gl::Texture& evaluate(const NodeRef& node);

private:
    Context();

    void connectNodes(const NodeRef& sourceNode, const NodePortRef& sourceNodePort, const NodeRef& destinationNode, const NodePortRef& destinationNodePort);

    BranchRef branchForNode(const NodeRef& node);
    std::deque<BranchRef> renderStackForRootBranch(const BranchRef& branch);

    gl::Fbo mFBO;

    std::vector<NodeRef> mNodes;
    // {node -> {key -> connection}}
    std::map<NodeRef, std::map<std::string, NodePortConnectionRef>> mInputConnections;
};

}}
