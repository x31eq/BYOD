#pragma once

#include "../BaseProcessor.h"
#include "../ParameterHelpers.h"

class DCBias : public BaseProcessor
{
public:
    explicit DCBias (UndoManager* um = nullptr) : BaseProcessor ("DC Bias", createParameterLayout(), um)
    {
        chowdsp::ParamUtils::loadParameterPointer (biasParam, vts, "bias");

        uiOptions.backgroundColour = Colours::slategrey;
        uiOptions.powerColour = Colours::yellow;
        uiOptions.info.description = "Adds a constant DC bias to the signal.";
        uiOptions.info.authors = StringArray { "Jatin Chowdhury" };
    }

    ProcessorType getProcessorType() const override { return Utility; }
    static ParamLayout createParameterLayout()
    {
        using namespace ParameterHelpers;
        auto params = createBaseParams();
        emplace_param<chowdsp::FloatParameter> (params, "bias", "Bias", NormalisableRange { -0.25f, 0.25f }, 0.0f, &floatValToString, &stringToFloatVal);

        return { params.begin(), params.end() };
    }

    void prepare (double sampleRate, int samplesPerBlock) override
    {
        dsp::ProcessSpec spec { sampleRate, (uint32) samplesPerBlock, 2 };
        bias.prepare (spec);
        bias.setRampDurationSeconds (0.1);
    }

    void processAudio (AudioBuffer<float>& buffer) override
    {
        bias.setBias (*biasParam);

        dsp::AudioBlock<float> block { buffer };
        dsp::ProcessContextReplacing<float> context { block };
        bias.process (context);
    }

private:
    chowdsp::FloatParameter* biasParam = nullptr;
    dsp::Bias<float> bias;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DCBias)
};
