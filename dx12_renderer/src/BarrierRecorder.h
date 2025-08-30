#pragma once

#include <resource/ResourceHandle.h>

class BarrierRecorder
{
public:
    BarrierRecorder() = default;
    ~BarrierRecorder() = default;

    void recordBarrierTransition(ResourceHandle handle, D3D12_RESOURCE_STATES newState);
    void recordBarrierUAV(ResourceHandle handle);

    void submitBarriers(ComPtr<ID3D12GraphicsCommandList> commandList);
private:
    std::array<D3D12_RESOURCE_BARRIER, 32> m_recordedBarriers;
    uint8_t m_numRecordedBarriers = 0;
};