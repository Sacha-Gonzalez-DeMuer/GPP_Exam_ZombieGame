/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ESingleton.h: template class to create singletons in the Engine.
/*=============================================================================*/
#ifndef ELITE_SINGLETON
#define	ELITE_SINGLETON

namespace Elite
{
	template<typename T_MemoryObject>
	class ESingleton
	{
	public:
		//=== Public Functions ===
		static T_MemoryObject* GetInstance() 
		{
			if (!m_pInstance)
				m_pInstance = new T_MemoryObject();
			return m_pInstance;
		};
		static void Destroy() 
		{ 
			delete m_pInstance; 
			m_pInstance = nullptr;
		};

	protected:
		//=== Constructors & Destructors
		ESingleton() = default;
		~ESingleton() = default;

		//=== Datamembers ===
		static T_MemoryObject* m_pInstance;

	private:
		ESingleton(ESingleton const&) {};
		ESingleton& operator=(ESingleton const&) {};
	};

	template<typename T_MemoryObject>
	typename T_MemoryObject* ESingleton<T_MemoryObject>::m_pInstance = 0;
}
#endif