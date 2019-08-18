//
//  FECreateShells.cpp
//  MeshTools
//
//  Created by Martin Xiberras on 5/31/18.


#include "stdafx.h"
#include "FECreateShells.h"
#include <GeomLib/GMeshObject.h>


FECreateShells::FECreateShells() : FEModifier("Create Shells")
{
    AddDoubleParam(0, "h", "h");
}


FEMesh* FECreateShells::Apply(FEGroup* pg)
{
    if (pg->Type() != FE_SURFACE)
    {
        FEModifier::SetError("Invalid selection");
        return 0;
    }
    
    if (pg->size() == 0)
    {
        FEModifier::SetError("Empty selection");
        return 0;
    }
    
    vector<int> faceList;
    for (list<int>::iterator it = pg->begin(); it != pg->end(); ++it)
    {
        faceList.push_back(*it);
    }
    
    FEMesh* pm = pg->GetMesh();
    FEMesh* pnm = new FEMesh(*pm);
    CreateShells(pnm, faceList);
    
    return pnm;
}



FEMesh* FECreateShells::Apply(FEMesh* pm)
{
    vector<int> faceList;
    for (int i=0; i<pm->Faces(); ++i)
    {
        if (pm->Face(i).IsSelected()) faceList.push_back(i);
    }
    
    FEMesh* pnm = new FEMesh(*pm);
    CreateShells(pnm, faceList);
    
    return pnm;
}



void FECreateShells::CreateShells(FEMesh* pm, vector<int>& faceList){
    
    //get the user value
    double thick = GetFloatValue(0);
    
    //we count the nbr of selected faces and that they are tri or quad
    int faces = 0;
    for (int i=0; i < (int)faceList.size(); ++i)
    {
        FEFace& face = pm->Face(faceList[i]);
        int n = face.Nodes();
        // verification
        if ((n != 4) && (n != 3) && (n != 9) && (n != 8) && (n != 6)) return;
        
        for (int j = 0; j<n; ++j) pm->Node(face.n[j]).m_ntag = 1;
        ++faces;
    }
    
    //error message when no face is selected
    if (faces==0){
        FEModifier::SetError("Empty Selection");
        return ;
    }
    
    // get the largest element group number
    int nid = 0;
    for (int i = 0; i<pm->Elements(); ++i)
    {
        FEElement& el = pm->Element(i);
        if (el.m_gid > nid) nid = el.m_gid;
    }
    nid++;
    
    
    // create the new shell elements
    int nbrelem = pm->Elements();
    pm->Create(0, nbrelem + faces);
    
    int n=nbrelem;
    for (int i=0; i <(int)faceList.size(); ++i){
        FEElement& pe=pm->Element(n);
        FEFace& face = pm->Face(faceList[i]);
        int nf = face.Nodes();
        switch (nf)
        {
            case 3: pe.SetType(FE_TRI3 ); break;
            case 4: pe.SetType(FE_QUAD4); break;
            case 6: pe.SetType(FE_TRI6 ); break;
            case 7: pe.SetType(FE_TRI7 ); break;
            case 8: pe.SetType(FE_QUAD8); break;
            case 9: pe.SetType(FE_QUAD9); break;
            default:
                assert(false);
                FEModifier::SetError("Failure");
                return ;
        }
        for (int j=0; j<nf; ++j) pe.m_node[j] = face.n[j];
        pe.m_gid = nid;
        double* h = pe.m_h;
        for (int j=0; j<pe.Nodes(); ++j) h[j] = thick;
        n++;
    }
    pm->RebuildMesh();
}
