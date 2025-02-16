/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     vp_user_feature_control.cpp
//! \brief    vp user feature control.
//! \details  vp user feature control.
//!
#include "vp_user_feature_control.h"
#include "vp_utils.h"

using namespace vp;

VpUserFeatureControl::VpUserFeatureControl(MOS_INTERFACE &osInterface, VpPlatformInterface *vpPlatformInterface, void *owner) :
    m_owner(owner), m_osInterface(&osInterface), m_vpPlatformInterface(vpPlatformInterface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    uint32_t compBypassMode = VPHAL_COMP_BYPASS_ENABLED;    // Vebox Comp Bypass is on by default
    auto skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);

    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    // Read user feature key to get the Composition Bypass mode
    if (skuTable && (!MEDIA_IS_SKU(skuTable, FtrVERing)))
    {
        m_ctrlValDefault.disableVeboxOutput = true;
        m_ctrlValDefault.disableSfc         = true;

        VP_PUBLIC_NORMALMESSAGE("No VeRing, disableVeboxOutput %d, disableSfc %d", (m_ctrlValDefault.disableVeboxOutput, m_ctrlValDefault.disableSfc));
    }
    else
    {
        status = ReadUserSetting(
            m_userSettingPtr,
            compBypassMode,
            __VPHAL_BYPASS_COMPOSITION,
            MediaUserSetting::Group::Sequence,
            compBypassMode,
            true);

        if (MOS_SUCCEEDED(status))
        {
            m_ctrlValDefault.disableVeboxOutput = VPHAL_COMP_BYPASS_DISABLED == compBypassMode;
        }
        else
        {
            // Default value
            m_ctrlValDefault.disableVeboxOutput = false;
        }

        VP_PUBLIC_NORMALMESSAGE("disableVeboxOutput %d", m_ctrlValDefault.disableVeboxOutput);

        if (skuTable && MEDIA_IS_SKU(skuTable, FtrSFCPipe))
        {
            // Read user feature key to Disable SFC
            bool disableSFC = false;
            status          = ReadUserSetting(
                m_userSettingPtr,
                disableSFC,
                __VPHAL_VEBOX_DISABLE_SFC,
                MediaUserSetting::Group::Sequence);

            if (MOS_SUCCEEDED(status))
            {
                m_ctrlValDefault.disableSfc = disableSFC;
            }
            else
            {
                // Default value
                m_ctrlValDefault.disableSfc = false;
            }
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("No FtrSFCPipe, disableSfc %d", m_ctrlValDefault.disableSfc);
            m_ctrlValDefault.disableSfc = true;
        }
        VP_PUBLIC_NORMALMESSAGE("disableSfc %d", m_ctrlValDefault.disableSfc);
    }

    // If AutoDn need to be disabled
    bool disableAutoDN = false;
    status = ReadUserSetting(
        m_userSettingPtr,
        disableAutoDN,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_AUTODN,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.disableAutoDn = disableAutoDN;
    }
    else
    {
        // Default value
        m_ctrlValDefault.disableAutoDn = false;
    }
    VP_PUBLIC_NORMALMESSAGE("disableAutoDn %d", m_ctrlValDefault.disableAutoDn);

    // bComputeContextEnabled is true only if Gen12+. 
    // Gen12+, compute context(MOS_GPU_NODE_COMPUTE, MOS_GPU_CONTEXT_COMPUTE) can be used for render engine.
    // Before Gen12, we only use MOS_GPU_NODE_3D and MOS_GPU_CONTEXT_RENDER.
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrCCSNode))
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        bool computeContextEnabled = false;
        status = ReadUserSettingForDebug(
            m_userSettingPtr,
            computeContextEnabled,
            __VPHAL_ENABLE_COMPUTE_CONTEXT,
            MediaUserSetting::Group::Sequence);

        if (MOS_SUCCEEDED(status))
        {
            m_ctrlValDefault.computeContextEnabled = computeContextEnabled ? true : false;
        }
        else
#endif
        {
            // Default value
            m_ctrlValDefault.computeContextEnabled = true;
        }
    }
    else
    {
        m_ctrlValDefault.computeContextEnabled = false;
    }
    VP_PUBLIC_NORMALMESSAGE("computeContextEnabled %d", m_ctrlValDefault.computeContextEnabled);

    if (m_vpPlatformInterface)
    {
        m_ctrlValDefault.eufusionBypassWaEnabled = m_vpPlatformInterface->IsEufusionBypassWaEnabled();
    }
    else
    {
        // Should never come to here.
        VP_PUBLIC_ASSERTMESSAGE("m_vpPlatformInterface == nullptr");
    }
    VP_PUBLIC_NORMALMESSAGE("eufusionBypassWaEnabled %d", m_ctrlValDefault.eufusionBypassWaEnabled);

    MT_LOG3(MT_VP_USERFEATURE_CTRL, MT_NORMAL, MT_VP_UF_CTRL_DISABLE_VEOUT, m_ctrlValDefault.disableVeboxOutput,
        MT_VP_UF_CTRL_DISABLE_SFC, m_ctrlValDefault.disableSfc, MT_VP_UF_CTRL_CCS, m_ctrlValDefault.computeContextEnabled);

    m_ctrlVal = m_ctrlValDefault;
}

VpUserFeatureControl::~VpUserFeatureControl()
{
}

MOS_STATUS VpUserFeatureControl::Update(PVP_PIPELINE_PARAMS params)
{
    VP_PUBLIC_CHK_NULL_RETURN(params);

    m_ctrlVal = m_ctrlValDefault;

    return MOS_STATUS_SUCCESS;
}
