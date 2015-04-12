//
//  GaussianBlurNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 22 Apr 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#include "GaussianBlurNode.h"
#include <numeric>

using namespace ci;
using namespace Cinder::Pipeline;

static const std::string GaussianBlurNodeInputPortKeyDimension = "dimension";
static const float GaussianBlurNodeRadiusDefault = 2.0f; // NB: actually sigma like CIGaussianBlur's inputRadius

// Based on Brad Larson's GPUImage work at https://github.com/BradLarson/GPUImage/blob/master/framework/Source/GPUImageGaussianBlurFilter.h
//  https://github.com/BradLarson/GPUImage/blob/master/framework/Source/GPUImageGaussianBlurFilter.m
// and Daniel Rákos' work at http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
const std::string FragmentShaderGaussianBlur = R"(
    #version 150
    uniform sampler2D image;
    uniform vec2 direction;
    uniform float dimension;
    uniform float mixAmount;

    in vec2 vTexCoord0;

    out vec4 oFragColor;

    #ifndef DEFAULT_OVERRIDE
        // defualt to blur radius 6, sigma 2
        #define SAMPLE_COUNT 7
        #define OFFSET_VALUES 0, 1, 2, 3, 4, 5, 6
        #define WEIGHT_VALUES 0.199675635, 0.17621313, 0.121109389, 0.0648251921, 0.027023159, 0.00877313502, 0.00221819594
    #endif

    const float offset[SAMPLE_COUNT] = float[](OFFSET_VALUES);
    const float weight[SAMPLE_COUNT] = float[](WEIGHT_VALUES);

    void main() {
        vec4 color = texture(image, vTexCoord0);

        vec4 sum = texture(image, vTexCoord0) * weight[0];
        for (int idx = 1; idx < SAMPLE_COUNT; idx++) {
            sum += texture(image, vTexCoord0 + direction * offset[idx] / dimension) * weight[idx];
            sum += texture(image, vTexCoord0 - direction * offset[idx] / dimension) * weight[idx];
        }

        oFragColor = mix(color, sum, mixAmount);
    }
)";

GaussianBlurNode::GaussianBlurNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(GaussianBlurNodeInputPortKeyDirection, NodePortType::Index, "Direction", static_cast<int>(GaussianBlurNode::BlurDirection::Vertical), {static_cast<int>(GaussianBlurNode::BlurDirection::Vertical), static_cast<int>(GaussianBlurNode::BlurDirection::Horizontal)}, {"Vertical", "Horizontal"}),
        NodePort::create(GaussianBlurNodeInputPortKeyRadius, NodePortType::Float, "Radius", GaussianBlurNodeRadiusDefault, 0.0f, 8.0f),
        NodePort::create(GaussianBlurNodeInputPortKeyMixAmount, NodePortType::Float, "Mix", 1.0f, 0.0f, 1.0f),
    };
    setInputPorts(inputPorts);
    // NB - output port "image" of type NodePortType::FBOImage is already present

    // setup shader when radius changes
    connectValueForInputPortKeyChangedHandler(GaussianBlurNodeInputPortKeyRadius, [&](const std::string key) {
        float radius = roundf(getValueForInputPortKey<float>(key));
        setupShaderForRadius(radius);
    });

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
    float mixAmount = getValueForInputPortKey<float>(GaussianBlurNodeInputPortKeyMixAmount);

    gl::ScopedTextureBind texture(inputTexture, 0);
    gl::ScopedGlslProg shader(mShader);

    mShader->uniform(NodeInputPortKeyImage, 0);
    mShader->uniform(GaussianBlurNodeInputPortKeyDirection, direction);
    mShader->uniform(GaussianBlurNodeInputPortKeyDimension, dimension);
    mShader->uniform(GaussianBlurNodeInputPortKeyMixAmount, mixAmount);
    gl::drawSolidRect(outputFBOImage->getFBO()->getBounds());
}

void GaussianBlurNode::setupShaderForRadius(const float radius) {
    float blurRadiusInPixels = radius;
    int calculatedSampleRadius = 0;
    if (blurRadiusInPixels > 0) {
        float minimumWeightToFindEdgeOfSamplingArea = 1.0f / 256.0f;
        calculatedSampleRadius = floor(sqrt(-2.0f * pow(blurRadiusInPixels, 2.0f) * log(minimumWeightToFindEdgeOfSamplingArea * sqrt(2.0f * M_PI * pow(blurRadiusInPixels, 2.0f))) ));
        calculatedSampleRadius += calculatedSampleRadius % 2;
    }

    int blurRadius = calculatedSampleRadius;
    float sigma = blurRadiusInPixels;

    std::vector<float> offsets(blurRadius + 1);
    for (size_t idx = 0; idx < offsets.size(); idx++) offsets[idx] = idx;

    // calculate weights
    std::vector<float> weights(blurRadius + 1);
    for (size_t idx = 0; idx < weights.size(); idx++) {
        float weight = (1.0f / sqrt(2.0f * M_PI * pow(sigma, 2.0f))) * exp(-pow(idx, 2.0f) / (2.0f * pow(sigma, 2.0f)));
        if (weight != weight) weight = 1.0f; // NaN check, occurs when blurRadius is 0
        weights[idx] = weight;
    }

    // normalize to prevent clamping
    float sumOfWeights = 2.0f * std::accumulate(weights.begin(), weights.end(), 0.0f);
    sumOfWeights -= weights[0];
    std::transform(weights.begin(), weights.end(), weights.begin(), [sumOfWeights](const float& val) { return val / sumOfWeights; });

    std::string offsetValuesString = toString(offsets[0]);
    for (size_t idx = 1; idx < offsets.size(); idx++) {
        offsetValuesString += ", " + toString(offsets[idx]);
    }

    std::string weightValuesString = toString(weights[0]);
    for (size_t idx = 1; idx < weights.size(); idx++) {
        weightValuesString += ", " + toString(weights[idx]);
    }

    auto format = gl::GlslProg::Format().vertex(sVertexShaderPassThrough).fragment(FragmentShaderGaussianBlur);
    format.define("DEFAULT_OVERRIDE");
    format.define("SAMPLE_COUNT", toString(weights.size()));
    format.define("OFFSET_VALUES", offsetValuesString);
    format.define("WEIGHT_VALUES", weightValuesString);
    try {
        mShader = gl::GlslProg::create(format);
    } catch (gl::GlslProgExc e) {
        throw e;
    }
}
