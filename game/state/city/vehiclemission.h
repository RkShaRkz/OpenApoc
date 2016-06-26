#pragma once

#include "game/state/city/building.h"
#include "library/strings.h"
#include "library/vec.h"
#include <list>
#include <map>

namespace OpenApoc
{

class Vehicle;
class Tile;
class TileMap;
class Building;
class UString;

class VehicleMission
{
  public:
	VehicleMission();

	// Methods used in pathfinding etc.
	bool getNextDestination(GameState &state, Vehicle &v, Vec3<float> &dest);
	void update(GameState &state, Vehicle &v, unsigned int ticks);
	bool isFinished(GameState &state, Vehicle &v);
	void start(GameState &state, Vehicle &v);

	// Methods to create new missions
	static VehicleMission *gotoLocation(Vehicle &v, Vec3<int> target);
	static VehicleMission *gotoBuilding(Vehicle &v, StateRef<Building> target);
	static VehicleMission *attackVehicle(Vehicle &v, StateRef<Vehicle> target);
	static VehicleMission *snooze(Vehicle &v, unsigned int ticks);

	// INTERNAL: Not to be used directly (Only works when in building)
	static VehicleMission *takeOff(Vehicle &v);
	// INTERNAL: Not to be used directly (Only works if directly above a pad)
	static VehicleMission *land(Vehicle &v, StateRef<Building> b);

	UString getName();

	enum class MissionType
	{
		GotoLocation,
		GotoBuilding,
		FollowVehicle,
		AttackVehicle,
		AttackBuilding,
		Snooze,
		TakeOff,
		Land,
	};
	static const std::map<MissionType, UString> TypeMap;

	MissionType type;

	// GotoLocation TakeOff
	Vec3<int> targetLocation;
	// GotoBuilding AttackBuilding Land
	StateRef<Building> targetBuilding;
	// FollowVehicle AttackVehicle
	StateRef<Vehicle> targetVehicle;
	// Snooze
	unsigned int timeToSnooze;

	std::list<Vec3<int>> currentPlannedPath;
};
} // namespace OpenApoc
