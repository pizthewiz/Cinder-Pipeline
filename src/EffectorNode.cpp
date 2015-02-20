//
//  EffectorNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 May 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#include "EffectorNode.h"

namespace Cinder { namespace Pipeline {

using namespace ci;

const std::string EffectorNode::sVertexShaderPassThrough = R"(
    #version 150
    uniform mat4 ciModelViewProjection;

    in vec4 ciPosition;
    in vec4 ciTexCoord0;

    out highp vec2 TexCoord0;

    void main() {
        TexCoord0 = ciTexCoord0.st;
        gl_Position = ciModelViewProjection * ciPosition;
    }
)";

void EffectorNode::setupShader(const DataSourceRef& vertexShader, const DataSourceRef& fragmentShader) {
    std::string vert = vertexShader ? loadString(vertexShader) : sVertexShaderPassThrough;
    std::string frag = loadString(fragmentShader);
    setupShader(vert, frag);
}

void EffectorNode::setupShader(const std::string& vertexShader, const std::string& fragmentShader) {
    const char* vert = !vertexShader.empty() ? vertexShader.c_str() : sVertexShaderPassThrough.c_str();
    const char* frag = fragmentShader.c_str();
    mShader = gl::GlslProg::create(vert, frag);
}

}}
