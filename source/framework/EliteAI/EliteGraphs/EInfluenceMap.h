#pragma once
#include "EIGraph.h"
#include "EGraphNodeTypes.h"
#include "EGraphConnectionTypes.h"

namespace Elite
{
	template<class T_GraphType>
	class InfluenceMap final : public T_GraphType
	{
	public:
		InfluenceMap(bool isDirectional) : T_GraphType(isDirectional) {}
		void InitializeBuffer() { m_InfluenceDoubleBuffer = vector<float>(m_Nodes.size()); }
		void PropagateInfluence(float deltaTime);

		void SetInfluenceAtPosition(Elite::Vector2 pos, float influence);
		void SetInfluenceAtPosition(Elite::Vector2 pos, float influence, float radius, int cellSize);

		void Render() const {}
		void SetNodeColorsBasedOnInfluence();

		float GetMomentum() const { return m_Momentum; }
		void SetMomentum(float momentum) { m_Momentum = momentum; }

		float GetDecay() const { return m_Decay; }
		void SetDecay(float decay) { m_Decay = decay; }

		float GetPropagationInterval() const { return m_PropagationInterval; }
		void SetPropagationInterval(float propagationInterval) { m_PropagationInterval = propagationInterval; }

		float GetMaxAbsInfluence() const { return m_MaxAbsInfluence; }

	protected:
		virtual void OnGraphModified(bool nrOfNodesChanged, bool nrOfConnectionsChanged) override;

	private:
		Elite::Color m_NegativeColor{ 1.f, 0.2f, 0.f };
		Elite::Color m_NeutralColor{ 0.f, 0.f, 0.f };
		Elite::Color m_PositiveColor{ 0.f, 0.2f, 1.f };

		float m_MaxAbsInfluence = 100.f;

		float m_Momentum = 0.3f; // a higher momentum means a higher tendency to retain the current influence
		float m_Decay = 0.5f; // determines the decay in influence over distance

		float m_PropagationInterval = .05f; //in Seconds
		float m_TimeSinceLastPropagation = 0.0f;

		vector<float> m_InfluenceDoubleBuffer;
	};

	template <class T_GraphType>
	void InfluenceMap<T_GraphType>::PropagateInfluence(float deltaTime)
	{

		// For some reason "newinfluence" keeps being 0 here, and I'm not sure why...
		// TODO: Fix
		for (auto pNode : m_Nodes) 
		{
			float newInfluence{ 0.0f };
			for (auto pConnection : GetNodeConnections(pNode)) 
			{
				newInfluence = GetNode(pConnection->GetTo())->GetInfluence() * expf(-pConnection->GetCost() * m_Decay);
			}
			m_InfluenceDoubleBuffer[pNode->GetIndex()] = Lerp(pNode->GetInfluence(), newInfluence, m_Momentum);
		}


		for (auto pNode : m_Nodes)
			pNode->SetInfluence(m_InfluenceDoubleBuffer[pNode->GetIndex()]);
	}

	template <class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetInfluenceAtPosition(Elite::Vector2 pos, float influence)
	{
		auto idx = GetNodeIdxAtWorldPos(pos);
		if (IsNodeValid(idx))
			GetNode(idx)->SetInfluence(influence);
	}

	template<class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetInfluenceAtPosition(Elite::Vector2 pos, float influence, float radius, int cellSize)
	{
		//BoundingBox
		Vector2 startPos{ pos.x - radius, pos.y - radius };
		Vector2 endPos{ pos.x + radius, pos.y + radius };
		float radiusSquared{ radius * radius };

		for (float x = startPos.x; x <= endPos.x; x += float(cellSize))
		{
			for (float y = startPos.y; y <= endPos.y; y += float(cellSize))
			{
				if (Elite::DistanceSquared(Vector2{ x, y }, pos) > radiusSquared) continue; //Cell out of radius

				auto idx = GetNodeIdxAtWorldPos(pos);
				if (!IsNodeValid(idx)) continue; //Inavlid index

				GetNode(idx)->SetInfluence(influence);
			}
		}
	}

	template<class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetNodeColorsBasedOnInfluence()
	{
		const float half = .5f;

		for (auto& pNode : m_Nodes)
		{
			Color nodeColor{};
			float influence = pNode->GetInfluence();
			float relativeInfluence = abs(influence) / m_MaxAbsInfluence;

			if (influence < 0)
			{
				nodeColor = Elite::Color{
				Lerp(m_NeutralColor.r, m_NegativeColor.r, relativeInfluence),
				Lerp(m_NeutralColor.g, m_NegativeColor.g, relativeInfluence),
				Lerp(m_NeutralColor.b, m_NegativeColor.b, relativeInfluence)
				};
			}
			else
			{
				nodeColor = Elite::Color{
				Lerp(m_NeutralColor.r, m_PositiveColor.r, relativeInfluence),
				Lerp(m_NeutralColor.g, m_PositiveColor.g, relativeInfluence),
				Lerp(m_NeutralColor.b, m_PositiveColor.b, relativeInfluence)
				};
			}

			pNode->SetColor(nodeColor);
		}
	}

	template<class T_GraphType>
	inline void InfluenceMap<T_GraphType>::OnGraphModified(bool nrOfNodesChanged, bool nrOfConnectionsChanged)
	{
		InitializeBuffer();
	}
}