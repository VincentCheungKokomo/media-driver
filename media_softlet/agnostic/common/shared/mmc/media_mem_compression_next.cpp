/*
* Copyright (c) 2022, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

//!
//! \file     media_mem_compression_next.cpp
//! \brief    Defines the common interface for media memory compression
//! \details  The mmc is to handle mmc operations,
//!

#include "media_mem_compression_next.h"
#include "mos_interface.h"

MediaMemCompNext::MediaMemCompNext(PMOS_INTERFACE osInterface, std::shared_ptr<mhw::mi::Itf> miItf) :
    m_osInterface(osInterface),
    m_miItf(miItf),
    m_mmcEnabled(false),
    m_mmcFeatureId(__MOS_USER_FEATURE_KEY_MAX_ID),
    m_mmcInuseFeatureId(__MOS_USER_FEATURE_KEY_MAX_ID)
{
    MEDIA_FEATURE_TABLE *skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    m_isCompSurfAllocable = MosInterface::IsCompressibelSurfaceSupported(skuTable);
    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
}

MOS_STATUS MediaMemCompNext::InitMmcEnabled()
{
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if (MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrE2ECompression))
    {
        m_mmcEnabled = IsMmcFeatureEnabled();
        UpdateMmcInUseFeature();
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaMemCompNext::DecompressResource(PMOS_RESOURCE resource)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    if (resource)
    {
        status = m_osInterface->pfnDecompResource(m_osInterface, resource);
    }

    return status;
}


bool MediaMemCompNext::IsMmcFeatureEnabled()
{
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

    userFeatureData.i32Data = m_bComponentMmcEnabled;

    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        m_mmcFeatureId,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_mmcEnabled = (userFeatureData.i32Data) ? true : false;

    if (NullHW::IsEnabled())
    {
        m_mmcEnabled = false;
    }

    return m_mmcEnabled;
}

// For VP, there is no such feature id, if need to add 1?
MOS_STATUS MediaMemCompNext::UpdateMmcInUseFeature()
{
    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
    MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
    userFeatureWriteData.Value.i32Data = m_mmcEnabled;
    userFeatureWriteData.ValueID = m_mmcInuseFeatureId;

    return MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);
}

bool MediaMemCompNext::IsMmcEnabled()
{
    return m_mmcEnabled;
}

void MediaMemCompNext::DisableMmc()
{
    m_mmcEnabled = false;
}

bool MediaMemCompNext::IsCompressibelSurfaceSupported()
{
    return m_isCompSurfAllocable;
}

MOS_STATUS MediaMemCompNext::SetSurfaceMmcMode(
    PMOS_SURFACE surface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if(m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &surface->OsResource, (PMOS_MEMCOMP_STATE)&surface->CompressionMode);
    else
        surface->CompressionMode = MOS_MMC_DISABLED;

    return status;
}

MOS_STATUS MediaMemCompNext::SetSurfaceMmcState(
        PMOS_SURFACE surface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if(m_mmcEnabled)
        status =  m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &surface->OsResource, &surface->MmcState);
    else
        surface->MmcState = MOS_MEMCOMP_DISABLED;

    return status;
}

MOS_STATUS MediaMemCompNext::GetSurfaceMmcState(
    PMOS_SURFACE surface,
    MOS_MEMCOMP_STATE *mmcState)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, mmcState);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if (m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &surface->OsResource, mmcState);
    else
        *mmcState = MOS_MEMCOMP_DISABLED;

    return status;
}

MOS_STATUS MediaMemCompNext::GetSurfaceMmcFormat(
    PMOS_SURFACE       surface,
    uint32_t *mmcFormat)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, mmcFormat);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if (m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, &surface->OsResource, mmcFormat);
    else
        *mmcFormat = 0;

    return status;
}

MOS_STATUS MediaMemCompNext::GetResourceMmcFormat(
    PMOS_RESOURCE resource,
    uint32_t    &mmcFormat)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, resource);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if (m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, resource, &mmcFormat);
    else
        mmcFormat = 0;

    return status;
}

MOS_STATUS MediaMemCompNext::SetSurfaceMmcFormat(
        PMOS_SURFACE surface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if(m_mmcEnabled)
        status =  m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, &surface->OsResource, &surface->CompressionFormat);
    else
        surface->CompressionFormat = 0;

    return status;
}

MOS_STATUS MediaMemCompNext::GetResourceMmcState(
    PMOS_RESOURCE resource,
    MOS_MEMCOMP_STATE &mmcMode)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, resource);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);
    //Need to Check its default value m_mmcEnable
    if (m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, resource, &mmcMode);
    else
        mmcMode = MOS_MEMCOMP_DISABLED;

    return status;
}
