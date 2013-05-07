/*
 * S_Mothership.h
 */

#ifndef S_MOTHERSHIP_H_INCLUDED
#define S_MOTHERSHIP_H_INCLUDED

// External includes
#define WIN32_LEAN_AND_MEAN
#include <d3dx9.h>
#include <vector>

// Project includes
#include <shared/game/Mothership.h>
#include <server/game/S_Resource.h>
#include <server/game/S_Ship.h>
#include <server/game/ServerEntity.h>
#include <shared/InputState.h>

namespace server {
	namespace entities {
		namespace mothership {
			static const string CONFIG_PREFIX = "mothership_";

			static float mass = 25000.0f;

			inline void initFromConfig() {
				ConfigSettings::config->getValue(CONFIG_PREFIX + "mass", mass);
			}
		}
	}
}

#pragma warning( push )
#pragma warning( disable : 4250 )

class S_Mothership : public Mothership, public ServerEntity{ //switch back to capsule

public:

	// Fields
	vector<S_Resource *> m_resourceList;
	// Constructors
	S_Mothership(int);
	S_Mothership(D3DXVECTOR3, Quaternion, int);

	// Methods
	virtual D3DXVECTOR3 calculateRotationalInertia(float mass);
	bool interact(S_Ship * ship);
	void update(float delta_time);

private:
	bool giveResource(S_Ship * ship);
	bool takeResource(S_Ship * ship);
	
};

#pragma warning( pop )

#endif S_MOTHERSHIP_H_INCLUDED