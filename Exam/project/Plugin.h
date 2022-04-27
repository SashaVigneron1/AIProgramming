#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

#include "EDecisionMaking.h"
#include "EBlackboard.h"

#include "SteeringBehaviours.h"


class IBaseInterface;
class IExamInterface;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

	// Accessors
	void SetTarget(const Elite::Vector2& target);
	Elite::Vector2 GetTarget() const { return m_Target; }

	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;

	void TryAndPickupItem();
	void TryAndShootEnemy(const Elite::Vector2& enemyLocation);

	bool HasGun() const;

	void SetSteeringBehaviour(const SteeringType& steeringType);

	void SetRunningAway(bool value) { m_IsRunningAway = value; m_AccRunningAwayTime = 0.0f; }
	bool IsRunningAway() const { return m_IsRunningAway; }
	void SetRunningFromPurge(bool value) { m_IsRunningFromPurge = value; m_AccRunningFromPurgeTime = 0.0f; }
	bool IsRunningFromPurge() const { return m_IsRunningFromPurge; }
	void SetWasBitten(bool value) { m_WasBitten = value; m_AccBittenTime = 0.0f; }
	bool WasBitten() const { return m_WasBitten; }
	void SetIsWalkingToHouse(bool value) { m_IsWalkingToHouse = value; m_AccWalkingToHouseTime = 0.0f; }
	bool IsWalkingToHouse() const { return m_IsWalkingToHouse; }
	void SetTargetHouse(Elite::Vector2 target) { m_TargetHouseCenter = target; }
	void SetIsGoingToCentre(bool value) { m_IsGoingToCentre = value; m_AccGoingToCentreTime = 0.0f; }
	bool IsGoingToCentre() const { return m_IsGoingToCentre; }

	void SetHasVisitedLeftBottom(bool value) { m_HasVisitedLeftBottom = value; }
	bool HasVisitedLeftBottom() const { return m_HasVisitedLeftBottom; }

	bool WasLastHouse(const HouseInfo& house) const { return (m_LastHouse.Center == house.Center); }
	void SetLastHouse(const HouseInfo& house) { m_LastHouse = house; }
	Elite::Vector2 GetLastHouseCenter() const { return m_LastHouse.Center; }

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;

	EnemyInfo GetClosestEnemyInfo();

	void CheckAndUseMedkit();
	void CheckAndUseFood();

	Elite::Blackboard* CreateBlackboard();

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	bool m_IsRunningAway = false;
	const float m_RunningAwayTime = 2.f;
	float m_AccRunningAwayTime = 0.0f;

	bool m_IsRunningFromPurge = false;
	const float m_RunningFromPurgeTime = 2.f;
	float m_AccRunningFromPurgeTime = 0.0f;

	bool m_IsWalkingToHouse = false;
	const float m_WalkingToHouseTime = 2.f;
	float m_AccWalkingToHouseTime = 0.0f;
	Elite::Vector2 m_TargetHouseCenter = Elite::Vector2(0,0);

	bool m_WasBitten = false;
	const float m_BittenTime = 3.f;
	float m_AccBittenTime = 0.0f;

	bool m_IsGoingToCentre = false;
	const float m_GoingToCentreTime = 15.f;
	float m_AccGoingToCentreTime = 0.0f;

	bool m_HasVisitedLeftBottom = false;


	// SteeringBehaviours
	ISteeringBehaviour* m_pActiveSteeringBehaviour = nullptr;
	ISteeringBehaviour* m_pSeekBehaviour = nullptr;
	ISteeringBehaviour* m_pFleeBehaviour = nullptr;
	ISteeringBehaviour* m_pFaceBehaviour = nullptr;
	ISteeringBehaviour* m_pWanderBehaviour = nullptr;
	ISteeringBehaviour* m_pEvadeBehaviour = nullptr;
	ISteeringBehaviour* m_pSeekFleeBehaviour = nullptr;

	Elite::BehaviourTree* m_pBehaviourTree = nullptr;

	HouseInfo m_LastHouse;

	// Inventory
	std::vector<bool> m_InventorySlotsOccupied;
	int GetNrOfItemsInInventory(eItemType itemType);

};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}