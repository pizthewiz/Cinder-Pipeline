//
//  FileSourceNode.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 07 Jul 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#pragma once

#include "SourceNode.h"

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class FileSourceNode> FileSourceNodeRef;

static const std::string FileSourceNodeInputPortKeyImageFilePath = "filePath";

class FileSourceNode : public SourceNode {
public:
    FileSourceNode() {
        std::vector<NodePortRef> inputPorts = {
            NodePort::create(FileSourceNodeInputPortKeyImageFilePath, NodePortType::FilePath)
        };
        setInputPorts(inputPorts);

        // setup texture when file path changes
        connectValueForInputPortKeyChangedHandler(FileSourceNodeInputPortKeyImageFilePath, [&](const std::string key) {
            mTexture = nullptr;
            ci::fs::path path = getValueForInputPortKey<ci::fs::path>(key);
            // TODO - check if exists
            mTexture = ci::gl::Texture::create(ci::loadImage(path));
        });
    }
    ~FileSourceNode() {}

    FileSourceNodeRef getPtr() { return std::static_pointer_cast<FileSourceNode>(shared_from_this()); }

    std::string getName() override { return "File Source"; }

    void render(const FBOImageRef& outputFBOImage) override {
        if (!mTexture) {
            return;
        }
        ci::gl::draw(mTexture);
    }

private:
    ci::gl::TextureRef mTexture;
};

}}
