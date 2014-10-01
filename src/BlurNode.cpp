//
//  BlurNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 22 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "BlurNode.h"

using namespace ci;
using namespace Cinder::Pipeline;

// (c) Paul Houx, 2014. https://github.com/paulhoux/Cinder-Samples/blob/master/BloomingNeon/assets/blur.frag
// plus tweak suggested in thread: https://forum.libcinder.org/topic/fbo-gaussian-blur-shader-101#23286000000989085
const std::string FragmentShaderBlur = R"(
    #version 120
    uniform sampler2D image;
    uniform vec2 pixelSize;
    uniform float mixAmount;

    void main() {
        vec2 position = gl_TexCoord[0].st;
        vec4 color = texture2D(image, position);

        vec4 sum = vec4(0.0);
        sum += texture2D(image, position + -10.0 * pixelSize) * 0.009167927656011385;
        sum += texture2D(image, position +  -9.0 * pixelSize) * 0.014053461291849008;
        sum += texture2D(image, position +  -8.0 * pixelSize) * 0.020595286319257878;
        sum += texture2D(image, position +  -7.0 * pixelSize) * 0.028855245532226279;
        sum += texture2D(image, position +  -6.0 * pixelSize) * 0.038650411513543079;
        sum += texture2D(image, position +  -5.0 * pixelSize) * 0.049494378859311142;
        sum += texture2D(image, position +  -4.0 * pixelSize) * 0.060594058578763078;
        sum += texture2D(image, position +  -3.0 * pixelSize) * 0.070921288047096992;
        sum += texture2D(image, position +  -2.0 * pixelSize) * 0.079358891804948081;
        sum += texture2D(image, position +  -1.0 * pixelSize) * 0.084895951965930902;
        sum += texture2D(image, position +   0.0 * pixelSize) * 0.086826196862124602;
        sum += texture2D(image, position +  +1.0 * pixelSize) * 0.084895951965930902;
        sum += texture2D(image, position +  +2.0 * pixelSize) * 0.079358891804948081;
        sum += texture2D(image, position +  +3.0 * pixelSize) * 0.070921288047096992;
        sum += texture2D(image, position +  +4.0 * pixelSize) * 0.060594058578763078;
        sum += texture2D(image, position +  +5.0 * pixelSize) * 0.049494378859311142;
        sum += texture2D(image, position +  +6.0 * pixelSize) * 0.038650411513543079;
        sum += texture2D(image, position +  +7.0 * pixelSize) * 0.028855245532226279;
        sum += texture2D(image, position +  +8.0 * pixelSize) * 0.020595286319257878;
        sum += texture2D(image, position +  +9.0 * pixelSize) * 0.014053461291849008;
        sum += texture2D(image, position + +10.0 * pixelSize) * 0.009167927656011385;

        gl_FragColor = mix(color, sum, mixAmount);
    }
)";

BlurNode::BlurNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        // TODO - would be nice if this were a resolution independent value, a multiplier maybe
        NodePort::create(BlurNodeInputPortKeyPixelSize, NodePortType::Vec2f, "Size"),
        NodePort::create(BlurNodeInputPortKeyMixAmount, NodePortType::Float, "Mix", 1.0f, 0.0f, 1.0f),
    };
    setInputPorts(inputPorts);
    // NB - output port "image" of type NodePortType::FBOImage is already present

    setupShader(sVertexShaderPassThrough, FragmentShaderBlur);
}

BlurNode::~BlurNode() {
}

void BlurNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>(NodeInputPortKeyImage);
    Vec2f pixelSize = getValueForInputPortKey<Vec2f>(BlurNodeInputPortKeyPixelSize);
    float mixAmount = getValueForInputPortKey<float>(BlurNodeInputPortKeyMixAmount);

    inputFBOImage->bindTexture(0); {
        mShader->bind(); {
            mShader->uniform(NodeInputPortKeyImage, 0);
            mShader->uniform(BlurNodeInputPortKeyPixelSize, pixelSize);
            mShader->uniform(BlurNodeInputPortKeyMixAmount, mixAmount);
            gl::drawSolidRect(outputFBOImage->getFBO().getBounds());
        } mShader->unbind();
    } inputFBOImage->unbindTexture();
}
