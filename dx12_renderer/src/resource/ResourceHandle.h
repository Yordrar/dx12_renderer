#pragma once

#include <d3d12.h>

class ResourceHandle
{
public:
    ResourceHandle( UINT slot = 0 );

    UINT getSlot() const { return m_slot; }

private:
    UINT m_slot;
};

