#pragma once

#include <Manager.h>

class Profiler : public Manager<Profiler>
{
	friend class Manager<Profiler>;
public:
	~Profiler() = default;

	static constexpr uint64_t sc_numQueriesPerFrame = 1024;

	uint64_t allocateQueryIndex();
	void startQuery( ComPtr<ID3D12GraphicsCommandList> commandList, uint64_t index );
	void endQuery( ComPtr<ID3D12GraphicsCommandList> commandList, uint64_t index );
	double getResolvedQuery( uint64_t index );

private:
	Profiler();
	uint64_t getInHeapQueryIndexForCurrentFrameFromAllocatedIndex( uint64_t allocatedIndex );
	uint64_t getInHeapQueryIndexForPreviousFrameFromAllocatedIndex( uint64_t allocatedIndex );

	ComPtr<ID3D12QueryHeap> m_queryHeap;
	ComPtr<ID3D12Resource> m_resolvedQueriesResource;
	uint64_t m_numAllocatedQueryIndices;
};
