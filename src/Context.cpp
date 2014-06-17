//
//  Context.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "Context.h"
#include "SourceNode.h"
#include "EffectorNode.h"
#include "cinder/Utilities.h"

using namespace ci;

namespace Cinder { namespace Pipeline {

ContextRef Context::create() {
    return ContextRef(new Context())->shared_from_this();
}

Context::Context() {
}

Context::~Context() {
}

#pragma mark -

void Context::setup(const Vec2i size, int attachments) {
#if defined(DEBUG)
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    cinder::app::console() << "GL_RENDERER: " << renderer << std::endl;
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    cinder::app::console() << "GL_VENDOR: " << vendor << std::endl;
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    cinder::app::console() << "GL_VERSION: " << version << std::endl;
    const char* shadingLanguageVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    cinder::app::console() << "GL_SHADING_LANGUAGE_VERSION: " << shadingLanguageVersion << std::endl;

    std::string extensionsString = std::string(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
    std::vector<std::string> extensions = split(extensionsString, " ");
    extensions.erase(std::remove_if(extensions.begin(), extensions.end(), [](const std::string& s){ return s.empty(); }));
    cinder::app::console() << "GL_EXTENSIONS: " << std::endl;
    for (const std::string& e : extensions) {
        cinder::app::console() << " " << e << std::endl;
    }

    GLint texSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
    cinder::app::console() << "GL_MAX_TEXTURE_SIZE: " << texSize << std::endl;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &texSize);
    cinder::app::console() << "GL_MAX_3D_TEXTURE_SIZE: " << texSize << std::endl;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texSize);
    cinder::app::console() << "GL_MAX_TEXTURE_IMAGE_UNITS: " << texSize << std::endl;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &texSize);
    cinder::app::console() << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: " << texSize << std::endl;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &texSize);
    cinder::app::console() << "GL_MAX_COLOR_ATTACHMENTS: " << texSize << std::endl;
    cinder::app::console() << std::string(13, '-') << std::endl;
#endif

//    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
//    if (size.x > texSize) {
//        // TODO - tile horizontally
//        cinder::app::console() << "ERROR - setup width '" << size.x << "' exceeds maximum texture size '" << texSize << "', tiling unimplemented" << std::endl;
//    }
//    if (size.y > texSize) {
//        // TODO - tile vertically
//        cinder::app::console() << "ERROR - setup height '" << size.y << "' exceeds maximum texture size '" << texSize << "', tiling unimplemented" << std::endl;
//    }
//
//    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &texSize);
//    if (attachments > GL_MAX_COLOR_ATTACHMENTS) {
//        cinder::app::console() << "ERROR - number of attachments '" << attachments << "' exceeds '" << texSize << "'" << std::endl;
//    }
//
//    float attachmentMemorySizeMB = size.x * size.y * 4 / 1024 / 1024;
//    float totalSizeMB = attachmentMemorySizeMB * attachments;
//
//    if (gl::isExtensionAvailable("GL_NV_texture_barrier")) {
//        if (texSize >= 2 * size.x && texSize >= size.y) {
//            // TODO - double-wide
//        } else if (texSize >= 2 * size.y && texSize >= size.x) {
//            // TODO - double-tall
//        } else {
//            // TODO - tile
//        }
//    }

    gl::Fbo::Format format;
    format.enableColorBuffer(true, attachments);
    format.enableDepthBuffer(false);

    mFBO = gl::Fbo(size.x, size.y, format);
    mFBO.bindFramebuffer(); {
        GLenum buffers[attachments];
        for (unsigned int idx = 0; idx < attachments; idx++) {
            buffers[idx] = GL_COLOR_ATTACHMENT0 + idx;
        }
        glDrawBuffers(attachments, buffers);
        gl::setViewport(mFBO.getBounds());
        gl::clear();
    } mFBO.unbindFramebuffer();
}

#pragma mark - CONNECTIONS

void Context::connectNodes(const NodeRef& sourceNode, const NodePortRef& sourcePort, const NodeRef& destinationNode, const NodePortRef& destinationPort) {
    // remove from output connections if destination port already connected
    if (mInputConnections[destinationNode].count(destinationPort->getKey()) > 0) {
        std::vector<NodePortConnectionRef> connections = mOutputConnections[sourceNode][sourcePort->getKey()];
        connections.erase(std::remove_if(connections.begin(), connections.end(), [destinationNode](const NodePortConnectionRef& c) {
            return c->getDestinationNode() == destinationNode;
        }), connections.end());
    }

    NodePortConnectionRef connection = NodePortConnection::create(sourceNode, sourcePort->getKey(), destinationNode, destinationPort->getKey());
    mInputConnections[destinationNode][destinationPort->getKey()] = connection;
    mOutputConnections[sourceNode][sourcePort->getKey()].push_back(connection);
}

void Context::connectNodes(const NodeRef& sourceNode, const std::string& sourceNodePortKey, const NodeRef& destinationNode, const std::string& destinationNodePortKey) {
    NodePortRef sourcePort = sourceNode->getOutputPortForKey(sourceNodePortKey);
    NodePortRef destinationPort = destinationNode->getInputPortForKey(destinationNodePortKey);
    connectNodes(sourceNode, sourcePort, destinationNode, destinationPort);
}

void Context::connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode) {
    connectNodes(sourceNode, "image", destinationNode, "image");
}

#pragma mark -

gl::Texture& Context::evaluate(const NodeRef& node) {
    BranchRef root = branchForNode(node);
    std::deque<BranchRef> renderStack = renderStackForRootBranch(root);

#if defined(DEBUG)
    // ASCII visualization
    cinder::app::console() << std::string(3, '#') << std::endl;
    for (const BranchRef& b : renderStack) {
        printBranch(b);
    }
    cinder::app::console() << std::endl;
#endif

    // render branches
    unsigned int outAttachment = 0;

    Area viewport = gl::getViewport();
    gl::setViewport(mFBO.getBounds());
    mFBO.bindFramebuffer(); {
        gl::pushMatrices(); {
            gl::setMatricesWindow(mFBO.getSize(), false);

            // int instead of GLenum for Cinder's FBO bindTexture/getTexture
            std::vector<int> availableAttachments;
            for (unsigned int idx = 0; idx < mFBO.getFormat().getNumColorBuffers(); idx++) {
                availableAttachments.push_back(idx);
            }
            std::deque<std::tuple<int, NodeRef>> attachmentsStack;

            for (const BranchRef& b : renderStack) {
                size_t attachmentIndex = 0;
                outAttachment = availableAttachments.at(attachmentIndex);
                int inAttachment = -1;

                for (size_t nodeIdx = 0; nodeIdx < b->getNodes().size(); nodeIdx++) {
                    // TODO - hoist draw buffer assignment
                    NodeRef n = b->getNodes().at(nodeIdx);
                    SourceNodeRef s = std::dynamic_pointer_cast<SourceNode>(n);
                    if (s) {
                        glDrawBuffer(GL_COLOR_ATTACHMENT0 + outAttachment);

                        FBOImageRef outputFBOImage = FBOImage::create(mFBO, outAttachment);
                        s->render(outputFBOImage);

                        inAttachment = outAttachment;
                    } else {
                        EffectorNodeRef e = std::dynamic_pointer_cast<EffectorNode>(n);
                        if (e) {
                            std::vector<std::string> imageInputPortKeys = n->getImageInputPortKeys();
                            size_t numberOfImageInputPorts = imageInputPortKeys.size();
                            if (numberOfImageInputPorts == 1) {
                                if (inAttachment == -1) {
                                    // grab off the stack
                                    std::tuple<int, NodeRef> t = attachmentsStack.front();
                                    inAttachment = std::get<0>(t);
                                    attachmentsStack.pop_front();
                                    availableAttachments.push_back(inAttachment);

                                }
                                FBOImageRef inputFBOImage = FBOImage::create(mFBO, inAttachment);
                                e->setValueForInputPortKey(inputFBOImage, imageInputPortKeys.at(0));

                                attachmentIndex = (attachmentIndex + 1) % availableAttachments.size();
                                outAttachment = availableAttachments.at(attachmentIndex);
                                glDrawBuffer(GL_COLOR_ATTACHMENT0 + outAttachment);

                                FBOImageRef outputFBOImage = FBOImage::create(mFBO, outAttachment);
                                e->render(outputFBOImage);

                                inAttachment = outAttachment;
                            } else if (numberOfImageInputPorts > 1) {
                                std::vector<std::tuple<int, NodeRef>> inputAttachments(numberOfImageInputPorts);
                                inputAttachments.assign(attachmentsStack.begin(), attachmentsStack.begin() + numberOfImageInputPorts);
                                attachmentsStack.erase(attachmentsStack.begin(), attachmentsStack.begin() + numberOfImageInputPorts);

                                for (const std::string& key : imageInputPortKeys) {
                                    NodePortConnectionRef connection = getInputConnectionForNodeWithPortKey(e, key);
                                    NodeRef inputNode = connection->getSourceNode();
                                    auto it = std::find_if(inputAttachments.begin(), inputAttachments.end(), [inputNode](std::tuple<int, NodeRef> t) {
                                        return std::get<1>(t) == inputNode;
                                    });
                                    inAttachment = std::get<0>(*it);
                                    inputAttachments.erase(it);

                                    FBOImageRef inputFBOImage = FBOImage::create(mFBO, inAttachment);
                                    e->setValueForInputPortKey(inputFBOImage, key);

                                    availableAttachments.push_back(inAttachment);
                                }

                                glDrawBuffer(GL_COLOR_ATTACHMENT0 + outAttachment);

                                FBOImageRef outputFBOImage = FBOImage::create(mFBO, outAttachment);
                                e->render(outputFBOImage);
                                
                                inAttachment = outAttachment;
                            }
                        }
                    }

                    // stash output attachment and accompanying node when branch concludes
                    if (nodeIdx == b->getNodes().size() - 1) {
                        // TODO - could also be a std::map<NodeRef, int> attachmentsMap;
                        attachmentsStack.push_front(std::make_tuple(outAttachment, n));
                        availableAttachments.erase(std::find(availableAttachments.begin(), availableAttachments.end(), outAttachment));
                    }
                }
            }
        } gl::popMatrices();
    } mFBO.unbindFramebuffer();
    gl::setViewport(viewport);

    return mFBO.getTexture(outAttachment);
}

#pragma mark -

void Context::printBranch(const BranchRef& branch) {
//    for (const NodeRef& n : branch->getNodes()) {
//        cinder::app::console() << n->getName() << " → ";
//    }
//    cinder::app::console() << " (" << branch->getMaxInputCost() << ")" << std::endl;
    for (const NodeRef& n : branch->getNodes()) {
        if (!std::dynamic_pointer_cast<SourceNode>(n)) {
            cinder::app::console() << " → ";
        }

        std::string name = n->getName();
        name.resize(3, ' ');
        cinder::app::console() << "[" << name << "]";
    }
    cinder::app::console() << " (" << branch->getMaxInputCost() << ")";
    cinder::app::console() << std::endl;
}

BranchRef Context::branchForNode(const NodeRef& node) {
    std::map<NodeRef, BranchRef> branchMap;
    std::deque<NodeRef> nodeStack;

    // create branches
    NodeRef n = node;
    while (n) {
        if (branchMap.count(n) == 0) {
            std::deque<NodeRef> nodes;

            NodeRef n2 = n;
            while (n2) {
                nodes.push_front(n2);

                std::vector<NodePortConnectionRef> connections = getInputConnectionsForNodeWithPortType(n2, NodePortType::FBOImage);
                if (connections.size() == 0) {
                    n2 = nullptr;
                } else if (connections.size() == 1) {
                    NodePortConnectionRef connection = connections.at(0);
                    n2 = connection->getSourceNode();

                    if (getOutputConnectionsForNodeWithPortKey(n2, connection->getSourcePortKey()).size() > 1) {
                        nodeStack.push_front(n2);
                        n2 = nullptr;
                    }
                } else if (connections.size() > 1) {
                    for (const NodePortConnectionRef& c : connections) {
                        nodeStack.push_front(c->getSourceNode());
                    }
                    n2 = nullptr;
                }
            }

            BranchRef branch = Branch::create();
            branch->setNodes(nodes);
            branchMap[n] = branch;
        }

        if (nodeStack.empty()) {
            n = nullptr;
        } else {
            n = nodeStack.front();
            nodeStack.pop_front();
        }
    }

    // connect branches
    for (auto& kv : branchMap) {
        BranchRef destinationBranch = kv.second;
        n = destinationBranch->getNodes().front();

        std::vector<NodePortConnectionRef> connections = getInputConnectionsForNodeWithPortType(n, NodePortType::FBOImage);
        for (const NodePortConnectionRef& c : connections) {
            BranchRef sourceBranch = branchMap[c->getSourceNode()];
            destinationBranch->connectInputBranch(sourceBranch);
        }
    }

    return branchMap[node];
}

std::deque<BranchRef> Context::renderStackForRootBranch(const BranchRef& branch) {
    std::deque<BranchRef> renderStack;
    std::deque<BranchRef> branchStack;

    BranchRef b = branch;
    while (b) {
        renderStack.push_front(b);

        std::vector<BranchConnectionRef> inputConnections = b->getInputConnections();
        if (inputConnections.empty()) {
            if (branchStack.empty()) {
                b = nullptr;
            } else {
                b = branchStack.front();
                branchStack.pop_front();
            }
        } else {
            // sort by cost DESC
            std::vector<BranchConnectionRef> sortedInputConnections = inputConnections;
            std::sort(sortedInputConnections.begin(), sortedInputConnections.end(), [](const BranchConnectionRef& c1, const BranchConnectionRef& c2) {
                return c1->getCost() > c2->getCost();
            });

            // follow cheapest path
            b = sortedInputConnections.back()->getSourceBranch();
            sortedInputConnections.pop_back();

            // push the others
            for (const BranchConnectionRef& c : sortedInputConnections) {
                branchStack.push_front(c->getSourceBranch());
            }
        }
    }

    return renderStack;
}

}}
