#include <ShaderBase.h>
#include <ThinDrawer.h>

ShaderBase::ShaderBase(ThinDrawer* thinDrawer)
{
    this->thinDrawer = thinDrawer;
    this->logicalDevice = thinDrawer->logicalDevice;
}