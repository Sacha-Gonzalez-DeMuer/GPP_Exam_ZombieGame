#include "../../stdafx.h"
#include "BaseAgent.h"
#include "../../ExtendedStructs.h"
#include "IExamInterface.h"


BaseAgent::BaseAgent(IExamInterface* pInterface)
	:m_pInterface{ pInterface }
{
}

BaseAgent::~BaseAgent()
{
	m_pInterface = nullptr;
}

void BaseAgent::Update(float dt, SteeringPlugin_Output& steering)
{
}

void BaseAgent::Render(float dt)
{
}

EAgentInfo BaseAgent::GetInfo() const
{
	return m_pInterface->Agent_GetInfo();
}
