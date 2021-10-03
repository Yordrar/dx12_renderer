#pragma once

#include <d3d12.h>

class ResourceHandle
{
public:
    ResourceHandle( UINT slot );

    UINT getSlot() const { return m_slot; }

private:
    UINT m_slot;
};

