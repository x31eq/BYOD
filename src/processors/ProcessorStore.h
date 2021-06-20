#pragma once

#include "BaseProcessor.h"

class ProcessorStore
{
public:
    ProcessorStore();

    void createProcList (PopupMenu& menu, int& menuID, ProcessorType type);

    std::function<void(BaseProcessor::Ptr)> addProcessorCallback = nullptr;

private:
    using StoreMap = std::map<String, std::function<BaseProcessor::Ptr(void)>>;
    static StoreMap store;
    
    std::unordered_map<String, ProcessorType> procTypeStore;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorStore)
};