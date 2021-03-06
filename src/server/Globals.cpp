#include <server/Globals.h>

namespace server {
	namespace game {
		int timelimit = 5 * 60 * 1000;
	}

	namespace world {
		// World size
		float size = 800.0f;

		// Spawn
		float mothership_distance_from_center = 500.0f;
		float ship_spawn_distance_from_center = 450.0f;

		// Asteroids
		float asteroids_num = 50;
		float asteroids_pos_start = 50.0f;
		float asteroids_pos_range = 450.0f;
		float asteroids_scale_start = 0.5f;
		float asteroids_scale_range = 5.0f;
		float asteroids_vel_range = 5.0f;
		float asteroids_rot_vel_range = PI/4.0f;
	}

	namespace entities {
		float max_lin_vel = 300.0f;
		float max_rot_vel = 6.283f;

		namespace asteroid {
			float scaleToRadius = 1.5f;
			float density = 8.0f;
		}

		namespace tractorbeam {
			float power = 0.2f;
			float length = 350.0f;
			float max_velocity_increase = 100.0f;
			float invincible_resource_time_limit = 1000.0f;
		}

		namespace resource {
			float max_travel_time = 3000;
		}

		namespace ship {
			float mass = 10000.0f;
			float linear_impulse = 5000.0f;
			float rotation_impulse = 3000.0f;
			float hard_braking_impulse = 7000.0f;
			float braking_impulse = 1000.0f;//2500.0f;
			float max_velocity = 50.0f;
			float max_rotation_velocity = 2.5f;
			int mash_number = 15;
			float mash_time_limit = 5000.0f;
		}

		namespace mothership {
			float mass = 100000.0f;
		}

		namespace extractor {
			float mass = 50000.0f;
			float resource_respawn_time = 15.0f;
		}

		namespace powerup {
			float max_velocity_rate = 2.0f;
			float impulse_rate = 3.0f;
			float pulse_rate = 2.0f;
			float speedup_time = 7000;
			float shield_time = 15000;
			float pulse_time = 500;
			float pulse_range = 125;
			float respawn = 15000;

		}
	}
}