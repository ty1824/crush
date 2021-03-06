/*
 * S_Resource.h
 */

#ifndef S_RESOURCE_H_INCLUDED
#define S_RESOURCE_H_INCLUDED

// External includes
#define WIN32_LEAN_AND_MEAN
#include <d3dx9.h>

// Project includes
#include <shared/game/Resource.h>
#include <server/game/ServerEntity.h>
#include <shared/InputState.h>
#include <server/game/S_Extractor.h>

#define SHIP_PLAYERNUM_TYPE char

class S_Extractor;

#pragma warning( push )
#pragma warning( disable : 4250 )

class S_Resource : public Resource, public ServerEntity{ //switch back to capsule

public:
	
	// Fields
	ServerEntity * m_carrier;
	bool m_onDropTimeout;
	long m_dropTimeoutStart;
	SHIP_PLAYERNUM_TYPE m_droppedFrom;
	long m_dropTimeoutLength;
	
	long m_startTravelTime;
	int m_spot;


	// Constructors
	S_Resource();
	S_Resource(D3DXVECTOR3, Quaternion);

	// Methods
	virtual D3DXMATRIX calculateRotationalInertia(float mass);
	void travel();
	void update(float delta_time);
};

#pragma warning( pop )

#endif S_RESOURCE_H_INCLUDED