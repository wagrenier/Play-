#include <assert.h>
#include "RootCounters.h"
#include "Intc.h"
#include "Log.h"

#define LOG_NAME ("psx_counters")

using namespace Psx;
using namespace std;

CRootCounters::CRootCounters(CIntc& intc) :
m_intc(intc)
{
	Reset();
}

CRootCounters::~CRootCounters()
{

}

void CRootCounters::Reset()
{
	memset(&m_counter, 0, sizeof(m_counter));
}

void CRootCounters::Update()
{
	for(unsigned int i = 0; i < MAX_COUNTERS; i++)
	{
		COUNTER& counter = m_counter[i];
		if(i == 2 && counter.mode.en) continue;
		uint32 counterMax = counter.mode.tar ? counter.target : 0xFFFF;
		counter.count = static_cast<uint16>(min<uint32>(counter.count + 0x800, counterMax));
		if(counter.count == counter.target)
		{
			if(counter.mode.iq1 && counter.mode.iq2)
			{
				counter.count = 0;
				m_intc.AssertLine(CIntc::LINE_CNT0 + i);
			}
		}
	}
}

uint32 CRootCounters::ReadRegister(uint32 address)
{
#ifdef _DEBUG
	DisassembleRead(address);
#endif
	unsigned int counterId = (address - CNT0_BASE) / 0x10;
	unsigned int registerId = address & 0x0F;
	assert(counterId < MAX_COUNTERS);
	switch(registerId)
	{
	case CNT_COUNT:
		return m_counter[counterId].count;
		break;
	case CNT_MODE:
		return m_counter[counterId].mode;
		break;
	case CNT_TARGET:
		return m_counter[counterId].target;
		break;
	}
	return 0;
}

void CRootCounters::WriteRegister(uint32 address, uint32 value)
{
#ifdef _DEBUG
	DisassembleWrite(address, value);
#endif
	unsigned int counterId = (address - CNT0_BASE) / 0x10;
	unsigned int registerId = address & 0x0F;
	assert(counterId < MAX_COUNTERS);
	switch(registerId)
	{
	case CNT_COUNT:
		m_counter[counterId].count = static_cast<uint16>(value);
		break;
	case CNT_MODE:
		m_counter[counterId].mode <<= value;
		break;
	case CNT_TARGET:
		m_counter[counterId].target = static_cast<uint16>(value);
		break;
	}
}

void CRootCounters::DisassembleRead(uint32 address)
{
	unsigned int counterId = (address - CNT0_BASE) / 0x10;
	unsigned int registerId = address & 0x0F;
	switch(registerId)
	{
	case CNT_COUNT:
		CLog::GetInstance().Print(LOG_NAME, "CNT%i: = COUNT\r\n", counterId);
		break;
	case CNT_MODE:
		CLog::GetInstance().Print(LOG_NAME, "CNT%i: = MODE\r\n", counterId);
		break;
	case CNT_TARGET:
		CLog::GetInstance().Print(LOG_NAME, "CNT%i: = TARGET\r\n", counterId);
		break;
	default:
		CLog::GetInstance().Print(LOG_NAME, "Reading an unknown register (0x%0.8X).\r\n", address);
		break;
	}
}

void CRootCounters::DisassembleWrite(uint32 address, uint32 value)
{
	unsigned int counterId = (address - CNT0_BASE) / 0x10;
	unsigned int registerId = address & 0x0F;
	switch(registerId)
	{
	case CNT_COUNT:
		CLog::GetInstance().Print(LOG_NAME, "CNT%i: COUNT = 0x%0.4X\r\n", counterId, value);
		break;
	case CNT_MODE:
		CLog::GetInstance().Print(LOG_NAME, "CNT%i: MODE = 0x%0.8X\r\n", counterId, value);
		break;
	case CNT_TARGET:
		CLog::GetInstance().Print(LOG_NAME, "CNT%i: TARGET = 0x%0.4X\r\n", counterId, value);
		break;
	default:
		CLog::GetInstance().Print(LOG_NAME, "Writing to an unknown register (0x%0.8X, 0x%0.8X).\r\n", address, value);
		break;
	}
}
