#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"

#include "EBehaviourTree.h"
#include "Behaviours.h"


using namespace Elite;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "ExamBot";
	info.Student_FirstName = "Sasha";
	info.Student_LastName = "Vigneron";
	info.Student_Class = "2DAE07";

	Blackboard* pBlackboard = CreateBlackboard();

	// SteeringBehaviours
	m_pSeekBehaviour = new Seek();
	m_pFleeBehaviour = new Flee(pBlackboard);
	m_pFaceBehaviour = new FaceFlee();
	m_pWanderBehaviour = new Wander(pBlackboard);
	m_pEvadeBehaviour = new Evade(pBlackboard);
	m_pSeekFleeBehaviour = new SeekTargetFleeEnemy();

	m_pActiveSteeringBehaviour = m_pWanderBehaviour;

	// Inventory
	for (size_t i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
	{
		m_InventorySlotsOccupied.push_back(false);
	}

	// Create BehaviourTree
	m_pBehaviourTree = new BehaviourTree{ pBlackboard,
												new BehaviourSelector
												{{

													

												// Attacking / Running
													new BehaviourSequence
													{{
														new BehaviourConditional(IsEnemyClose),
														new BehaviourConditional(CanShoot),
														new BehaviourAction(ShootEnemy)
													}},

													//Purge zone
													new BehaviourSequence
													{{
														new BehaviourConditional(IsInsidePurgeZone),
														new BehaviourAction(LeavePurgeZone)
													}},

													// Attacking / Running 2
													/*new BehaviourSequence
													{{
														new BehaviourConditional(IsEnemyClose),
														new BehaviourAction(RunAwayFromEnemy)
													}},*/
													new BehaviourSequence
													{{
														new BehaviourConditional(HasBeenBitten),
														new BehaviourConditional(CanShoot),
														new BehaviourAction(LookBehind)
													}},
													new BehaviourSequence
													{{
														new BehaviourConditional(HasBeenBitten),
														new BehaviourAction(StartRunning)
													}},

														// Looting
													new BehaviourSequence
													{{
														new BehaviourConditional(IsItemClose),
														new BehaviourAction(PickupClosestItem)
													}},

													new BehaviourSequence
													{{
														new BehaviourConditional(IsInsideHouse),
														new BehaviourAction(FleeHouse)
													}},
													new BehaviourSequence
													{{
														new BehaviourConditional(IsInTopRightOfHouse),
														new BehaviourAction(SetHouseLooted)
													}},
													new BehaviourSequence
													{{
														new BehaviourConditional(IsInBottomLeftOfHouse),
														new BehaviourAction(GoToTopRightOfHouse)
													}},
													
													new BehaviourSequence
													{{
														new BehaviourConditional(IsHouseClose),
														new BehaviourAction(GoToBottomLeftOfHouse)
													}},

														// General
													new BehaviourSequence
													{{
														new BehaviourConditional(IsOutsideMap),
														new BehaviourAction(GoToCenterOfMap)
													}},
													new BehaviourAction(ChangeToWander)
												}}
										};


}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called when the plugin gets unloaded
	if(m_pBehaviourTree) delete m_pBehaviourTree;
	delete m_pSeekBehaviour;
	delete m_pFleeBehaviour;
	delete m_pFaceBehaviour;
	delete m_pWanderBehaviour;
	delete m_pEvadeBehaviour;
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = false; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	#pragma region Initialization
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)
	auto agentInfo = m_pInterface->Agent_GetInfo();

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
			std::cout << "Purge Zone in FOV:" << e.Location.x << ", " << e.Location.y << " ---EntityHash: " << e.EntityHash << "---Radius: " << zoneInfo.Radius << std::endl;
		}
	}
	#pragma endregion

	#pragma region Inventory

	CheckAndUseMedkit();
	CheckAndUseFood();

	#pragma endregion

	#pragma region DecisionMaking

	m_pBehaviourTree->Update(dt);

	#pragma endregion

	#pragma region Movement

	// Running Timer
	if (m_IsRunningAway)
	{
		m_CanRun = true;
		m_AccRunningAwayTime += dt;

		if (m_AccRunningAwayTime >= m_RunningAwayTime)
		{
			m_AccRunningAwayTime = 0.f;
			m_CanRun = false;
			m_IsRunningAway = false;
		}
	}

	// House Timer
	if (m_IsWalkingToHouse) 
	{
		m_Target = m_TargetHouseCenter;

		m_AccWalkingToHouseTime += dt;
		if (m_AccWalkingToHouseTime >= m_WalkingToHouseTime) 
		{
			m_IsWalkingToHouse = false;
			m_AccWalkingToHouseTime = 0.0f;
		}
	}

	// Purge Timer
	if (m_IsRunningFromPurge)
	{
		m_AccRunningFromPurgeTime += dt;
		if (m_AccRunningFromPurgeTime >= m_RunningFromPurgeTime)
		{
			m_IsRunningFromPurge = false;
			m_AccRunningFromPurgeTime = 0.0f;
		}
	}
	// Bitten Timer
	if (m_WasBitten)
	{
		m_AccBittenTime += dt;

		if (m_AccBittenTime >= m_BittenTime)
		{
			m_AccBittenTime = 0.f;
			m_WasBitten = false;
		}
	}

	// Centre Timer
	if (m_IsGoingToCentre)
	{
		m_AccGoingToCentreTime += dt;

		if (m_AccGoingToCentreTime >= m_GoingToCentreTime)
		{
			m_AccGoingToCentreTime = 0.f;
			m_IsGoingToCentre = false;
		}
	}
	
	if (agentInfo.Stamina > 5.0f)
	{
		m_CanRun = true;
	}
	else if(!m_WasBitten && !m_IsRunningAway)
		m_CanRun = false;

	m_pActiveSteeringBehaviour->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(m_pActiveSteeringBehaviour->GetTarget()));

	auto extendedSteering = m_pActiveSteeringBehaviour->CalculateSteering(dt, agentInfo);
	extendedSteering.RunMode = m_CanRun;

	#pragma endregion

	//Reset States
	m_GrabItem = false;
	m_UseItem = false;
	m_RemoveItem = false;

	//Slice Steering
	auto steering = SteeringPlugin_Output();
	steering.AngularVelocity = extendedSteering.AngularVelocity;
	steering.LinearVelocity = extendedSteering.LinearVelocity;
	steering.AutoOrient = extendedSteering.AutoOrient;
	steering.RunMode = extendedSteering.RunMode;

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

void Plugin::SetTarget(const Elite::Vector2& target)
{
	m_Target = target;
	m_pActiveSteeringBehaviour->SetTarget(m_Target);
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}
vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

Elite::Blackboard* Plugin::CreateBlackboard()
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("pPlugin", this);
	pBlackboard->AddData("m_pInterface", m_pInterface);

	pBlackboard->AddData("lastHousePos", Vector2(0,0));

	return pBlackboard;
}


bool Plugin::HasGun() const
{
	ItemInfo currItem;
	for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
	{
		// If slot is full
		if (m_InventorySlotsOccupied[i])
		{
			if (m_pInterface->Inventory_GetItem(i, currItem))
			{
				if (currItem.Type == eItemType::PISTOL) return true;
			}
		}
	}
	return false;
}

void Plugin::CheckAndUseMedkit()
{
	AgentInfo agentInfo = m_pInterface->Agent_GetInfo();
	int maxHealth = 10;
	int requiredHealth = int(maxHealth - agentInfo.Health);

	ItemInfo currItem;
	for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
	{
		// If slot is occupied
		if (m_InventorySlotsOccupied[i])
		{
			if (m_pInterface->Inventory_GetItem(i, currItem))
			{
				if (currItem.Type == eItemType::MEDKIT)
				{

					// Check health
					if (m_pInterface->Inventory_GetItem(i, currItem)
						&& m_pInterface->Medkit_GetHealth(currItem) <= requiredHealth)
					{
						// Use medkit item
						m_pInterface->Inventory_UseItem(i);
						m_pInterface->Inventory_RemoveItem(i);
						m_InventorySlotsOccupied[i] = false;
						break;
					}

				}
			}
		}
	}
}
void Plugin::CheckAndUseFood()
{
	AgentInfo agentInfo = m_pInterface->Agent_GetInfo();
	int maxEnergy = 10;
	int requiredEnergy = int(maxEnergy - agentInfo.Energy);

	ItemInfo currItem;
	for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
	{
		// If slot is occupied
		if (m_InventorySlotsOccupied[i])
		{
			if (m_pInterface->Inventory_GetItem(i, currItem))
			{
				if (currItem.Type == eItemType::FOOD)
				{
					// Check food
					if (m_pInterface->Inventory_GetItem(i, currItem)
						&& m_pInterface->Food_GetEnergy(currItem) <= requiredEnergy)
					{

						// Use food item
						m_pInterface->Inventory_UseItem(i);
						m_pInterface->Inventory_RemoveItem(i);
						m_InventorySlotsOccupied[i] = false;
						break;
					}

				}
			}
		}
	}
}

void Plugin::SetSteeringBehaviour(const SteeringType& steeringType)
{
	Evade* evadeBehaviour = nullptr;
	SeekTargetFleeEnemy* seekFleeBehaviour = nullptr;
	EnemyInfo info;
	switch (steeringType)
	{
	case SteeringType::Seek:
		m_pActiveSteeringBehaviour = m_pSeekBehaviour;
		break;
	case SteeringType::Flee:
		m_pActiveSteeringBehaviour = m_pFleeBehaviour;
		break;
	case SteeringType::Wander:
		m_pActiveSteeringBehaviour = m_pWanderBehaviour;
		break;
	case SteeringType::FaceFlee:
		m_pActiveSteeringBehaviour = m_pFaceBehaviour;
		break;
	case SteeringType::Evade:
		m_pActiveSteeringBehaviour = m_pEvadeBehaviour;
		evadeBehaviour = static_cast<Evade*>(m_pEvadeBehaviour);
		if (evadeBehaviour)
		{
			info = GetClosestEnemyInfo();
			if (info.Type != eEnemyType::DEFAULT)
			{
				evadeBehaviour->SetEnemy(info);
			}
		}
		break;
	case SteeringType::SeekFlee:
		m_pActiveSteeringBehaviour = m_pSeekFleeBehaviour;
		seekFleeBehaviour = static_cast<SeekTargetFleeEnemy*>(m_pSeekFleeBehaviour);
		if (seekFleeBehaviour)
		{
			info = GetClosestEnemyInfo();
			if (info.Type != eEnemyType::DEFAULT)
			{
				seekFleeBehaviour->SetFleeTarget(info.Location);
			}
		}
		break;


	default:
		std::cerr << "[Plugin.cpp(SetSteeringBehaviour)] Unknown SteeringType Requested.\n";
		break;
	}
}

EnemyInfo Plugin::GetClosestEnemyInfo()
{
	std::vector<EntityInfo> entities = GetEntitiesInFOV();

	EntityInfo* closestEnemy = nullptr;
	float closestEnemyDistance = FLT_MAX;

	for (EntityInfo& entity : entities)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			float currDistance = Elite::Distance(m_pInterface->Agent_GetInfo().Position, entity.Location);
			if (currDistance < closestEnemyDistance)
			{
				closestEnemy = &entity;
				closestEnemyDistance = currDistance;
			}
		}
	}

	if (!closestEnemy) return EnemyInfo();
	else
	{
		EnemyInfo enemyInfo = EnemyInfo();
		m_pInterface->Enemy_GetInfo(*closestEnemy, enemyInfo);
		return enemyInfo;
	}
}

void Plugin::TryAndPickupItem()
{
	// Check if has space in inventory
	bool hasSpaceInInventory = false;

	for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
	{
		if (!m_InventorySlotsOccupied[i])
		{
			hasSpaceInInventory = true;
			break;
		}
	}

	// Make space in inventory
	if (!hasSpaceInInventory)
	{
		// Find Most Frequent Item In Inventory
		int nrOfFoodItems = GetNrOfItemsInInventory(eItemType::FOOD);
		int nrOfMedkitItems = GetNrOfItemsInInventory(eItemType::MEDKIT);
		int nrOfPistols = GetNrOfItemsInInventory(eItemType::PISTOL);

		int max = std::max<int>(nrOfFoodItems, std::max<int>(nrOfMedkitItems, nrOfPistols));
		if (max == nrOfFoodItems) 
		{
			// eat food item with least energy
			int leastEnergySlot = 0;
			int leastEnergy = INT_MAX;

			for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
			{
				if (m_InventorySlotsOccupied[i])
				{
					ItemInfo item;
					m_pInterface->Inventory_GetItem(i, item);
					if (item.Type == eItemType::FOOD)
					{
						int currEnergy = m_pInterface->Food_GetEnergy(item);
						if (currEnergy < leastEnergy)
						{
							leastEnergy = currEnergy;
							leastEnergySlot = i;
						}
					}
				}
			}

			// Use this item
			m_pInterface->Inventory_UseItem(leastEnergySlot);
			m_InventorySlotsOccupied[leastEnergySlot] = false;
		}
		else if (max == nrOfMedkitItems)
		{
			// Use medkit with least health
			int leastHealthSlot = 0;
			int leastHealth = INT_MAX;

			for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
			{
				if (m_InventorySlotsOccupied[i])
				{
					ItemInfo item;
					m_pInterface->Inventory_GetItem(i, item);
					if (item.Type == eItemType::MEDKIT)
					{
						int currHealth = m_pInterface->Medkit_GetHealth(item);
						if (currHealth < leastHealth)
						{
							leastHealth = currHealth;
							leastHealthSlot = i;
						}
					}
				}
			}

			// Use this item
			m_pInterface->Inventory_UseItem(leastHealthSlot);
			m_InventorySlotsOccupied[leastHealthSlot] = false;
		}
		else 
		{
			// Pistol is most frequent
			int leastAmmoSlot = 0;
			int leastAmmo = INT_MAX;

			for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
			{
				if (m_InventorySlotsOccupied[i])
				{
					ItemInfo item;
					m_pInterface->Inventory_GetItem(i, item);
					if (item.Type == eItemType::PISTOL)
					{
						int currAmmo = m_pInterface->Weapon_GetAmmo(item);
						if (currAmmo < leastAmmo)
						{
							leastAmmo = currAmmo;
							leastAmmoSlot = i;
						}
					}
				}
			}

			// Drop this item
			m_pInterface->Inventory_RemoveItem(leastAmmoSlot);
			m_InventorySlotsOccupied[leastAmmoSlot] = false;
		}

		hasSpaceInInventory = true;
	}
	
	// Get Closest ItemInfo
	vector<EntityInfo> entities = GetEntitiesInFOV();

	EntityInfo* closestItem = nullptr;
	float closestItemDistance = 2.0f; // Pickup range

	for (EntityInfo& entity : entities)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			float currDistance = Elite::Distance(m_pInterface->Agent_GetInfo().Position, entity.Location);
			if (currDistance < closestItemDistance)
			{
				closestItem = &entity;
				closestItemDistance = currDistance;
			}
		}
	}

	// Add To Inventory
	if (closestItem)
	{
		ItemInfo item;
		m_pInterface->Item_GetInfo(*closestItem, item);

		if (m_pInterface->Item_Grab(*closestItem, item))
		{
			for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
			{
				// If slot is already full
				if (!m_InventorySlotsOccupied[i])
				{
					// Add item to this slot
					m_pInterface->Inventory_AddItem(i, item);
					m_InventorySlotsOccupied[i] = true;

					if (item.Type == eItemType::GARBAGE)
					{
						// Remove Item
						m_pInterface->Inventory_RemoveItem(i);
						m_InventorySlotsOccupied[i] = false;
					}
					return;
				}
			}
		}
	}
}
void Plugin::TryAndShootEnemy(const Elite::Vector2& target)
{
	AgentInfo agentInfo = m_pInterface->Agent_GetInfo();

	//ToDo: Shoot Enemy
	Elite::Vector2 toTarget = target - agentInfo.Position;
	float angle = std::atan2f(toTarget.y, toTarget.x);
	float currAngle = agentInfo.Orientation;

	angle += b2_pi / 2; // Offset
	if (angle > b2_pi) angle = -b2_pi + (angle - b2_pi);

	float diffAngle = angle - currAngle;
	if (diffAngle > b2_pi) diffAngle = -b2_pi + (diffAngle - b2_pi);
	if (diffAngle < -b2_pi) diffAngle = b2_pi - (diffAngle - b2_pi);

	// Variables 
	float rangeToStopChecking{ 0.06f };

	if (diffAngle < rangeToStopChecking && diffAngle > -rangeToStopChecking)
	{
		// I know it has gun, because of the Behaviour
		// Get Gun Slot
		ItemInfo currItem;
		for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
		{
			// If slot is occupied
			if (m_InventorySlotsOccupied[i])
			{
				if (m_pInterface->Inventory_GetItem(i, currItem))
				{
					if (currItem.Type == eItemType::PISTOL)
					{
						// Use gun item
						m_pInterface->Inventory_UseItem(i);

						// Check magazine
						if (m_pInterface->Inventory_GetItem(i, currItem)
							&& m_pInterface->Weapon_GetAmmo(currItem) <= 0)
						{
							m_pInterface->Inventory_RemoveItem(i);
							m_InventorySlotsOccupied[i] = false;
						}

					}
				}
			}
		}
	}
}
int Plugin::GetNrOfItemsInInventory(eItemType itemType) 
{
	int amountOfItemsHeld{};
	for (int i = 0; i < (int)m_pInterface->Inventory_GetCapacity(); i++)
	{
		if (m_InventorySlotsOccupied[i]) 
		{
			ItemInfo itemInfo;
			m_pInterface->Inventory_GetItem(i, itemInfo);
			if (itemInfo.Type == itemType) amountOfItemsHeld++;
		}
	}
	return amountOfItemsHeld;
}
