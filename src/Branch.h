//
//  Branch.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 May 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class Branch> BranchRef;
typedef std::shared_ptr<class BranchConnection> BranchConnectionRef;

class BranchConnection : public std::enable_shared_from_this<BranchConnection> {
public:
    static BranchConnectionRef create(const BranchRef& source, const BranchRef& destination, unsigned int cost = 0) {
        return BranchConnectionRef(new BranchConnection(source, destination, cost))->shared_from_this();
    }

    ~BranchConnection() {}

    inline BranchRef& getSourceBranch() {
        return mSourceBranch;
    }
    inline BranchRef& getDestinationBranch() {
        return mDestinationBranch;
    }
    inline unsigned int getCost() const {
        return mCost;
    }

private:
    BranchConnection(const BranchRef& source, const BranchRef& destination, unsigned int cost) : mSourceBranch(source), mDestinationBranch(destination), mCost(cost) {}

    BranchRef mSourceBranch;
    BranchRef mDestinationBranch;
    unsigned int mCost;
};

class Branch : public std::enable_shared_from_this<Branch> {
public:
    static BranchRef create() { return BranchRef(new Branch())->shared_from_this(); }
    ~Branch() {}

    void setNodes(std::deque<NodeRef>& nodes) { mNodes = nodes; }
    const std::deque<NodeRef>& getNodes() const { return mNodes; }

    void connectInputBranch(const BranchRef& branch) {
        BranchConnectionRef connection = BranchConnection::create(branch, shared_from_this(), branch->getMaxInputCost());
        mInputConnections.push_back(connection);

        auto result = std::max_element(mInputConnections.begin(), mInputConnections.end(), [](const BranchConnectionRef& c1, const BranchConnectionRef& c2) {
            return c1->getCost() < c2->getCost();
        });
        unsigned int cost = mInputConnections.at(std::distance(mInputConnections.begin(), result))->getCost();
        setMaxInputCost(cost + (unsigned int)mInputConnections.size());

        branch->addOutputConnection(connection);
    }

    std::vector<BranchConnectionRef>& getInputConnections() { return mInputConnections; }
    std::vector<BranchConnectionRef>& getOutputConnections() { return mOutputConnections; }

    void setMaxInputCost(unsigned int cost) { mMaxInputCost = cost; }
    unsigned int getMaxInputCost() const { return mMaxInputCost; }

private:
    Branch() : mMaxInputCost(0) {}

    void addOutputConnection(const BranchConnectionRef& connection) { mOutputConnections.push_back(connection); }

    std::deque<NodeRef> mNodes;
    std::vector<BranchConnectionRef> mInputConnections;
    std::vector<BranchConnectionRef> mOutputConnections;
    unsigned int mMaxInputCost;
};

}}
