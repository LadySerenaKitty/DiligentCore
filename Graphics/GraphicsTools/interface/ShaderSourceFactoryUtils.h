/*
 *  Copyright 2023 Diligent Graphics LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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

#pragma once

/// \file
/// Defines graphics engine utilities

#include "../../GraphicsEngine/interface/Shader.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

#include "../../Primitives/interface/DefineGlobalFuncHelperMacros.h"

/// Shader source file substitute info.
struct ShaderSourceFileSubstitueInfo
{
    /// Source file name.
    const Char* Name DEFAULT_INITIALIZER(nullptr);

    /// Substitute file name.
    const Char* Substitute DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    constexpr ShaderSourceFileSubstitueInfo() noexcept
    {}

    constexpr ShaderSourceFileSubstitueInfo(const Char* _Name, const Char* _Substitute) noexcept :
        Name{_Name},
        Substitute{_Substitute}
    {}
#endif
};

/// Compound shader source factory create info.
struct CompoundShaderSourceFactoryCreateInfo
{
    /// An array of shader source input stream factories.
    IShaderSourceInputStreamFactory** ppFactories DEFAULT_INITIALIZER(nullptr);

    /// The number of factories in ppFactories array.
    Uint32 NumFactories DEFAULT_INITIALIZER(0);

    /// An array of shader source file substitutes.
    ShaderSourceFileSubstitueInfo* pFileSubstitutes DEFAULT_INITIALIZER(nullptr);

    /// The number of file substitutes in pFileSubstitutes array.
    Uint32 NumFileSubstitutes DEFAULT_INITIALIZER(0);

#if DILIGENT_CPP_INTERFACE
    constexpr CompoundShaderSourceFactoryCreateInfo() noexcept
    {}

    constexpr CompoundShaderSourceFactoryCreateInfo(IShaderSourceInputStreamFactory** _ppFactories,
                                                    Uint32                            _NumFactories,
                                                    ShaderSourceFileSubstitueInfo*    _pFileSubstitutes   = nullptr,
                                                    Uint32                            _NumFileSubstitutes = 0) noexcept :
        ppFactories{_ppFactories},
        NumFactories{_NumFactories},
        pFileSubstitutes{_pFileSubstitutes},
        NumFileSubstitutes{_NumFileSubstitutes}
    {}
#endif
};
typedef struct CompoundShaderSourceFactoryCreateInfo CompoundShaderSourceFactoryCreateInfo;
// clang-format on

/// Creates a compound shader source factory.
///
/// \param [in]  CreateInfo - Compound shader source factory create info, see Diligent::CompoundShaderSourceFactoryCreateInfo.
/// \param [out] ppFactory  - Address of the memory location where the pointer to the created factory will be written.
///
/// \remarks    Compound shader source stream factory is a wrapper around multiple shader source stream factories.
///             It is used to combine multiple shader source stream factories into a single one. When a source file
///             is requested, the factory will iterate over all factories in the array and return the first one that
///             returns a non-null stream.
///
///             The factory also allows substituting source file names. This is useful when the same shader source
///             is used for multiple shaders, but some of them require a modified version of the source.
void DILIGENT_GLOBAL_FUNCTION(CreateCompoundShaderSourceFactory)(const CompoundShaderSourceFactoryCreateInfo REF CreateInfo, IShaderSourceInputStreamFactory** ppFactory);

#include "../../Primitives/interface/UndefGlobalFuncHelperMacros.h"

DILIGENT_END_NAMESPACE // namespace Diligent
