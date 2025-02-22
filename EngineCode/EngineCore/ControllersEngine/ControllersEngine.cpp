#include "EngineCore/stdafx.h"
#include "ControllersEngine.h"

#include "EngineCore/MainEngine/Engine.h"
#include "EngineCore/Actors/ActorObjects.h"

#include "EngineCore/ControllersEngine/IControllersState.h"

#include "EngineCore/EngineHelpers/ActorsCommonFunctions.h"

#include "Common/MemoryLeaks.h"


ControllersEngine::ControllersEngine( Engine* parent )
{
	engine = parent;
	m_globalState = new IControllersState();
	m_globalState->Engine = engine;
}


ControllersEngine::~ControllersEngine()
{
	delete m_globalState;
}



/**@brief Wywołuje kontrolery na obsługiwanych obiektach.

Funkcja wywoływana przed MovementEngine::ProceedMovement.
@see @ref Controllers*/
void ControllersEngine::ProceedControllersPre( float timeInterval )
{
	//todo:	przerobić na wersję wielowątkową
	for ( unsigned int i = 0; i < m_preControlledObjects.size(); ++i )
		if ( m_preControlledObjects[i]->GetController() != nullptr )
			m_preControlledObjects[i]->GetController()->ControlObjectPre( m_preControlledObjects[i], m_globalState );
}

/**@brief Wywołuje kontrolery na obsługiwanych obiektach.

Funkcja wywoływana po MovementEngine::ProceedMovement.
@see @ref Controllers*/
void ControllersEngine::ProceedControllersPost( float timeInterval )
{
	//todo:	przerobić na wersję wielowątkową
	for ( unsigned int i = 0; i < m_postControlledObjects.size( ); ++i )
		if ( m_postControlledObjects[i]->GetController( ) != nullptr )
			m_postControlledObjects[i]->GetController( )->ControlObjectPost( m_postControlledObjects[i], m_globalState );
}

/**@brief Aktualizuje stan m_globalState.*/
void ControllersEngine::SingleThreadedUpdatePhase( float timeInterval )
{
	UpdateGlobalState( timeInterval );
}


/**@brief Dodaje podany obiekt do kontrolerów wywoływanych przed przesunięciem obiektów.*/
bool ControllersEngine::AddPreControlled( DynamicActor* actor )
{
	if ( actor == nullptr )
		return false;

	m_preControlledObjects.push_back( actor );
	return true;
}

/**@brief Dodaje podany obiekt do kontrolerów wywoływanych po przesunięciu obiektów.*/
bool ControllersEngine::AddPostControlled( DynamicActor* actor )
{
	if ( actor == nullptr )
		return false;

	m_postControlledObjects.push_back( actor );
	return true;
}

/**@brief Usuwa aktora z tablicy obiektów pre kontrolowanych.*/
void ControllersEngine::RemovePreCtrlActor( ActorBase* actor )
{
	ActorsCommonFunctions::RemoveActor( m_preControlledObjects, static_cast< DynamicActor* >( actor ) );
}

/**@brief Usuwa aktora z tablicy obiektów post kontrolowanych.*/
void ControllersEngine::RemovePostCtrlActor( ActorBase* actor )
{
	ActorsCommonFunctions::RemoveActor( m_postControlledObjects, static_cast< DynamicActor* >( actor ) );
}

/**@brief Usuwa aktora z modułu.*/
void ControllersEngine::RemoveActor( ActorBase* actor )
{
	RemovePreCtrlActor( actor );
	RemovePostCtrlActor( actor );
}

/**@brief Usuwa wszystkich aktorów.*/
void ControllersEngine::RemoveAllActors()
{
	m_postControlledObjects.clear();
	m_preControlledObjects.clear();
}

/**@brief */
IControllersState* ControllersEngine::GetGlobalState()
{
	return m_globalState;
}

//====================================================================================//
//			Private	
//====================================================================================//

void		ControllersEngine::UpdateGlobalState				( float timeInterval )
{
	m_globalState->Camera = engine->Rendering.GetCurrentCamera()->GetCameraData();
}

