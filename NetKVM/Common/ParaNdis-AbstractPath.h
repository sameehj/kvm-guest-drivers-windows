#pragma once
#include "ParaNdis-VirtQueue.h"
#include "Parandis_DesignPatterns.h"

class CParaNdisAbstractPath
{
public:
#if NDIS_SUPPORT_NDIS620
    CParaNdisAbstractPath()
    {
        memset(&DPCAffinity, 0, sizeof(DPCAffinity));
    }
#else
    CParaNdisAbstractPath() : DPCTargetProcessor(0) {}
#endif

    bool WasInterruptReported() 
    {
        return m_interruptReported;
    }

    void ClearInterruptReport()
    {
        m_interruptReported = false;
    }

    void ReportInterrupt() {
        m_interruptReported = true;
    }

    UINT getMessageIndex() {
        return m_messageIndex;
    }

    UINT getQueueIndex() {
        return m_queueIndex;
    }

    virtual NDIS_STATUS SetupMessageIndex(u16 vector);

    /* TODO - Path classes should inherit from CVirtQueue*/
    virtual void DisableInterrupts()
    {
        m_pVirtQueue->DisableInterrupts();
    }

    void EnableInterrupts()
    {
        m_pVirtQueue->EnableInterrupts();
    }

    void Renew()
    {
        m_pVirtQueue->Renew();
    }

    ULONG getCPUIndex();

    LARGE_INTEGER& GetLastInterruptTimestamp()
    {
        return m_LastInterruptTimeStamp;
    }

#if NDIS_SUPPORT_NDIS620
    GROUP_AFFINITY DPCAffinity;
#else
    KAFFINITY DPCTargetProcessor = 0;
#endif

protected:
    PPARANDIS_ADAPTER m_Context;
    CVirtQueue *m_pVirtQueue;
    LARGE_INTEGER m_LastInterruptTimeStamp;

    u16 m_messageIndex = (u16)-1;
    u16 m_queueIndex = (u16)-1;
    bool m_interruptReported;
};


template <class VQ> class CParaNdisTemplatePath : public CParaNdisAbstractPath, public CObserver<SMNotifications>{
public:
    CParaNdisTemplatePath() : m_ObserverAdded(false) {
        m_pVirtQueue = &m_VirtQueue;
    }

    bool CreatePath()
    {
        m_ObserverAdded = m_Context->m_StateMachine.Add(this) > 0;
        return true;
    }

    ~CParaNdisTemplatePath() {
        if (m_ObserverAdded)
        {
            m_Context->m_StateMachine.Remove(this);
        }
    }

    void Shutdown()
    {
        TPassiveSpinLocker LockedContext(m_Lock);
        m_VirtQueue.Shutdown();
    }

    static BOOLEAN _Function_class_(MINIPORT_SYNCHRONIZE_INTERRUPT)
    RestartQueueSynchronously(PVOID ctx)
    {
        auto This = static_cast<CParaNdisTemplatePath<VQ>*>(ctx);
        return !This->m_VirtQueue.Restart();
    }

    /* We get notified by the state machine on surprise removal */
    void Notify(SMNotifications message) override
    {
        if (message == SupriseRemoved)
        {
            m_VirtQueue.DoNotTouchHardware();
        }
    }

protected:
    CNdisSpinLock m_Lock;
    bool m_ObserverAdded;
    VQ m_VirtQueue;
};
