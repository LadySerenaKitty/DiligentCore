/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#pragma once

/// \file
/// Definition of the Diligent::IRasterizationRateMapMtl interface

#include "../../GraphicsEngine/interface/DeviceObject.h"
#include "../../GraphicsEngine/interface/Buffer.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

// {89148E0E-1300-4FF2-BEA4-F1127ED24CF9}
static const INTERFACE_ID IID_RasterizationRateMapMtl =
    {0x89148e0e, 0x1300, 0x4ff2, {0xbe, 0xa4, 0xf1, 0x12, 0x7e, 0xd2, 0x4c, 0xf9}};

#define DILIGENT_INTERFACE_NAME IRasterizationRateMapMtl
#include "../../../Primitives/interface/DefineInterfaceHelperMacros.h"

#define IRasterizationRateMapMtlInclusiveMethods \
    IDeviceObjectInclusiveMethods;               \
    IRasterizationRateMapMtlMethods RasterizationRateMapMtl

// clang-format off


/// Rasterization rate map description
struct RasterizationRateMapDesc DILIGENT_DERIVE(DeviceObjectAttribs)
    
    /// Width of the final render target
    Uint32 ScreenWidth  DEFAULT_INITIALIZER(0);

    /// Height of the final render target
    Uint32 ScreenHeight DEFAULT_INITIALIZER(0);
    
    /// Number of layers
    Uint32 LayerCount   DEFAULT_INITIALIZER(0);
};
typedef struct RasterizationRateMapDesc RasterizationRateMapDesc;


/// Rasterization rate map layer description
struct RasterizationRateLayerDesc
{
    /// Number of elements in pHorizontal
    Uint32       HorizontalCount DEFAULT_INITIALIZER(0);

    /// Number of elements in pVertical
    Uint32       VerticalCount   DEFAULT_INITIALIZER(0);

    /// Shading rates in row
    const float* pHorizontal     DEFAULT_INITIALIZER(nullptr);

    /// Shading rates in column
    const float* pVertical       DEFAULT_INITIALIZER(nullptr);
};
typedef struct RasterizationRateLayerDesc RasterizationRateLayerDesc;


/// Rasterization rate map create info
struct RasterizationRateMapCreateInfo
{
    /// Rasterization rate map description
    RasterizationRateMapDesc          Desc;
    
    /// Array of rasterization rate map layer descriptions
    const RasterizationRateLayerDesc* pLayers    DEFAULT_INITIALIZER(nullptr);
};
typedef struct RasterizationRateMapCreateInfo RasterizationRateMapCreateInfo;


/// Exposes Metal-specific functionality of a rasterization rate map object.
DILIGENT_BEGIN_INTERFACE(IRasterizationRateMapMtl, IDeviceObject)
{
#if DILIGENT_CPP_INTERFACE
    /// Returns the buffer description used to create the object
    virtual const RasterizationRateMapDesc& METHOD(GetDesc)() const override = 0;
#endif

    /// Returns a pointer to a Metal rasterization rate map object.
    VIRTUAL id<MTLRasterizationRateMap> METHOD(GetMtlResource)(THIS) CONST PURE;

    /// Returns size of the attachments for specified layer.
    VIRTUAL void METHOD(GetPhysicalSizeForLayer)(THIS_
                                                 Uint32     LayerIndex,
                                                 Uint32 REF PhysicalWidth,
                                                 Uint32 REF PhysicalHeight) CONST PURE;

    /// The granularity, in physical pixels, at which the rasterization rate varies.
    /// For better performance tile size should be multiple of physical granularity.
    VIRTUAL void METHOD(GetPhysicalGranularity)(THIS_
                                                Uint32 REF XGranularity,
                                                Uint32 REF YGranularity) CONST PURE;
    
    /// Converts a point in logical viewport coordinates to the corresponding physical coordinates in a render layer.
    VIRTUAL void METHOD(MapScreenToPhysicalCoordinates)(THIS_
                                                        Uint32    LayerIndex,
                                                        float     ScreenCoordX,
                                                        float     ScreenCoordY,
                                                        float REF PhysicalCoordX,
                                                        float REF PhysicalCoordY) CONST PURE;
    
    /// Converts a point in physical coordinates inside a layer to its corresponding logical viewport coordinates.
    VIRTUAL void METHOD(MapPhysicalToScreenCoordinates)(THIS_
                                                        Uint32    LayerIndex,
                                                        float     PhysicalCoordX,
                                                        float     PhysicalCoordY,
                                                        float REF ScreenCoordX,
                                                        float REF ScreenCoordY) CONST PURE;

    /// Returns requirements for parameter buffer which used for resolve pass.
    VIRTUAL void METHOD(GetParameterBufferSizeAndAlign)(THIS_
                                                        Uint32 REF Size,
                                                        Uint32 REF Align) CONST PURE;
    
    /// Copy rasterization rate map parameters to the buffer.
    
    /// \param [in] pDstBuffer - Parameter buffer which will be used for resolve pass.
    ///                          Buffer must be created with USAGE_UNIFIED.
    /// \param [in] Offset     - Offset in buffer, must be multiple of align which returned by GetParameterBufferSizeAndAlign().
    VIRTUAL void METHOD(CopyParameterDataToBuffer)(THIS_
                                                   IBuffer* pDstBuffer,
                                                   Uint32   Offset) CONST PURE;
};
DILIGENT_END_INTERFACE

#include "../../../Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

#    define IRasterizationRateMapMtl_GetDesc(This) (const struct RasterizationRateMapDesc*)IDeviceObject_GetDesc(This)

#    define IRasterizationRateMapMtl_GetMtlResource(This)                       CALL_IFACE_METHOD(RasterizationRateMapMtl, GetMtlResource,                 This)
#    define IRasterizationRateMapMtl_GetPhysicalSizeForLayer(This, ...)         CALL_IFACE_METHOD(RasterizationRateMapMtl, GetPhysicalSizeForLayer,        This, __VA_ARGS__)
#    define IRasterizationRateMapMtl_GetPhysicalGranularity(This, ...)          CALL_IFACE_METHOD(RasterizationRateMapMtl, GetPhysicalGranularity,         This, __VA_ARGS__)
#    define IRasterizationRateMapMtl_MapScreenToPhysicalCoordinates(This, ...)  CALL_IFACE_METHOD(RasterizationRateMapMtl, MapScreenToPhysicalCoordinates, This, __VA_ARGS__)
#    define IRasterizationRateMapMtl_MapPhysicalToScreenCoordinates(This, ...)  CALL_IFACE_METHOD(RasterizationRateMapMtl, MapPhysicalToScreenCoordinates, This, __VA_ARGS__)
#    define IRasterizationRateMapMtl_GetParameterBufferSizeAndAlign(This, ...)  CALL_IFACE_METHOD(RasterizationRateMapMtl, GetParameterBufferSizeAndAlign, This, __VA_ARGS__)
#    define IRasterizationRateMapMtl_CopyParameterDataToBuffer(This, ...)       CALL_IFACE_METHOD(RasterizationRateMapMtl, CopyParameterDataToBuffer,      This, __VA_ARGS__)

#endif

DILIGENT_END_NAMESPACE // namespace Diligent
