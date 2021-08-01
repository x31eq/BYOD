#pragma once

#include "../../utility/DCBlocker.h"
#include "TubeScreamerWDF.h"
#include "processors/BaseProcessor.h"

class TubeScreamer : public BaseProcessor
{
public:
    TubeScreamer (UndoManager* um = nullptr);

    ProcessorType getProcessorType() const override { return Drive; }
    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void prepare (double sampleRate, int samplesPerBlock) override;
    void processAudio (AudioBuffer<float>& buffer) override;

private:
    std::atomic<float>* gainParam = nullptr;
    std::atomic<float>* diodeTypeParam = nullptr;
    std::atomic<float>* nDiodesParam = nullptr;

    std::unique_ptr<TubeScreamerWDF> wdf[2];
    DCBlocker dcBlocker;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TubeScreamer)
};