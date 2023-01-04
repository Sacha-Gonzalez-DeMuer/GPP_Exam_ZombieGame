#pragma once
#include "../../ExtendedStructs.h"
class IExamInterface;

class BaseAgent
{
public:
	BaseAgent(IExamInterface* pInterface);
	virtual ~BaseAgent();
	BaseAgent(const BaseAgent& other) = delete;
	BaseAgent(BaseAgent&& other) = delete;
	BaseAgent& operator=(const BaseAgent& other) = delete;
	BaseAgent& operator=(BaseAgent&& other) = delete;

	//--- Agent Functions ---
	virtual void Update(float dt, SteeringPlugin_Output& steering);
	virtual void Render(float dt);

	EAgentInfo GetInfo() const;
protected:
	IExamInterface* m_pInterface;
};

