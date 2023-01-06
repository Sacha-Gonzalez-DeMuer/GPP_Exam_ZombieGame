#include "stdafx.h"
#include "Time.h"

Time* Time::Get()
{
    static Time* t = new Time();

    return t;
}

Time::Time()
{
}

void Time::Update(float deltaTime)
{
    m_DeltaTime = deltaTime;
}
