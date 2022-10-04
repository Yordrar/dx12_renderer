#include "RenderPass.h"

RenderPass::RenderPass( std::string name, std::initializer_list<Scene> scenes )
{
    m_scenes.reserve( m_scenes.size() );
    for ( Scene scene : scenes )
    {
        m_scenes.push_back( scene );
    }
}

RenderPass::~RenderPass()
{

}
