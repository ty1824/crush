//=================================================================================================
// EnginePGroup.h - Engine Particle Group
//=================================================================================================

#ifndef ENGINEBEAMPGROUP_H_INCLUDED
#define ENGINEBEAMPGROUP_H_INCLUDED

// Defines
#define WIN32_LEAN_AND_MEAN

// Global includes
#include <d3dx9.h>

// Project includes
#include <client/graphics/ParticleGroup.h>
#include <client/graphics/entities/C_Ship.h>
#include <client/graphics/ParticleSystem.h> // for static functions

class EnginePGroup : public ParticleGroup {
public:
	EnginePGroup(LPDIRECT3DTEXTURE9 ptexParticle);
	~EnginePGroup();

	//static const float defaultSize;
	static const float ttl;
	static const float defaultSize;
	static const float zStartOffset;
	static const float speed;
	static const int color_r;
	static const int color_g;
	static const int color_b;

	C_Ship * shipEnt;
	float prevElapsedTime;
	
	virtual void updateGroup();
	virtual bool initNewParticle(Particle * pParticle);
	virtual bool updateParticle(Particle * pParticle, float elapsedTime);

};

#endif // ENGINEBEAMPGROUP_H_INCLUDED
