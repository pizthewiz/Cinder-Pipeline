//
//  ShadowsHighlightsNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 12 Apr 2015.
//  Copyright 2015 Chorded Constructions. All rights reserved.
//

#include "ShadowsHighlightsNode.h"

using namespace ci;
using namespace Cinder::Pipeline;

// Taken from Brad Larson's GPUImage https://github.com/BradLarson/GPUImage/blob/master/framework/Source/GPUImageHighlightShadowFilter.m
const std::string FragmentShaderVibrance = R"(
    #version 150
    uniform sampler2D image;
    uniform float shadows;
    uniform float highlights;
    uniform float mixAmount;

    in vec2 vTexCoord0;

    out vec4 oFragColor;

    const vec3 luminanceWeighting = vec3(0.3, 0.3, 0.3);

    void main() {
        vec4 color = texture(image, vTexCoord0);

        float luminance = dot(color.rgb, luminanceWeighting);

        float shadow = clamp((pow(luminance, 1.0/(shadows+1.0)) + (-0.76)*pow(luminance, 2.0/(shadows+1.0))) - luminance, 0.0, 1.0);
        float highlight = clamp((1.0 - (pow(1.0-luminance, 1.0/(2.0-highlights)) + (-0.8)*pow(1.0-luminance, 2.0/(2.0-highlights)))) - luminance, -1.0, 0.0);
        vec3 r = vec3(0.0, 0.0, 0.0) + ((luminance + shadow + highlight) - 0.0) * ((color.rgb - vec3(0.0, 0.0, 0.0))/(luminance - 0.0));
        vec4 result = vec4(r, color.a);

        oFragColor = mix(color, result, mixAmount);
    }
)";

ShadowsHighlightsNode::ShadowsHighlightsNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(ShadowsHighlightsNodeInputPortKeyShadows, NodePortType::Float, "Shadows", 0.0f, 0.0f, 1.0f),
        NodePort::create(ShadowsHighlightsNodeInputPortKeyHighlights, NodePortType::Float, "Highlights", 1.0f, 0.0f, 1.0f),
        NodePort::create(ShadowsHighlightsNodeInputPortKeyMixAmount, NodePortType::Float, "Mix", 1.0f, 0.0f, 1.0f),
    };
    setInputPorts(inputPorts);
    // NB - output port "image" of type NodePortType::FBOImage is already present

    setupShader(sVertexShaderPassThrough, FragmentShaderVibrance);
}

ShadowsHighlightsNode::~ShadowsHighlightsNode() {
}

void ShadowsHighlightsNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>(NodeInputPortKeyImage);
    float shadows = getValueForInputPortKey<float>(ShadowsHighlightsNodeInputPortKeyShadows);
    float highlights = getValueForInputPortKey<float>(ShadowsHighlightsNodeInputPortKeyHighlights);
    float mixAmount = getValueForInputPortKey<float>(ShadowsHighlightsNodeInputPortKeyMixAmount);

    gl::ScopedTextureBind texture(inputFBOImage->getTexture(), 0);
    gl::ScopedGlslProg shader(mShader);

    mShader->uniform(NodeInputPortKeyImage, 0);
    mShader->uniform(ShadowsHighlightsNodeInputPortKeyShadows, shadows);
    mShader->uniform(ShadowsHighlightsNodeInputPortKeyHighlights, highlights);
    mShader->uniform(ShadowsHighlightsNodeInputPortKeyMixAmount, mixAmount);
    gl::drawSolidRect(outputFBOImage->getFBO()->getBounds());
}
