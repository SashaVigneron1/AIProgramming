/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#pragma once
#include "EliteMath/EMath.h"
#include "EBehaviourTree.h"

#include "SteeringBehaviours.h"
#include "CombinedSteeringBehaviours.h"

#include "Plugin.h"

bool gIsDebugging = false;

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
Elite::BehaviourState ShootEnemy(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "ShootEnemy\n";

	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);

	if (!pPlugin) return Elite::BehaviourState::Failure;
	if (!pInterface) return Elite::BehaviourState::Failure;

	auto agentInfo = pInterface->Agent_GetInfo();

	auto entities = pPlugin->GetEntitiesInFOV();
	Elite::Vector2 targetPos;
	float closestDistance{ FLT_MAX };
	for (const EntityInfo& entity : entities)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			float currDistance = Elite::Distance(agentInfo.Position, entity.Location);
			if (currDistance < closestDistance)
			{
				targetPos = entity.Location;
				closestDistance = currDistance;
			}
		}
	}

	pPlugin->SetSteeringBehaviour(SteeringType::FaceFlee);
	pPlugin->SetTarget(targetPos);
	pPlugin->TryAndShootEnemy(targetPos);
	pPlugin->SetHasVisitedLeftBottom(false);

	return Elite::BehaviourState::Success;
}
Elite::BehaviourState RunAwayFromEnemy(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "RunAwayFromEnemy\n";

	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);

	if (!pPlugin) return Elite::BehaviourState::Failure;
	if (!pInterface) return Elite::BehaviourState::Failure;

	auto agentInfo = pInterface->Agent_GetInfo();

	auto entities = pPlugin->GetEntitiesInFOV();

	pPlugin->SetHasVisitedLeftBottom(false);
	//if (!pPlugin->IsRunningAway()) pPlugin->SetRunningAway(true);
	pPlugin->SetSteeringBehaviour(SteeringType::SeekFlee);

	if (entities.size() > 0) 
	{
		Elite::Vector2 targetPos;
		float closestDistance{ FLT_MAX };
		for (const EntityInfo& entity : entities)
		{
			if (entity.Type == eEntityType::ENEMY)
			{
				float currDistance = Elite::Distance(agentInfo.Position, entity.Location);
				if (currDistance < closestDistance)
				{
					targetPos = entity.Location;
					closestDistance = currDistance;
				}
			}
		}

		if (pPlugin->IsGoingToCentre()) 
		{
			pPlugin->SetTarget(Elite::Vector2(0, 0) + 10 * targetPos);
		}
		else
		{
			//pPlugin->SetTarget(targetPos);
		}
	}
	

	

	return Elite::BehaviourState::Success;
}
Elite::BehaviourState StartRunning(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "StartRunning\n";

	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);

	if (!pPlugin) return Elite::BehaviourState::Failure;
	if (!pInterface) return Elite::BehaviourState::Failure;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	Elite::Vector2 targetPos{ agentInfo.Position - agentInfo.LinearVelocity };

	pPlugin->SetHasVisitedLeftBottom(false);
	if (!pPlugin->IsRunningAway() && agentInfo.Stamina > 5.0f) pPlugin->SetRunningAway(true);

	return Elite::BehaviourState::Success;
}
Elite::BehaviourState LookBehind(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "lookbehind\n";

	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);

	if (!pPlugin) return Elite::BehaviourState::Failure;
	if (!pInterface) return Elite::BehaviourState::Failure;

	Elite::Vector2 target;
	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	target = agentInfo.Position - agentInfo.LinearVelocity;

	pPlugin->SetHasVisitedLeftBottom(false);
	pPlugin->SetSteeringBehaviour(SteeringType::FaceFlee);
	pPlugin->SetTarget(target);

	return Elite::BehaviourState::Success;
}
Elite::BehaviourState SetHouseLooted(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "donelooting\n";

	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);

	if (!pPlugin) return Elite::BehaviourState::Failure;
	if (!pInterface) return Elite::BehaviourState::Failure;

	auto agentInfo = pInterface->Agent_GetInfo();

	auto houses = pPlugin->GetHousesInFOV();
	HouseInfo closestHouse;
	float closestDistance{ FLT_MAX };
	for (const HouseInfo& house : houses)
	{
		float currDistance = Elite::Distance(agentInfo.Position, house.Center);
		if (currDistance < closestDistance)
		{
			closestHouse = house;
			closestDistance = currDistance;
		}
	}

	pPlugin->SetLastHouse(closestHouse);
	return Elite::BehaviourState::Success;
}

Elite::BehaviourState PickupClosestItem(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "item\n";

	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);

	if (!pPlugin) return Elite::BehaviourState::Failure;
	if (!pInterface) return Elite::BehaviourState::Failure;

	auto agentInfo = pInterface->Agent_GetInfo();

	auto entities = pPlugin->GetEntitiesInFOV();
	Elite::Vector2 targetPos;
	float closestDistance{ FLT_MAX };
	for (const EntityInfo& entity : entities)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			float currDistance = Elite::Distance(agentInfo.Position, entity.Location);
			if (currDistance < closestDistance)
			{
				targetPos = entity.Location;
				closestDistance = currDistance;
			}
		}
	}

	pPlugin->SetSteeringBehaviour(SteeringType::Seek);
	pPlugin->SetTarget(targetPos);
	pPlugin->TryAndPickupItem();

	return Elite::BehaviourState::Success;
}

Elite::BehaviourState GoToBottomLeftOfHouse(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "bottomleft\n";


	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);

	if (!pPlugin) return Elite::BehaviourState::Failure;

	std::vector<HouseInfo> houses = pPlugin->GetHousesInFOV();
	if (houses.size() > 0) 
	{
		for (const HouseInfo& house : houses)
		{
			if (!pPlugin->WasLastHouse(house))
			{
				const float offset{ 5.f };
				Elite::Vector2 bottomLeft;
				bottomLeft = house.Center - Elite::Vector2((house.Size.x / 2.f) - offset, (house.Size.y / 2.f) - offset);
				pBlackboard->ChangeData("lastHousePos", bottomLeft);
				
				break;
			}
		}
	}

	Elite::Vector2 houseLeftBottom = Elite::Vector2(0, 0);
	pBlackboard->GetData("lastHousePos", houseLeftBottom);
	pPlugin->SetTarget(houseLeftBottom);
	pPlugin->SetTargetHouse(houseLeftBottom);
	pPlugin->SetIsWalkingToHouse(true);
	pPlugin->SetSteeringBehaviour(SteeringType::Seek);

	return Elite::BehaviourState::Success;
}
Elite::BehaviourState GoToTopRightOfHouse(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "topright\n";

	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);

	if (!pPlugin) return Elite::BehaviourState::Failure;

	std::vector<HouseInfo> houses = pPlugin->GetHousesInFOV();
	if (houses.size() > 0)
	{
		const float offset{ 5.f };
		Elite::Vector2 topRight;
		topRight = houses[0].Center + Elite::Vector2((houses[0].Size.x / 2.f) - offset, (houses[0].Size.y / 2.f) - offset);
		pPlugin->SetSteeringBehaviour(SteeringType::Seek);
		pPlugin->SetTarget(topRight);
		pPlugin->SetHasVisitedLeftBottom(true);
	}

	return Elite::BehaviourState::Success;
}
Elite::BehaviourState FleeHouse(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "Fleehouse\n";

	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);

	if (!pPlugin) return Elite::BehaviourState::Failure;

	std::vector<HouseInfo> houses = pPlugin->GetHousesInFOV();
	if (houses.size() > 0)
	{
		pPlugin->SetSteeringBehaviour(SteeringType::Flee);
		pPlugin->SetTarget(houses[0].Center);
		pPlugin->SetHasVisitedLeftBottom(false);

	}

	return Elite::BehaviourState::Success;
}

Elite::BehaviourState GoToCenterOfMap(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "GoCentre\n";

	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);

	if (!pPlugin) return Elite::BehaviourState::Failure;

	if (!pPlugin->IsGoingToCentre()) pPlugin->SetIsGoingToCentre(true);
	pPlugin->SetSteeringBehaviour(SteeringType::Seek);
	pPlugin->SetTarget(Elite::Vector2(0.f,0.f));

	return Elite::BehaviourState::Success;
}

Elite::BehaviourState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "Wander\n";

	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);

	if (!pPlugin) return Elite::BehaviourState::Failure;

	pPlugin->SetSteeringBehaviour(SteeringType::Wander);



	return Elite::BehaviourState::Success;
}


Elite::BehaviourState LeavePurgeZone(Elite::Blackboard* pBlackboard)
{
	if (gIsDebugging) std::cout << "LeavingPurge\n";

	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("m_pInterface", pInterface);

	if (!pPlugin || !pInterface) return Elite::BehaviourState::Failure;

	std::vector<EntityInfo> entities = pPlugin->GetEntitiesInFOV();
	for (const EntityInfo& entity : entities)
	{
		if (entity.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			pInterface->PurgeZone_GetInfo(entity, zoneInfo);

			Elite::Vector2 target = zoneInfo.Center;
			pPlugin->SetSteeringBehaviour(SteeringType::Flee);
			pPlugin->SetTarget(target);
			if (!pPlugin->IsRunningFromPurge()) pPlugin->SetRunningFromPurge(true);
		}
	}

	return Elite::BehaviourState::Success;
}

//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------
bool IsInsidePurgeZone(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);
	if (pPlugin)
	{
		if (pPlugin->IsRunningFromPurge()) return true;

		std::vector<EntityInfo> entities = pPlugin->GetEntitiesInFOV();
		for (const EntityInfo& entity : entities) 
		{
			if (entity.Type == eEntityType::PURGEZONE)
			{
				return true;
			}
		}
	}
	return false;
}
bool IsEnemyClose(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	if (pPlugin)
	{
		if (pPlugin->IsRunningAway()) return true;

		std::vector<EntityInfo> entities = pPlugin->GetEntitiesInFOV();
		for (const EntityInfo& entity : entities)
		{
			if (entity.Type == eEntityType::ENEMY)
			{
				return true;
			}
		}
	}
	return false;
}
bool CanShoot(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	if (pPlugin)
	{
		return pPlugin->HasGun();
	}
	return false;
}
bool HasBeenBitten(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);
	if (pInterface && pPlugin)
	{
		if (pPlugin->WasBitten()) return true;
		if (pInterface->Agent_GetInfo().WasBitten)
		{
			if (!pPlugin->WasBitten()) pPlugin->SetWasBitten(true);
			return true;
		}
	}
	return false;
}

bool IsItemClose(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);

	if (pPlugin)
	{
		std::vector<EntityInfo> entities = pPlugin->GetEntitiesInFOV();

		for (EntityInfo& entity : entities)
		{
			if (entity.Type == eEntityType::ITEM)
			{
				return true;
			}
		}
	}
	return false;
}

bool IsHouseClose(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	if (pPlugin)
	{
		if (pPlugin->IsWalkingToHouse()) return true;

		std::vector<HouseInfo> houses = pPlugin->GetHousesInFOV();

		for (const HouseInfo& house : houses)
		{
			if (!pPlugin->WasLastHouse(house)
				&& !pPlugin->HasVisitedLeftBottom())
			{
				return true;
			}
		}
	}
	return false;
}
bool IsInBottomLeftOfHouse(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);
	if (pPlugin && pInterface)
	{
		if (pPlugin->HasVisitedLeftBottom()) return true;

		AgentInfo agentInfo = pInterface->Agent_GetInfo();

		std::vector<HouseInfo> houses = pPlugin->GetHousesInFOV();

		for (const HouseInfo& house : houses) 
		{
			const float offset{ 5.f };
			Elite::Vector2 bottomLeft;
			bottomLeft = house.Center - Elite::Vector2((house.Size.x / 2.f) - offset, (house.Size.y / 2.f) - offset);

			const float maxDistance = 5.f;
			float distance = Elite::Distance(agentInfo.Position, bottomLeft);
			if (distance < maxDistance) return true;
		}
	}
	return false;
}
bool IsInTopRightOfHouse(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);
	if (pPlugin && pInterface)
	{
		AgentInfo agentInfo = pInterface->Agent_GetInfo();

		std::vector<HouseInfo> houses = pPlugin->GetHousesInFOV();

		for (const HouseInfo& house : houses)
		{
			const float offset{ 5.f };
			Elite::Vector2 topRight;
			topRight = house.Center + Elite::Vector2((house.Size.x / 2.f) - offset, (house.Size.y / 2.f) - offset);

			const float maxDistance = 5.f;
			float distance = Elite::Distance(agentInfo.Position, topRight);
			if (distance < maxDistance) return true;
		}
	}
	return false;
}
bool IsInsideHouse(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);
	if (pPlugin && pInterface)
	{
		AgentInfo agentInfo = pInterface->Agent_GetInfo();

		std::vector<HouseInfo> houses = pPlugin->GetHousesInFOV();

		for (const HouseInfo& house : houses)
		{
			if (agentInfo.Position.x > (house.Center.x - house.Size.x / 2.f)
				&& agentInfo.Position.x <(house.Center.x + house.Size.x / 2.f)
				&& agentInfo.Position.y > (house.Center.y - house.Size.y / 2.f)
				&& agentInfo.Position.y <(house.Center.y + house.Size.y / 2.f))
			{
				if (pPlugin->WasLastHouse(house)) return true;
			}
		}
	}
	return false;
}

bool IsOutsideMap(Elite::Blackboard* pBlackboard)
{
	Plugin* pPlugin{ nullptr };
	IExamInterface* pInterface{ nullptr };
	pBlackboard->GetData("pPlugin", pPlugin);
	pBlackboard->GetData("m_pInterface", pInterface);
	if (pInterface && pPlugin)
	{
		if (pPlugin->IsGoingToCentre()) return true;

		AgentInfo agentInfo = pInterface->Agent_GetInfo();
		if (agentInfo.Position.x < -200.f
			|| agentInfo.Position.x > 200.f
			|| agentInfo.Position.y < -200.f
			|| agentInfo.Position.y > 200.f)
			return true;
	}
	return false;
}

#endif