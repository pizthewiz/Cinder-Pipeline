//
//  Effector.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "Effector.h"
#include "cinder/Utilities.h"

namespace Cinder { namespace Pipeline {

const std::string Effector::sVertexShaderPassThrough = R"(
    #version 120
    void main() {
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = ftransform();
    }
)";

Effector::Effector(DataSourceRef vertexShader, DataSourceRef fragmentShader) {
    std::string vert = vertexShader ? loadString(vertexShader) : sVertexShaderPassThrough;
    std::string frag = loadString(fragmentShader);
    Effector(vert, frag);
}

Effector::Effector(const std::string& vertexShader, const std::string& fragmentShader) {
    setupShader(vertexShader, fragmentShader);
}

#pragma mark - 

void Effector::setupShader(const std::string& vertexShader, const std::string& fragmentShader) {
    const char* vert = !vertexShader.empty() ? vertexShader.c_str() : sVertexShaderPassThrough.c_str();
    const char* frag = fragmentShader.c_str();
    mShader = gl::GlslProg::create(vert, frag);
}

}}
