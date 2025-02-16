/*
* Copyright (c) 2019, Intel Corporation
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
* FITNESS FOR A PARTICULAR PURPOSNextE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//! \file     mos_os_next.cpp
//! \brief    Unified OS Formats
//! \details  Unified OS Formats
//!

#include "mos_os_next.h"
#include "mos_interface.h"
#include "mos_gpucontext_specific_next.h"

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED

std::shared_ptr<GpuCmdResInfoDumpNext> GpuCmdResInfoDumpNext::m_instance = nullptr;

const GpuCmdResInfoDumpNext *GpuCmdResInfoDumpNext::GetInstance(PMOS_CONTEXT mosCtx)
{
    if (m_instance == nullptr)
    {
        m_instance = std::make_shared<GpuCmdResInfoDumpNext>(mosCtx);
    }
    return m_instance.get();
}

GpuCmdResInfoDumpNext::GpuCmdResInfoDumpNext(PMOS_CONTEXT mosCtx)
{
    MediaUserSettingSharedPtr   userSettingPtr  = nullptr;
    MediaUserSetting::Value     value;

    userSettingPtr = ::GetUserSettingInstance(mosCtx);

    ReadUserSetting(
        userSettingPtr,
        m_dumpEnabled,
        __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_INFO_ENABLE,
        MediaUserSetting::Group::Device);

    if (!m_dumpEnabled)
    {
        return;
    }

    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_INFO_PATH,
        MediaUserSetting::Group::Device);

    auto path = value.ConstString();
    if(path.size() > 0)
    {
        m_path = path;
        if (path.back() != '/' && path.back() != '\\')
        {
            m_path += '/';
        }
    }
    m_path = m_path + "gpuCmdResInfo_" + std::to_string(MosUtilities::MosGetPid()) + ".txt";
}

void GpuCmdResInfoDumpNext::Dump(PMOS_INTERFACE pOsInterface) const
{
    if (!m_dumpEnabled)
    {
        return;
    }

    using std::endl;

    std::ofstream outputFile;
    outputFile.open(m_path, std::ios_base::app);
    if(!outputFile.is_open())
    {
        MOS_OS_ASSERTMESSAGE("file open failed");
        return;
    }

    auto &cmdResInfoPtrs = GetCmdResPtrs(pOsInterface);

    outputFile << "--PerfTag: " << std::to_string(pOsInterface->pfnGetPerfTag(pOsInterface)) << " --Cmd Num: "
        << cmdResInfoPtrs.size() << " --Dump Count: " << ++m_cnt << endl;

    outputFile << "********************************CMD Paket Begin********************************" << endl;
    for (auto e : cmdResInfoPtrs)
    {
        Dump(e, outputFile);
    }
    outputFile << "********************************CMD Paket End**********************************" << endl << endl;

    outputFile.close();
}

const char *GpuCmdResInfoDumpNext::GetResType(MOS_GFXRES_TYPE resType) const
{
    switch (resType)
    {
    case MOS_GFXRES_INVALID:
        return "MOS_GFXRES_INVALID";
    case MOS_GFXRES_BUFFER:
        return "MOS_GFXRES_BUFFER";
    case MOS_GFXRES_2D:
        return "MOS_GFXRES_2D";
    case MOS_GFXRES_VOLUME:
        return "MOS_GFXRES_VOLUME";
    default:
        return "";
    }
}

const char *GpuCmdResInfoDumpNext::GetTileType(MOS_TILE_TYPE tileType) const
{
    switch (tileType)
    {
    case MOS_TILE_X:
        return "MOS_TILE_X";
    case MOS_TILE_Y:
        return "MOS_TILE_Y";
    case MOS_TILE_YF:
        return "MOS_TILE_YF";
    case MOS_TILE_YS:
        return "MOS_TILE_YS";
    case MOS_TILE_LINEAR:
        return "MOS_TILE_LINEAR";
    case MOS_TILE_INVALID:
        return "MOS_TILE_INVALID";
    default:
        return "";
    }
}
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED
