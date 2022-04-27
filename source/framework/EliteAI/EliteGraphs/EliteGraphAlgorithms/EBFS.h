#pragma once

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);
	private:
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template <class T_NodeType, class T_ConnectionType>
	BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
	{

		// Nodes we still have to check
		std::queue<T_NodeType*> openList; 
		// Already checked nodes AND history of nodes
		std::map<T_NodeType*, T_NodeType*> closedList; 

		// Add start node
		openList.push(pStartNode); 

		while (!openList.empty()) 
		{
			// Taking a node from the openlist
			T_NodeType* pCurrentNode = openList.front();
			// And remove this node from the queue
			openList.pop();

			// If endnode was found, stop
			if (pCurrentNode == pDestinationNode) break;

			for (auto con : m_pGraph->GetNodeConnections(pCurrentNode)) 
			{
				T_NodeType* pNextNode = m_pGraph->GetNode(con->GetTo());
				if (closedList.find(pNextNode) == closedList.end())
				{
					// We did not find this node in the closedlist
					openList.push(pNextNode);
					closedList[pNextNode] = pCurrentNode;
				}
			}
		}

		// Track back
		vector<T_NodeType*> path;

		T_NodeType* pCurrNode = pDestinationNode;

		while (pCurrNode != pStartNode) 
		{
			path.push_back(pCurrNode);
			pCurrNode = closedList[pCurrNode];
		}

		path.push_back(pStartNode);
		// Reverse path
		std::reverse(path.begin(), path.end());

		return path;
	}
}

