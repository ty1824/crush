/*
 * S_Ship.cpp
 */

// External includes
#include <stdio.h>

// Project includes
#include <shared/util/SharedUtils.h>
#include <shared/ConfigSettings.h>
#include <shared/game/Entity.h>
#include <server/game/S_Extractor.h>
#include <server/game/S_Ship.h>
//#include <server/game/S_TractorBeam.h>

using namespace shared::utils;
using namespace server::entities::ship;

S_Ship::S_Ship() :
	Entity(SHIP),
	Ship(),
	ServerEntity(),
	m_linear_impulse(linear_impulse),
	m_rotation_impulse(rotation_impulse),
	m_max_velocity(max_velocity),
	m_max_rotation_velocity(max_rotation_velocity),
	m_enable_reverse_noise(false)
{
	init();
}

S_Ship::S_Ship(D3DXVECTOR3 pos, Quaternion orientation, int pNum) :
	Entity(genId(), SHIP, pos, orientation, 3),
	Ship(pNum),
	ServerEntity(mass, 3.0, calculateRotationalInertia(mass)),
	m_linear_impulse(linear_impulse),
	m_rotation_impulse(rotation_impulse),
	m_braking_impulse(braking_impulse),
	m_hard_braking_impulse(hard_braking_impulse),
	m_max_velocity(max_velocity),
	m_max_rotation_velocity(max_rotation_velocity),
	m_isBraking(false),
	m_enable_reverse_noise(false)
{	
	init();
}

void S_Ship::init() {
	forward_rot_thruster = D3DXVECTOR3(0, 0, 5);
	reverse_rot_thruster = D3DXVECTOR3(0, 0, -5);
	m_resourceSpots = 1;
	m_resource = NULL;
	m_powerup = NULL;
	m_pressToggle = false;
	m_mashNumber = mash_number;
	m_mashTimeLimit = mash_time_limit;
	m_shieldOn = false;
	m_pulseOn = false;
	m_enable_reverse_noise = false;

	m_baseRadius = m_radius;
	m_baseLength = m_length;
}

// TODO!!!:
// This method needs to be extracted to the server/physics engine.
void S_Ship::addPlayerInput(InputState input) {
	
	if(input.getPushBurst()) {

		m_tractorBeam->setIsOn(true);
		m_tractorBeam->setIsPulling(false);
	}
	// Once holding on an object it is hooked so no longer need to be pulling
	else if(input.getTractorBeam() != 0){// && !m_tractorBeam->m_isHolding) { 

		m_tractorBeam->setIsOn(true);
		m_tractorBeam->setIsPulling(true);
		m_tractorBeam->m_strength = (float) input.getTractorBeam();
	}
	else {//if(!m_tractorBeam->m_isHolding){
		m_tractorBeam->setIsOn(false);
	}

	if(input.getBrake()) m_isBraking = true;
	else m_isBraking = false;

	m_reversing = input.getReverse();
	m_reverse = m_reversing && m_enable_reverse_noise;

	if(input.getMash() && !m_pressToggle) {
		m_presses.push_back(GetTickCount());
		m_pressToggle = true;
	}
	else if(!input.getMash()) m_pressToggle = false;

	if(m_powerup != NULL && m_powerup->m_stateType == HOLDING && input.getPowerup()) {
		m_powerup->start();
	}

	m_thruster = input.getThrust();

	// Linear thrust calculations
	if (abs(input.getThrust()) > FP_ZERO || abs(input.getStrafe()) > FP_ZERO) {
		m_thrusting = true;
		D3DXVECTOR3 main_thrust_force((float)input.getStrafe(), 0, (float)m_thruster), 
					main_thrust_adj;
		float thrust_normalize = max(1.0f / D3DXVec3Length(&main_thrust_force), 1.0f);
		main_thrust_force *= thrust_normalize;
		D3DXVec3Rotate(&main_thrust_adj, &main_thrust_force, &m_orientation);
		
		applyLinearImpulse(main_thrust_adj * m_linear_impulse);
	} 
	
	if (m_reversing) {
		D3DXVECTOR3 main_thrust_force(0, 0, -1), 
					main_thrust_adj;
		D3DXVec3Normalize(&main_thrust_force, &main_thrust_force);
		D3DXVec3Rotate(&main_thrust_adj, &main_thrust_force, &m_orientation);
		
		applyLinearImpulse(main_thrust_adj * m_linear_impulse);
	}

	// Rotational thrust calculations
	if (abs(input.getPitch()) > FP_ZERO || abs(input.getTurn()) > FP_ZERO) {
		m_rotating = true;
		D3DXVECTOR3 rot_thrust_adj((float)input.getPitch(), (float)input.getTurn(), 0);
		float thrust_normalize = max(1.0f / D3DXVec3Length(&rot_thrust_adj), 1.0f);
		rot_thrust_adj *= thrust_normalize;
		D3DXVec3Rotate(&rot_thrust_adj, &rot_thrust_adj, &m_orientation);

		applyAngularImpulse(rot_thrust_adj * m_rotation_impulse);
	}
}

D3DXVECTOR3 S_Ship::getDamping() {
	D3DXVECTOR3 shipImp = t_impulse;
	float mag_velocity = D3DXVec3Length(&m_velocity), mag_angular_velocity = D3DXVec3Length(&m_angular_velocity);
	if (mag_velocity > FP_ZERO) {
		// Linear stabilizer
		D3DXVECTOR3 lin_stabilizer_vec(-m_momentum);
		D3DXVec3Normalize(&lin_stabilizer_vec, &lin_stabilizer_vec);

		if (m_thrusting) {
			// Thrusting, we only want to reduce the impulse
			D3DXVECTOR3 lin_stabilizer_force = lin_stabilizer_vec * m_linear_impulse;
			float damping_factor = (mag_velocity / m_max_velocity);
			shipImp += lin_stabilizer_force * damping_factor;
		}
		else if(m_isBraking) {
			// Not thrusting, we need to slow down as quickly as possible
			D3DXVECTOR3 lin_stabilizer_force = lin_stabilizer_vec * m_hard_braking_impulse;
			shipImp += Vec3ComponentAbsMin(lin_stabilizer_force, -m_momentum);
		} 
		else {
			// Not thrusting, we need to slow down as quickly as possible
			D3DXVECTOR3 lin_stabilizer_force = lin_stabilizer_vec * m_braking_impulse;
			shipImp += Vec3ComponentAbsMin(lin_stabilizer_force, -m_momentum);
		}
	}

	return shipImp;
}


void S_Ship::applyDamping() {
		
	// Linear damping
	float mag_velocity = D3DXVec3Length(&m_velocity), mag_angular_velocity = D3DXVec3Length(&m_angular_velocity);
//if(!m_tractorBeam->m_isHolding){
	if (mag_velocity > FP_ZERO) {
		// Linear stabilizer
		D3DXVECTOR3 lin_stabilizer_vec(-m_momentum);
		D3DXVec3Normalize(&lin_stabilizer_vec, &lin_stabilizer_vec);

		if (m_thrusting || m_reversing) {
			// Thrusting, we only want to reduce the impulse
			D3DXVECTOR3 lin_stabilizer_force = lin_stabilizer_vec * m_linear_impulse;
			float damping_factor = (mag_velocity / m_max_velocity);
			applyLinearImpulse(lin_stabilizer_force * damping_factor);
		} else if(m_isBraking) {
			// Not thrusting, we need to slow down as quickly as possible
			D3DXVECTOR3 lin_stabilizer_force = lin_stabilizer_vec * m_hard_braking_impulse;
			float damping_factor = max(1.0f, (mag_velocity / m_max_velocity));
			applyLinearImpulse(Vec3ComponentAbsMin(lin_stabilizer_force * damping_factor, -m_momentum));
		} else {
			// Not thrusting, we need to slow down as quickly as possible
			D3DXVECTOR3 lin_stabilizer_force = lin_stabilizer_vec * m_braking_impulse;
			float damping_factor = max(1.0f, mag_velocity / m_max_velocity);
			applyLinearImpulse(Vec3ComponentAbsMin(lin_stabilizer_force * damping_factor, -m_momentum));
		}
	}
//}

	// Rotation damping
	if (mag_angular_velocity > FP_ZERO) {
		// Rotation stabilizer
		D3DXVECTOR3 rot_stabilizer_vec(-m_angular_momentum);
		D3DXVec3Normalize(&rot_stabilizer_vec, &rot_stabilizer_vec);

		D3DXVECTOR3 rot_stabilizer_force = rot_stabilizer_vec * m_rotation_impulse;
		
		if (m_rotating) {
			// Rotating, we only want to reduce the impulse
			float damping_factor = mag_angular_velocity > m_max_rotation_velocity ? 
				pow((mag_angular_velocity / m_max_rotation_velocity), 2) : 
				mag_angular_velocity / m_max_rotation_velocity;
			applyAngularImpulse(rot_stabilizer_force * damping_factor);
		} else {
			// Not rotating, we need to slow down as quickly as possible
			float damping_factor = max(1, pow((mag_angular_velocity / m_max_rotation_velocity), 2));
			applyAngularImpulse(Vec3ComponentAbsMin(rot_stabilizer_force * damping_factor, -m_angular_momentum));
		}
	}

	m_thrusting = false;
	m_rotating = false;

}

void S_Ship::dropResource(long time) {
	if(m_resource == NULL) return;

	S_Resource * tmp = m_resource;
	m_resource = NULL;
	tmp->m_carrier = NULL;
	tmp->m_onDropTimeout = true;
	tmp->m_dropTimeoutStart = GetTickCount();
	tmp->m_dropTimeoutLength  = time;
	tmp->m_droppedFrom = m_playerNum;
	tmp->m_spot = -1;
	tmp->reset(); // temporary to stop resources from moving far far away when dropped
}

void S_Ship::disableTractorBeam() {
	m_tractorBeam->disable(5000);
}


// Drops resource and locks enemy tractor beam on it (assumes enemy tractor beam on ship)
void S_Ship::unlockResource(S_Ship * enemy) {
	S_Resource * tmp = m_resource;
	dropResource(4000);
	enemy->m_tractorBeam->lockOn(tmp);

}

void S_Ship::updateDefensiveOffensiveCounter() {
	m_isLockOnTarget =m_heldBy.size() > 0;

	// Erases mashes that have gone for too long
	for(int i = m_presses.size() -1; i >= 0; i--) {
		long time = GetTickCount();
		if(time - m_presses[i] > m_mashTimeLimit) //config
			m_presses.erase(m_presses.begin() + i);
	}
	
	if(m_heldBy.size() > 0) {
		if(m_presses.size() >= 15) { 
			for(int i = 0; i < m_heldBy.size(); i++)
				((S_Ship *)m_heldBy[i])->disableTractorBeam();
			m_heldBy.clear();
			m_presses.clear();
		}
	}
	else if(m_tractorBeam->m_object != NULL && 
		m_tractorBeam->m_object->m_type == SHIP && 
		((S_Ship *)m_tractorBeam->m_object)->m_resource != NULL) 
	{
			if(m_presses.size() >= m_mashNumber) { 
				((S_Ship *)m_tractorBeam->m_object)->unlockResource(this);
				m_presses.clear();
			}
	}
	else {
		m_presses.clear();
	}
}

void S_Ship::update(float delta_time) {
	m_useAltSprite = m_resource != NULL;

	// If a Powerup's use is up ends it
	if(m_powerup != NULL && m_powerup->m_stateType == CONSUMED){
		if(m_powerup->check(GetTickCount())){
			m_powerup->end();
		}
	}

	updateDefensiveOffensiveCounter();

	applyDamping();
	
	ServerEntity::update(delta_time);
}

void S_Ship::calcTractorBeam() {

	m_tractorBeam->updateData();
}

void S_Ship::checkDropoff(S_Mothership * mother) {
	if(mother->m_playerNum == m_playerNum && m_resource!= NULL) { // get rid of if want to steal without hitting mothership
		if(D3DXVec3Length(&(m_pos-mother->m_pos)) < mother->m_length*2){
			mother->interact(this);
		}
	}
}


bool S_Ship::checkShield() {
	if(m_shieldOn) return true;
	else return false;
}

void S_Ship::disableShield() {
	m_shieldOn = false;
}

D3DXMATRIX S_Ship::calculateRotationalInertia(float mass){
	float radius_squared = 5 * 5;
	float height_squared = (2 * 5.0f) * (2 * 5.0f);
	return *D3DXMatrixScaling(&D3DXMATRIX(), (1.0f / 12.0f) * mass * (3 * radius_squared + height_squared),
											 (1.0f / 12.0f) * mass * (3 * radius_squared + height_squared),
											 (0.5f) * mass * radius_squared);
};


bool S_Ship::interact(S_Powerup * power) {
	if(power->m_ship == NULL && power->m_stateType == SPAWNED && m_powerup == NULL) {	
		power->pickUp(this);
		m_powerup = power;
		return true;
	}

	return false;
}

int S_Ship::interact(S_Resource * res) {
	if(m_resource == NULL && (res->m_carrier == NULL || res->m_carrier->m_type == EXTRACTOR)) {
		if(((res->m_droppedFrom != m_playerNum && res->m_onDropTimeout) || res->m_droppedFrom != m_playerNum)) {
			m_resource = res;
			res->m_carrier = this;
			res->m_spot = 0;
			res->m_startTravelTime = GetTickCount();
			cout<<"Gathered"<<endl;

			for(int i = 0; i < res->m_heldBy.size(); i++)
				((S_Ship *)res->m_heldBy[i])->m_tractorBeam->lockOff();
			res->m_heldBy.clear();

			return 1;
		}

	}

	if(m_tractorBeam->m_object != NULL && m_tractorBeam->m_object == res){
		m_tractorBeam->m_isColliding = true;
		return -1;
	}

	return 0;
}

bool S_Ship::interact(S_Asteroid * asteroid) {
	if(m_tractorBeam->m_object != NULL && m_tractorBeam->m_object == asteroid){
		m_tractorBeam->m_isColliding = true;
		return false;
	}
	else if(m_resource != NULL && !m_shieldOn && m_tractorBeam->m_lastHeld != asteroid) {
		dropResource(2000);
	}
	return true;
}

bool S_Ship::interact(S_Ship * ship) {
	bool switched = false;

	if(m_tractorBeam->m_object != NULL && m_tractorBeam->m_object == ship){
		m_tractorBeam->m_isColliding = true;
		m_tractorBeam->lockOff();
	}
	if(m_resource != NULL && !m_shieldOn && ship->m_resource == NULL && m_resource->m_droppedFrom != ship->m_playerNum) {
		S_Resource * r = m_resource;
		dropResource(2000);
		ship->interact(r);
		switched = true;
	}

	if(ship->m_tractorBeam->m_object != NULL && ship->m_tractorBeam->m_object == this){
		m_tractorBeam->m_isColliding = true;
		ship->m_tractorBeam->lockOff();
	}
	if(!switched && ship->m_resource != NULL && !ship->m_shieldOn && m_resource == NULL && ship->m_resource->m_droppedFrom != m_playerNum) {
		S_Resource * r = ship->m_resource;
		ship->dropResource(2000);
		interact(r);
	}
	return true;
}



void S_Ship::setFowardImpulse(float forward) {
	m_linear_impulse = forward;
}

float S_Ship::getForwardImpulse() {
	return m_linear_impulse;
}

void S_Ship::setMaxVelocity(float vel) {
	m_max_velocity = vel;
}

float S_Ship::getMaxVelocity() {
	return m_max_velocity;
}
