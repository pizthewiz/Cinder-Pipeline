//
//  TintNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Jun 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#include "TintNode.h"

using namespace ci;
using namespace Cinder::Pipeline;

const std::string FragmentShaderTint = R"(
    #version 150
    uniform sampler2D image;
    uniform vec4 tintColor;
    uniform float mixAmount;

    in vec2 vTexCoord0;

    out vec4 oFragColor;

    // http://en.wikipedia.org/wiki/Luma_(video)
    float lumaRec601(vec4 color) {
        return dot(color.rgb, vec3(0.299, 0.587, 0.114));
    }
    // http://en.wikipedia.org/wiki/Luminance_(colorimetry)
    float lumaRec709(vec4 color) {
        return dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    }

    void main() {
        vec4 color = texture(image, vTexCoord0);
        float y = lumaRec601(color);
        vec4 result = tintColor * vec4(y, y, y, 1.0);
        oFragColor = mix(color, result, mixAmount);
    }
)";

TintNode::TintNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(TintNodeInputPortKeyColor, NodePortType::Color, "Color", ColorAf(1.0f, 1.0f, 1.0f, 1.0f)),
        NodePort::create(TintNodeInputPortKeyMix, NodePortType::Float, "Mix", 1.0f, 0.0f, 1.0f),
    };
    setInputPorts(inputPorts);
    // NB - output port "image" of type NodePortType::FBOImage is already present

    setupShader(sVertexShaderPassThrough, FragmentShaderTint);
}

TintNode::~TintNode() {
}

void TintNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>(NodeInputPortKeyImage);
    ColorAf tintColor = getValueForInputPortKey<ColorAf>(TintNodeInputPortKeyColor);
    float mixAmount = getValueForInputPortKey<float>(TintNodeInputPortKeyMix);

    gl::ScopedTextureBind texture(inputFBOImage->getTexture(), 0);
    gl::ScopedGlslProg shader(mShader);

    mShader->uniform(NodeInputPortKeyImage, 0);
    mShader->uniform(TintNodeInputPortKeyColor, tintColor);
    mShader->uniform(TintNodeInputPortKeyMix, mixAmount);
    gl::drawSolidRect(outputFBOImage->getFBO()->getBounds());
}
