#pragma once
class Time
{
public:
	static Time* Get();
	Time();

	void Update(float deltaTime);
	float DeltaTime() const { return m_DeltaTime; };
private:
	float m_DeltaTime;
};

