//
//  Pipeline.h
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

typedef std::shared_ptr<class Pipeline> PipelineRef;

class Pipeline : public std::enable_shared_from_this<Pipeline> {
public:
    static PipelineRef create();
    ~Pipeline();

    // NB - attachments = max FBOImage input ports in any node + 1
    void setup(const Vec2i size, int attachments = 3);

    template<typename NodeT>
    std::shared_ptr<NodeT> makeNode(NodeT* node) {
        std::shared_ptr<NodeT> result(node);
//        result->setContext(shared_from_this());
        mNodes.push_back(result);
        return result;
    }
    // TODO - destroy

    void connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode, const NodePortRef& destinationNodePort);
    void connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode, const std::string& destinationNodePortKey);
    void connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode);
//    void disconnectNodes(const NodePortConnectionRef& connection);

    gl::Texture& evaluate(const NodeRef& node);

private:
    Pipeline();

    BranchRef branchForNode(const NodeRef& node);
    std::deque<BranchRef> renderStackForRootBranch(const BranchRef& branch);

    gl::Fbo mFBO;

    std::vector<NodeRef> mNodes;
    std::vector<NodePortConnectionRef> mConnections;
};

}}
