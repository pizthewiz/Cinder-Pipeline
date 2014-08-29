//
//  ISFNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 26 Aug 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "ISFNode.h"

#include "cinder/Json.h"
#include <regex>

// 💀🚧⚠️ NOT YET FUNCTIONAL!

using namespace ci;
using namespace Cinder::Pipeline;

const std::string DictKeyDescription = "DESCRIPTION";
const std::string DictKeyInputs = "INPUTS";
const std::string DictKeyInputName = "NAME";
const std::string DictKeyInputType = "TYPE";
const std::string DictKeyInputTypeImage = "image";
const std::string DictKeyInputTypeBool = "bool";
const std::string DictKeyInputTypeFloat = "float";
const std::string DictKeyInputTypePoint2D = "point2D";
const std::string DictKeyInputTypeColor = "color";
const std::string DictKeyInputTypeLong = "long";
const std::string DictKeyInputTypeEvent = "event";
const std::string DictKeyInputValueDefault = "DEFAULT";
const std::string DictKeyInputValueMinimum = "MIN";
const std::string DictKeyInputValueMaximum = "MAX";
//const std::string DictKeyInputValueIdentity = "IDENTITY";
const std::string DictKeyInputLabel = "LABEL";
const std::string DictKeyInputValues = "VALUES";
const std::string DictKeyInputLabels = "LABELS";

const std::string FragmentShaderTest = R"(
    /*{
        "DESCRIPTION": "demonstrates all the input types",
        "CREDIT": "by no one",
        "CATEGORIES": [
            "TEST-GLSL FX"
        ],
        "INPUTS": [
            {
                "NAME": "inputImage",
                "TYPE": "image"
            },
            {
                "NAME": "flashToggle",
                "TYPE": "bool",
                "DEFAULT": 1.0
            },
            {
                "NAME": "level",
                "TYPE": "float",
                "DEFAULT": 0.5,
                "MIN": 0.0,
                "MAX": 1.0
            },
            {
                "NAME": "location",
                "TYPE": "point2D",
                "DEFAULT": [
                    0,
                    0
                ]
            },
            {
                "NAME": "lowColor",
                "TYPE": "color",
                "DEFAULT": [
                    0.0,
                    0.0,
                    1.0,
                    1.0
                ]
            },
            {
                "NAME": "longInputIsPopUpButton",
                "VALUES": [
                    0,
                    1,
                    2
                ],
                "LABELS": [
                    "red",
                    "green",
                    "blue"
                ],
                "DEFAULT": 1,
                "TYPE": "long"
            },
            {
                "NAME": "flashEvent",
                "TYPE": "event"
            }
        ]
    }*/

    void main() {
        vec4 srcPixel = IMG_THIS_PIXEL(inputImage);
        float luma = (srcPixel.r+srcPixel.g+srcPixel.b)/3.0;
        vec4 dstPixel = (luma>level) ? srcPixel : vec4(0,0,0,1);
        gl_FragColor = dstPixel;
    }
)";

ISFNode::ISFNode() {
    parseShader("", FragmentShaderTest);
}

ISFNode::~ISFNode() {
}

void ISFNode::render(const FBOImageRef& outputFBOImage) {
}

#pragma mark -

void ISFNode::parseShader(const DataSourceRef& vertexShader, const DataSourceRef& fragmentShader) {
    std::string vert = vertexShader ? loadString(vertexShader) : "TODO";
    std::string frag = loadString(fragmentShader);
    parseShader(vert, frag);
}

void ISFNode::parseShader(const std::string& vertexShader, const std::string& fragmentShader) {
    // naïve extraction of JSON dict, assumed to be in the first C++ comment block
    std::regex re(".*/\\*(.*)\\*/\\r?\\n?(.*)", std::regex::extended);
    std::smatch match;
    std::regex_search(fragmentShader, match, re);
    if (match.empty()) {
        // TODO - throw?
        return;
    }

    JsonTree data;
    try {
        data = JsonTree(match[1]);
    } catch (...) {
        // TODO - throw?
        return;
    }

//    if (data.hasChild("DESCRIPTION")) {
//        ci::app::console() << "description: " << data.getValueForKey("DESCRIPTION") << std::endl;
//    }
//
//    if (data.hasChild("CREDIT")) {
//        ci::app::console() << "credit: " << data.getValueForKey("CREDIT") << std::endl;
//    }
//
//    if (data.hasChild("CATEGORIES")) {
//        JsonTree categoryData = data.getChild("CATEGORIES");
//        for (size_t idx = 0; idx < categoryData.getChildren().size(); idx++) {
//            ci::app::console() << "category: " << categoryData.getValueAtIndex(idx) << std::endl;
//        }
//    }

    // inputs
    if (!data.hasChild(DictKeyInputs)) {
        // TODO - throw?
        return;
    }
    for (auto input : data.getChild(DictKeyInputs).getChildren()) {
        if (!input.hasChild(DictKeyInputName) || !input.hasChild(DictKeyInputType)) {
            // TODO - throw?
            return;
        }

        std::string name = input.getValueForKey(DictKeyInputName);
        std::string type = input.getValueForKey(DictKeyInputType);
        ci::app::console() << "input: '" << name << "' " << type << std::endl;

        if (type == DictKeyInputTypeImage) {
            // TODO - ???
        } else if (type == DictKeyInputTypeBool) {
            // default
            if (input.hasChild(DictKeyInputValueDefault)) {
                bool value = input.getValueForKey<bool>(DictKeyInputValueDefault);
                ci::app::console() << "default: " << value << std::endl;
            }
        } else if (type == DictKeyInputTypeFloat) {
            // default
            if (input.hasChild(DictKeyInputValueDefault)) {
                float value = input.getValueForKey<float>(DictKeyInputValueDefault);
                ci::app::console() << "default: " << value << std::endl;
            }
            // min
            if (input.hasChild(DictKeyInputValueMinimum)) {
                float value = input.getValueForKey<float>(DictKeyInputValueMinimum);
                ci::app::console() << "min: " << value << std::endl;
            }
            // max
            if (input.hasChild(DictKeyInputValueMaximum)) {
                float value = input.getValueForKey<float>(DictKeyInputValueMaximum);
                ci::app::console() << "max: " << value << std::endl;
            }
        } else if (type == DictKeyInputTypePoint2D) {
            // default
            if (input.hasChild(DictKeyInputValueDefault)) {
                JsonTree pointData = input.getChild(DictKeyInputValueDefault);
                if (pointData.getNumChildren() != 2) {
                    // TODO - ?
                    continue;
                }
                Vec2f value = Vec2f(pointData.getValueAtIndex<float>(0), pointData.getValueAtIndex<float>(1));
                ci::app::console() << "default: " << value << std::endl;
            }
            // min
            if (input.hasChild(DictKeyInputValueMinimum)) {
                JsonTree pointData = input.getChild(DictKeyInputValueMinimum);
                if (pointData.getNumChildren() != 2) {
                    // TODO - ?
                    continue;
                }
                Vec2f value = Vec2f(pointData.getValueAtIndex<float>(0), pointData.getValueAtIndex<float>(1));
                ci::app::console() << "min: " << value << std::endl;
            }
            // max
            if (input.hasChild(DictKeyInputValueMaximum)) {
                JsonTree pointData = input.getChild(DictKeyInputValueMaximum);
                if (pointData.getNumChildren() != 2) {
                    // TODO - ?
                    continue;
                }
                Vec2f value = Vec2f(pointData.getValueAtIndex<float>(0), pointData.getValueAtIndex<float>(1));
                ci::app::console() << "max: " << value << std::endl;
            }
        } else if (type == DictKeyInputTypeColor) {
            // default
            if (input.hasChild(DictKeyInputValueDefault)) {
                JsonTree pointData = input.getChild(DictKeyInputValueDefault);
                if (pointData.getNumChildren() != 4) {
                    // TODO - ?
                    continue;
                }
                ColorAf value = ColorAf(pointData.getValueAtIndex<float>(0), pointData.getValueAtIndex<float>(1), pointData.getValueAtIndex<float>(2), pointData.getValueAtIndex<float>(3));
                ci::app::console() << "default: " << value << std::endl;
            }
            // min
            if (input.hasChild(DictKeyInputValueMinimum)) {
                JsonTree pointData = input.getChild(DictKeyInputValueMinimum);
                if (pointData.getNumChildren() != 4) {
                    // TODO - ?
                    continue;
                }
                ColorAf value = ColorAf(pointData.getValueAtIndex<float>(0), pointData.getValueAtIndex<float>(1), pointData.getValueAtIndex<float>(2), pointData.getValueAtIndex<float>(3));
                ci::app::console() << "min: " << value << std::endl;
            }
            // max
            if (input.hasChild(DictKeyInputValueMaximum)) {
                JsonTree pointData = input.getChild(DictKeyInputValueMaximum);
                if (pointData.getNumChildren() != 4) {
                    // TODO - ?
                    continue;
                }
                ColorAf value = ColorAf(pointData.getValueAtIndex<float>(0), pointData.getValueAtIndex<float>(1), pointData.getValueAtIndex<float>(2), pointData.getValueAtIndex<float>(3));
                ci::app::console() << "max: " << value << std::endl;
            }
        } else if (type == DictKeyInputTypeLong) {
            // values
            if (!input.hasChild(DictKeyInputValues)) {
                // TODO - ?
                continue;
            }
            std::vector<long> values;
            JsonTree valuesData = input.getChild(DictKeyInputValues);
            for (size_t idx = 0; idx < valuesData.getChildren().size(); idx++) {
                values.push_back(valuesData.getValueAtIndex<long>(idx));
                ci::app::console() << "value: " << values.back() << std::endl;
            }
            // labels
            if (input.hasChild(DictKeyInputLabels)) {
                JsonTree labelsData = input.getChild(DictKeyInputLabels);
                if (labelsData.getNumChildren() != values.size()) {
                    // TODO - ?
                    continue;
                }
                std::vector<std::string> labels;
                for (size_t idx = 0; idx < labelsData.getChildren().size(); idx++) {
                    labels.push_back(labelsData.getValueAtIndex(idx));
                    ci::app::console() << "label: " << labels.back() << std::endl;
                }
            }
            // default
            if (input.hasChild(DictKeyInputValueDefault)) {
                long value = input.getValueForKey<float>(DictKeyInputValueDefault);
                // TODO - check that it exists in values
                ci::app::console() << "default: " << value << std::endl;
            }
        } else if (type == DictKeyInputTypeEvent) {
            // TODO - ???
        }
    }

    // imported resources - BAIL
    if (data.hasChild("IMPORTED")) {
        // TODO - throw?
        return;
    }

    // multi-pass rendering - BAIL
    if (data.hasChild("PASSES")) {
        // TODO - throw?
        return;
    }

    // persistent buffers - BAIL
    if (data.hasChild("PERSISTENT_BUFFERS")) {
        // TODO - throw?
        return;
    }
}
