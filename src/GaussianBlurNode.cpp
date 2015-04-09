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

static const std::string GaussianBlurNodeInputPortKeyDimension = "dimension";

// Based on Paul Houx's work at https://github.com/paulhoux/Cinder-Samples/blob/master/BloomingNeon/assets/blur.frag
//  https://forum.libcinder.org/topic/fbo-gaussian-blur-shader-101#23286000000989085
// and Daniel Rákos' work at http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
const std::string FragmentShaderGaussianBlur = R"(
    #version 150
    uniform sampler2D image;
    uniform vec2 direction;
    uniform float dimension;
    uniform float reach;
    uniform float mixAmount;

    in vec2 vTexCoord0;

    out vec4 oFragColor;

    const float offset[11] = float[](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0);
    const float weight[11] = float[](0.086826196862124602, 0.084895951965930902, 0.079358891804948081, 0.070921288047096992, 0.060594058578763078, 0.049494378859311142, 0.038650411513543079, 0.028855245532226279, 0.020595286319257878, 0.014053461291849008, 0.009167927656011385);

    void main() {
        vec4 color = texture(image, vTexCoord0);

        vec4 sum = texture(image, vTexCoord0) * weight[0];
        for (int idx=1; idx < 11; idx++) {
            sum += texture(image, vTexCoord0 + direction * offset[idx] / dimension * reach) * weight[idx];
            sum += texture(image, vTexCoord0 - direction * offset[idx] / dimension * reach) * weight[idx];
        }

        oFragColor = mix(color, sum, mixAmount);
    }
)";

GaussianBlurNode::GaussianBlurNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(GaussianBlurNodeInputPortKeyDirection, NodePortType::Index, "Direction", static_cast<int>(GaussianBlurNode::BlurDirection::Vertical), {static_cast<int>(GaussianBlurNode::BlurDirection::Vertical), static_cast<int>(GaussianBlurNode::BlurDirection::Horizontal)}, {"Vertical", "Horizontal"}),
        NodePort::create(GaussianBlurNodeInputPortKeyReach, NodePortType::Float, "Reach", 1.0f, 0.0f, 10.0f),
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
    gl::TextureBaseRef inputTexture = inputFBOImage->getTexture();

    int index = getValueForInputPortKey<int>(GaussianBlurNodeInputPortKeyDirection);
    int value = getInputPortForKey(GaussianBlurNodeInputPortKeyDirection)->getValues().at(index);
    vec2 direction = value == static_cast<int>(GaussianBlurNode::BlurDirection::Vertical) ? vec2(0.0f, 1.0f) : vec2(1.0f, 0.0f);
    float dimension = value == static_cast<int>(GaussianBlurNode::BlurDirection::Vertical) ? inputTexture->getHeight() : inputTexture->getWidth();
    float reach = getValueForInputPortKey<float>(GaussianBlurNodeInputPortKeyReach);
    float mixAmount = getValueForInputPortKey<float>(GaussianBlurNodeInputPortKeyMixAmount);

    gl::ScopedTextureBind texture(inputTexture, 0);
    gl::ScopedGlslProg shader(mShader);

    mShader->uniform(NodeInputPortKeyImage, 0);
    mShader->uniform(GaussianBlurNodeInputPortKeyDirection, direction);
    mShader->uniform(GaussianBlurNodeInputPortKeyDimension, dimension);
    mShader->uniform(GaussianBlurNodeInputPortKeyReach, reach);
    mShader->uniform(GaussianBlurNodeInputPortKeyMixAmount, mixAmount);
    gl::drawSolidRect(outputFBOImage->getFBO()->getBounds());
}
