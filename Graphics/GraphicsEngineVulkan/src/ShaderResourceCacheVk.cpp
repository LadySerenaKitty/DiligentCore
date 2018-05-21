/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#include "pch.h"

#include "ShaderResourceCacheVk.h"
#include "DeviceContextVkImpl.h"
#include "BufferVkImpl.h"
#include "BufferViewVkImpl.h"
#include "TextureViewVkImpl.h"
#include "TextureVkImpl.h"

namespace Diligent
{
    void ShaderResourceCacheVk::InitializeSets(IMemoryAllocator &MemAllocator, Uint32 NumSets, Uint32 SetSizes[])
    {
        // Memory layout:
        //                                              ______________________________________________________________
        //  m_pMemory                                  |                 m_pResources, m_NumResources == m            |
        //  |                                          |                                                              |
        //  V                                          |                                                              V
        //  |  DescriptorSet[0]  |   ....    |  DescriptorSet[Ns-1]  |  Res[0]  |  ... |  Res[n-1]  |    ....     | Res[0]  |  ... |  Res[m-1]  |
        //            |                                                  A \
        //            |                                                  |  \
        //            |__________________________________________________|   \RefCntAutoPtr
        //                       m_pResources, m_NumResources == n            \_________     
        //                                                                    |  Object |
        //                                                                     --------- 
        //                                                                    
        //  Ns = m_NumSets

        VERIFY(m_pAllocator == nullptr && m_pMemory == nullptr, "Cache already initialized");
        m_pAllocator = &MemAllocator;
        m_NumSets = NumSets;
        m_TotalResources = 0;
        for(Uint32 t=0; t < NumSets; ++t)
            m_TotalResources += SetSizes[t];
        auto MemorySize = NumSets * sizeof(DescriptorSet) + m_TotalResources * sizeof(Resource);
        if(MemorySize > 0)
        {
            m_pMemory = ALLOCATE( *m_pAllocator, "Memory for shader resource cache data", MemorySize);
            auto *pSets = reinterpret_cast<DescriptorSet*>(m_pMemory);
            auto *pCurrResPtr = reinterpret_cast<Resource*>(pSets + m_NumSets);
            for (Uint32 t = 0; t < NumSets; ++t)
            {
                new(&GetDescriptorSet(t)) DescriptorSet(SetSizes[t], SetSizes[t] > 0 ? pCurrResPtr : nullptr);
                pCurrResPtr += SetSizes[t];
            }
            VERIFY_EXPR((char*)pCurrResPtr == (char*)m_pMemory + MemorySize);
        }
    }

    void ShaderResourceCacheVk::InitializeResources(Uint32 Set, Uint32 Offset, Uint32 ArraySize, SPIRVShaderResourceAttribs::ResourceType Type)
    {
        auto &DescrSet = GetDescriptorSet(Set);
        for (Uint32 res = 0; res < ArraySize; ++res)
            new(&DescrSet.GetResource(Offset + res)) Resource{Type};
    }

    ShaderResourceCacheVk::~ShaderResourceCacheVk()
    {
        if (m_pMemory)
        {
            auto *pResources = reinterpret_cast<Resource*>( reinterpret_cast<DescriptorSet*>(m_pMemory) + m_NumSets);
            for(Uint32 res=0; res < m_TotalResources; ++res)
                pResources[res].~Resource();
            for (Uint32 t = 0; t < m_NumSets; ++t)
                GetDescriptorSet(t).~DescriptorSet();

            m_pAllocator->Free(m_pMemory);
        }
    }

    template<bool VerifyOnly>
    void ShaderResourceCacheVk::TransitionResources(DeviceContextVkImpl *pCtxVkImpl)
    {
        auto *pResources = reinterpret_cast<Resource*>(reinterpret_cast<DescriptorSet*>(m_pMemory) + m_NumSets);
        for (Uint32 res = 0; res < m_TotalResources; ++res)
        {
            auto &Res = pResources[res];
            switch (Res.Type)
            {
                case SPIRVShaderResourceAttribs::ResourceType::UniformBuffer:
                case SPIRVShaderResourceAttribs::ResourceType::StorageBuffer:
                {
                    auto *pBufferVk = Res.pObject.RawPtr<BufferVkImpl>();
                    VkAccessFlags RequiredAccessFlags = 
                        Res.Type == SPIRVShaderResourceAttribs::ResourceType::UniformBuffer ? 
                            VK_ACCESS_UNIFORM_READ_BIT : 
                            (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
                    if(pBufferVk->GetAccessFlags() != RequiredAccessFlags)
                    {
                        if(VerifyOnly)
                            LOG_ERROR_MESSAGE("Buffer \"", pBufferVk->GetDesc().Name, "\" is not in correct state. Did you forget to call TransitionShaderResources() or specify COMMIT_SHADER_RESOURCES_FLAG_TRANSITION_RESOURCES flag in a call to CommitShaderResources()?");
                        else
                            pCtxVkImpl->BufferMemoryBarrier(*pBufferVk, RequiredAccessFlags);
                    }
                }
                break;

                case SPIRVShaderResourceAttribs::ResourceType::UniformTexelBuffer:
                case SPIRVShaderResourceAttribs::ResourceType::StorageTexelBuffer:
                {
                    auto *pBuffViewVk = Res.pObject.RawPtr<BufferViewVkImpl>();
                    auto *pBufferVk = ValidatedCast<BufferVkImpl>(pBuffViewVk->GetBuffer());
                    VkAccessFlags RequiredAccessFlags =
                        Res.Type == SPIRVShaderResourceAttribs::ResourceType::UniformTexelBuffer ?
                        VK_ACCESS_SHADER_READ_BIT :
                        (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
                    if (pBufferVk->GetAccessFlags() != RequiredAccessFlags)
                    {
                        if (VerifyOnly)
                            LOG_ERROR_MESSAGE("Buffer \"", pBufferVk->GetDesc().Name, "\" is not in correct state. Did you forget to call TransitionShaderResources() or specify COMMIT_SHADER_RESOURCES_FLAG_TRANSITION_RESOURCES flag in a call to CommitShaderResources()?");
                        else
                            pCtxVkImpl->BufferMemoryBarrier(*pBufferVk, RequiredAccessFlags);
                    }
                }
                break;

                case SPIRVShaderResourceAttribs::ResourceType::SeparateImage:
                case SPIRVShaderResourceAttribs::ResourceType::SampledImage:
                case SPIRVShaderResourceAttribs::ResourceType::StorageImage:
                {
                    auto *pTextureViewVk = Res.pObject.RawPtr<TextureViewVkImpl>();
                    auto *pTextureVk = ValidatedCast<TextureVkImpl>(pTextureViewVk->GetTexture());
                    VkImageLayout RequiredLayout = 
                        Res.Type == SPIRVShaderResourceAttribs::ResourceType::StorageImage ? 
                            VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    if(pTextureVk->GetLayout() != RequiredLayout)
                    {
                        if (VerifyOnly)
                            LOG_ERROR_MESSAGE("Texture \"", pTextureVk->GetDesc().Name, "\" is not in correct state. Did you forget to call TransitionShaderResources() or specify COMMIT_SHADER_RESOURCES_FLAG_TRANSITION_RESOURCES flag in a call to CommitShaderResources()?");
                        else
                            pCtxVkImpl->TransitionImageLayout(*pTextureVk, RequiredLayout);
                    }
                }
                break;

                case SPIRVShaderResourceAttribs::ResourceType::AtomicCounter:
                {
                    // Nothing to do with atomic counters
                }
                break;

                case SPIRVShaderResourceAttribs::ResourceType::SeparateSampler:
                {
                    // Nothing to do with samplers
                }
                break;

                default: UNEXPECTED("Unexpected resource type");
            }
        }
    }

    template void ShaderResourceCacheVk::TransitionResources<false>(DeviceContextVkImpl *pCtxVkImpl);
    template void ShaderResourceCacheVk::TransitionResources<true>(DeviceContextVkImpl *pCtxVkImpl);
}
