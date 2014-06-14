//
//  Pipeline.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "Pipeline.h"
#include "SourceNode.h"
#include "EffectorNode.h"
#include "cinder/Utilities.h"

using namespace ci;

namespace Cinder { namespace Pipeline {

PipelineRef Pipeline::create() {
    return PipelineRef(new Pipeline())->shared_from_this();
}

Pipeline::Pipeline() {
}

Pipeline::~Pipeline() {
}

#pragma mark -

void Pipeline::setup(const Vec2i size, int attachments) {
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

void Pipeline::connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode, const NodePortRef& destinationPort) {
    NodePortConnectionRef connection = NodePortConnection::create(sourceNode, "image", destinationNode, destinationPort->getKey());
    mConnections.push_back(connection);
}

void Pipeline::connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode, const std::string& destinationNodePortKey) {
    NodePortRef destinationPort = destinationNode->getInputPortForKey(destinationNodePortKey);
    connectNodes(sourceNode, destinationNode, destinationPort);
}

void Pipeline::connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode) {
    connectNodes(sourceNode, destinationNode, "image");
}

//void Pipeline::disconnectNodes(const NodePortConnectionRef& connection) {
//    // TODO - ???
//}

gl::Texture& Pipeline::evaluate(const NodeRef& node) {
    BranchRef root = branchForNode(node);
    std::deque<BranchRef> renderStack = renderStackForRootBranch(root);

#if defined(DEBUG)
    // ASCII visualization
    cinder::app::console() << std::string(13, '#') << std::endl;
    for (const BranchRef& b : renderStack) {
        unsigned int spaceCount = b->getMaxInputCost() * 5 + MAX((int)b->getMaxInputCost() - 1, 0) * 3;
        cinder::app::console() << std::string(spaceCount, ' ');

        for (const NodeRef& n : b->getNodes()) {
            if (!std::dynamic_pointer_cast<SourceNode>(n)) {
                cinder::app::console() << " → ";
            }

            std::string name = n->getName();
            name.resize(3, ' ');
            cinder::app::console() << "[" << name << "]";
        }
        cinder::app::console() << " (" << b->getMaxInputCost() << ")";
        cinder::app::console() << std::endl;
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
                                    NodePortConnectionRef connection = e->getConnectionForInputPortKey(key);
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

#pragma mark - PRIVATE

BranchRef Pipeline::branchForNode(const NodeRef& node) {
    std::deque<NodeRef> nodes;
    BranchRef branch = Branch::create();

    NodeRef n = node;
    while (n) {
        nodes.push_front(n);

        if (std::dynamic_pointer_cast<SourceNode>(n)) {
            n = nullptr;
        } else {
            std::vector<std::string> imageInputPortKeys = n->getImageInputPortKeys();
            if (imageInputPortKeys.size() == 1) {
                NodePortConnectionRef connection = n->getConnectionForInputPortKey(imageInputPortKeys.at(0));
                n = connection->getSourceNode();
            } else if (imageInputPortKeys.size() > 1) {
                for (const std::string& key : imageInputPortKeys) {
                    NodePortConnectionRef connection = n->getConnectionForInputPortKey(key);
                    BranchRef b = branchForNode(connection->getSourceNode());
                    branch->connectInputBranch(b);
                }

                n = nullptr;
            }
        }
    }
    branch->setNodes(nodes);

    return branch;
}

std::deque<BranchRef> Pipeline::renderStackForRootBranch(const BranchRef& branch) {
    std::deque<BranchRef> renderStack;
    std::deque<BranchRef> branchStack;

    BranchRef b = branch;
    while (b) {
        renderStack.push_front(b);

        if (b->getInputConnections().empty()) {
            if (branchStack.empty()) {
                b = nullptr;
            } else {
                b = branchStack.front();
                branchStack.pop_front();
            }
        } else {
            // sort by cost DESC
            std::vector<BranchConnectionRef> sortedInputConnections = b->getInputConnections();
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
