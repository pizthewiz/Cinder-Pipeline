//
//  GaussianBlurNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 22 Apr 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#include "GaussianBlurNode.h"

using namespace ci;
using namespace Cinder::Pipeline;

// (c) Paul Houx, 2014. https://github.com/paulhoux/Cinder-Samples/blob/master/BloomingNeon/assets/blur.frag
// plus tweak suggested in thread: https://forum.libcinder.org/topic/fbo-gaussian-blur-shader-101#23286000000989085
const std::string FragmentShaderGaussianBlur = R"(
    #version 150
    uniform sampler2D image;
    uniform vec2 pixelSize;
    uniform float mixAmount;

    in vec2 vTexCoord0;

    out vec4 FragColor;

    void main() {
        vec2 position = vTexCoord0;
        vec4 color = texture(image, position);

        vec4 sum = vec4(0.0);
        sum += texture(image, position + -10.0 * pixelSize) * 0.009167927656011385;
        sum += texture(image, position +  -9.0 * pixelSize) * 0.014053461291849008;
        sum += texture(image, position +  -8.0 * pixelSize) * 0.020595286319257878;
        sum += texture(image, position +  -7.0 * pixelSize) * 0.028855245532226279;
        sum += texture(image, position +  -6.0 * pixelSize) * 0.038650411513543079;
        sum += texture(image, position +  -5.0 * pixelSize) * 0.049494378859311142;
        sum += texture(image, position +  -4.0 * pixelSize) * 0.060594058578763078;
        sum += texture(image, position +  -3.0 * pixelSize) * 0.070921288047096992;
        sum += texture(image, position +  -2.0 * pixelSize) * 0.079358891804948081;
        sum += texture(image, position +  -1.0 * pixelSize) * 0.084895951965930902;
        sum += texture(image, position +   0.0 * pixelSize) * 0.086826196862124602;
        sum += texture(image, position +  +1.0 * pixelSize) * 0.084895951965930902;
        sum += texture(image, position +  +2.0 * pixelSize) * 0.079358891804948081;
        sum += texture(image, position +  +3.0 * pixelSize) * 0.070921288047096992;
        sum += texture(image, position +  +4.0 * pixelSize) * 0.060594058578763078;
        sum += texture(image, position +  +5.0 * pixelSize) * 0.049494378859311142;
        sum += texture(image, position +  +6.0 * pixelSize) * 0.038650411513543079;
        sum += texture(image, position +  +7.0 * pixelSize) * 0.028855245532226279;
        sum += texture(image, position +  +8.0 * pixelSize) * 0.020595286319257878;
        sum += texture(image, position +  +9.0 * pixelSize) * 0.014053461291849008;
        sum += texture(image, position + +10.0 * pixelSize) * 0.009167927656011385;

        FragColor = mix(color, sum, mixAmount);
    }
)";

GaussianBlurNode::GaussianBlurNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        // TODO - would be nice if this were a resolution independent value, a multiplier maybe
        NodePort::create(GaussianBlurNodeInputPortKeyPixelSize, NodePortType::Vec2, "Size"),
        NodePort::create(GaussianBlurNodeInputPortKeyMixAmount, NodePortType::Float, "Mix", 1.0f, 0.0f, 1.0f),
    };
    setInputPorts(inputPorts);
    // NB - output port "image" of type NodePortType::FBOImage is already present

    setupShader(sVertexShaderPassThrough, FragmentShaderGaussianBlur);
}

GaussianBlurNode::~GaussianBlurNode() {
}

void GaussianBlurNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>(NodeInputPortKeyImage);
    vec2 pixelSize = getValueForInputPortKey<vec2>(GaussianBlurNodeInputPortKeyPixelSize);
    float mixAmount = getValueForInputPortKey<float>(GaussianBlurNodeInputPortKeyMixAmount);

    gl::ScopedTextureBind texture(inputFBOImage->getTexture(), 0);
    gl::ScopedGlslProg shader(mShader);

    mShader->uniform(NodeInputPortKeyImage, 0);
    mShader->uniform(GaussianBlurNodeInputPortKeyPixelSize, pixelSize);
    mShader->uniform(GaussianBlurNodeInputPortKeyMixAmount, mixAmount);
    gl::drawSolidRect(outputFBOImage->getFBO()->getBounds());
}
