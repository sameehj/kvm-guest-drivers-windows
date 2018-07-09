#pragma once
#include "ParaNdis-VirtQueue.h"
#include "ndis56common.h"
#include "ParaNdis-AbstractPath.h"

#define MAX_SCATTER_GATHER_BUFFERS 10

struct CBuffer {
    PVOID buffer;
    ULONG size;
    DECLARE_CNDISLIST_ENTRY(CBuffer);
};

typedef CNdisList<CBuffer, CRawAccess, CCountingObject> BufferList;

class CParaNdisCX : public CParaNdisTemplatePath<CVirtQueue>, public CPlacementAllocatable {
public:
    CParaNdisCX();
    ~CParaNdisCX();

    bool Create(PPARANDIS_ADAPTER Context, UINT DeviceQueueIndex);

    virtual NDIS_STATUS SetupMessageIndex(u16 vector);

    BOOLEAN CParaNdisCX::SendControlMessage(
        UCHAR cls,
        UCHAR cmd,
        PVOID buffer1,
        ULONG size1,
        PVOID buffer2,
        ULONG size2,
        int levelIfOK
        );

    BOOLEAN CParaNdisCX::SendControlMessage(
        UCHAR cls,
        UCHAR cmd,
        BufferList &outList,
        BufferList &inList,
        int levelIfOK
        );

    template<typename ...Args>
    VOID CParaNdisCX::InsertBuffersToList(BufferList &list, CBuffer *buff, Args ... args)
    {
        if (buff && buff->buffer)
        {
            list.PushBack(buff);
        }
        InsertBuffersToList(list, args...);
    }

protected:
    tCompletePhysicalAddress m_ControlData;

private:
    UINT CParaNdisCX::GetBuffer();
    VOID InsertBuffersToList(BufferList &list);
};
