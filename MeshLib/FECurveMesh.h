#pragma once
#include <MathLib/math3d.h>
#include "FENode.h"
#include "FEElement.h"
#include "FELineMesh.h"
#include "box.h"
#include <MeshTools/Serializable.h>

// A class that represents a mesh for a curve
// This mesh only consists of nodes and edges
class FECurveMesh : public FELineMesh
{
public:
	// curve types
	enum CurveType
	{
		INVALID_CURVE,		// there is an error in this mesh
		EMPTY_CURVE,		// an empty curve (no nodes or edges)
		SIMPLE_CURVE,		// a single curve with two end-points
		CLOSED_CURVE,		// a single closed loop
		COMPLEX_CURVE,		// anything else
	};

public:
	// constructor
	FECurveMesh();

	// allocate storage for mesh
	void Create(int nodes, int edges);

	// return the type of the curve
	CurveType Type() const;

	// clear curve data
	void Clear();

	void UpdateMeshData() override;

public:
	// Add a node to the mesh. Returns the index of the newly added node
	// If the snap flag is on, then it will be checked if the point already exists in the mesh
	int AddNode(const vec3d& r, bool snap = true, double snapTolerance = 0.01);

	// adds an edge between nodes n0, n1
	// If the edge already exists, it will not be added
	// Returns the index of the new (or existing) edge
	// (or -1 if adding the edge has failed (e.g. if n0==n1))
	// Note that this will set the type to INVALID_CURVE
	// Call Update to reevaluate the curve mesh
	int AddEdge(int n0, int n1);

	// Update the mesh
	void Update();

	// tag all the nodes of this mesh
	void TagAllNodes(int ntag);

	// tag all the edges of this mesh
	void TagAllEdges(int ntag);

	// sort the nodes and edges based on edge connectivity
	void Sort();

	// invert the order of the nodes
	void Invert();

	// reorder nodes based on look-up table
	void ReorderNodes(vector<int>& NLT);

	// reorder edges based on look-up table
	void ReorderEdges(vector<int>& ELT);

	// attach another curve to this one
	void Attach(const FECurveMesh& c);

	// get the bounding box of the mesh
	BOX BoundingBox() const;

	// returns the total lenght of all edge segments
	double Length() const;

	// count the number of end points
	int EndPoints() const;

	// return a list of end points
	vector<int> EndPointList() const;

	// count the curve segments (a segment is a simply-connected curve)
	int Segments() const;

	// remove a node (and connecting edges)
	void RemoveNode(int node);

	// Serialization
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	void FlipEdge(FEEdge& e);

	void UpdateEdgeNeighbors();
	void UpdateEdgeIDs();
	void UpdateNodeIDs();
	void UpdateType();

private:
	CurveType		m_type;		//!< curve type
};
