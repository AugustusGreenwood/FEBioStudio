#include "stdafx.h"
#include "LaplaceSolver.h"
#include <MeshLib/FEMesh.h>
#include <MeshLib/FENodeNodeList.h>
#include <MeshLib/FENodeElementList.h>
#include <MeshLib/MeshMetrics.h>

LaplaceSolver::LaplaceSolver()
{
}

// Solves the Laplace equation on the mesh.
// Input: val = initial values for all nodes
//        bn  = boundary flags: 0 = free, 1 = fixed
// Output: val = solution
bool LaplaceSolver::Solve(FEMesh* pm, vector<double>& val, vector<int>& bn)
{
	// convergence criteria
	const int MAX_ITER = 1000;	// max number of iterations
	const double eps = 0.0001;	// max change between iterations

	// make sure the value and flag arrays are of the correct size
	int NN = pm->Nodes();
	if ((int) val.size() != NN) return false;
	if ((int) bn.size() != NN) return false;

	// find the value range
	double vmin = 0.0, vmax = 0.0;
	bool binit = false;
	for (int i=0; i<NN; ++i)
	{
		if (bn[i])
		{
			if (binit)
			{ 
				if (val[i] < vmin) vmin = val[i];
				if (val[i] > vmax) vmax = val[i];
			}
			else
			{
				vmin = vmax = val[i]; 
				binit = true; 
			}
		}
	}

	// if min and max are the same, then the solution is trivial
	if (vmin == vmax)
	{
		for (int i=0; i<NN; ++i) val[i] = vmin;
		return true;
	}

	// calculate the average value
	// we'll use this to initialize the free nodes
	double vavg = 0.5*(vmin + vmax);
	for (int i=0; i<NN; ++i)
		if (bn[i] == 0) val[i] = vavg;

	// calculate the element volumes
	int NE = pm->Elements();
	vector<double> Ve(NE);
	for (int i=0; i<NE; ++i) Ve[i] = FEMeshMetrics::ElementVolume(*pm, pm->Element(i));

	// create Node-Node list
	FENodeNodeList NNL(pm);

	// create node-element list
	FENodeElementList NEL;
	NEL.Build(pm);

	// we'll assign values to the edges and the nodes
	NNL.InitValues(0.0);
	vector<double> D(NN, 0.0);

	// build the diagonal terms
	for (int i=0; i<NN; ++i)
	{
		if (bn[i] == 0)
		{
			int eval = NEL.Valence(i);
			for (int j=0; j<eval; ++j)
			{
				int iel = NEL.ElementIndex(i, j);
				FEElement_& ej = *NEL.Element(i, j);

				int na = ej.FindNodeIndex(i);
				assert (na != -1);

				double Vj = Ve[iel];
				int nj = ej.Nodes();
				double dot = 0.0;
				for (int k=0; k<nj; ++k)
				{
					vec3d Ga = FEMeshMetrics::ShapeGradient(*pm, ej, na, k);
					dot += Ga*Ga;
				}
				dot *= Vj / nj;
			
				D[i] += dot;
			}
		}
	}

	// build the edge weights
	for (int i=0; i<NN; ++i)
	{
		int nval = NNL.Valence(i);
		for (int j=0; j<nval; ++j)
		{
			int ni = i;
			int nj = NNL.Node(i, j);

			// we must loop over the union of Sa and Sb
			int nei = NEL.Valence(ni);

			double Kij = 0.0;

			for (int k=0; k<nei; ++k)
			{
				int kel = NEL.ElementIndex(ni, k);
				if (NEL.HasElement(nj, kel))
				{
					FEElement_& ek = *NEL.Element(ni, k);
					int na = ek.FindNodeIndex(ni); assert(na != -1);
					int nb = ek.FindNodeIndex(nj); assert(nb != -1);

					double Vk = Ve[kel];
					int nk = ek.Nodes();

					double dot = 0.0;
					for (int k=0; k<nk; ++k)
					{
						vec3d Ga = FEMeshMetrics::ShapeGradient(*pm, ek, na, k);
						vec3d Gb = FEMeshMetrics::ShapeGradient(*pm, ek, nb, k);
						dot += Ga*Gb;
					}
					dot *= Vk / nk;
			
					Kij += dot;
				}
			}

			NNL.Value(i, j) = Kij;
		}
	}

	// inverted diagonal values
	vector<double> Dinv(NN);
	for (int i=0; i<NN; ++i) Dinv[i] = 1.0 / D[i];

	// start the iterations
	int niter = 0;
	double maxd;
	do
	{
		maxd = 0.0;
		for (int i=0; i<NN; ++i)
		{
			if (bn[i] == 0)
			{
				double sum = 0;
				int nval = NNL.Valence(i);
				for (int k=0; k<nval; ++k) sum -= val[NNL.Node(i, k)]*NNL.Value(i,k);
				sum *= Dinv[i];

				double dv = fabs(val[i] - sum);
				val[i] = sum;

				if (dv > maxd) maxd = dv;
			}
		}
		niter++;
	}
	while ((niter < MAX_ITER)&&(maxd > eps));

	return true;
}
