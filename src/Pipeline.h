//
//  Pipeline.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class Pipeline> PipelineRef;
typedef std::shared_ptr<class Branch> BranchRef;

class Branch : public std::enable_shared_from_this<Branch> {
public:
    static BranchRef create() { return BranchRef(new Branch())->shared_from_this(); }
    ~Branch() {}

    void setNodes(std::deque<NodeRef>& nodes) { mNodes = nodes; }
    const std::deque<NodeRef>& getNodes() const { return mNodes; }

    void connectInputBranch(const BranchRef& branch) {
        mInputBranches.push_back(std::make_tuple(branch, branch->getMaxInputCost()));

        unsigned int cost = 0;
        for (std::tuple<BranchRef, unsigned int> t : mInputBranches) {
            unsigned int c = std::get<1>(t);
            if (c > cost) {
                cost = c;
            }
        }
        setMaxInputCost(cost + mInputBranches.size());

        branch->connectOutputBranch(shared_from_this());
    }

    std::vector<std::tuple<BranchRef, unsigned int>>& getInputBranches() { return mInputBranches; }
    std::vector<BranchRef>& getOutputBranches() { return mOutputBranches; }

    void setMaxInputCost(unsigned int cost) { mMaxInputCost = cost; }
    unsigned int getMaxInputCost() const { return mMaxInputCost; }

private:
    Branch() : mMaxInputCost(0) {}

    void connectOutputBranch(const BranchRef& branch) { mOutputBranches.push_back(branch); }

    std::deque<NodeRef> mNodes;
    std::vector<std::tuple<BranchRef, unsigned int>> mInputBranches;
    std::vector<BranchRef> mOutputBranches;
    unsigned int mMaxInputCost;
};

class Pipeline : public std::enable_shared_from_this<Pipeline> {
public:
    static PipelineRef create();
    ~Pipeline();

    void setup(const Vec2i size);

    gl::Texture& evaluate(const NodeRef& node);

private:
    Pipeline();

//    BranchRef branchForNode(const NodeRef& node);
//    std::deque<BranchRef> renderStackForRootBranch(const BranchRef& branch);

    gl::Fbo mFBO;
};

}}
