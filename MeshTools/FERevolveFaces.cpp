#include "stdafx.h"
#include "FERevolveFaces.h"
#include <MeshLib/MeshMetrics.h>

FERevolveFaces::FERevolveFaces() : FEModifier("Revolve faces")
{
    AddVecParam(vec3d(0,0,1), "axis", "axis");
    AddVecParam(vec3d(0,0,0), "center", "center");
    AddDoubleParam(1.0, "angle", "angle");	// assumed in degrees
    AddIntParam(1, "segments", "segments");
}

inline bool pointOnAxis(const vec3d& c, const vec3d& n, const vec3d& p)
{
    vec3d q = p - c;
    q = q - n*(q*n);
    double L2 = q.SqrLength();
    return (L2 < 1e-10);
}

FEMesh* FERevolveFaces::Apply(FEMesh* pm)
{
    //if (pm->IsType(FE_QUAD4)) return RevolveShellMesh(pm);
    return RevolveSolidMesh(pm);
}

FEMesh* FERevolveFaces::RevolveSolidMesh(FEMesh* pm)
{
    bool isQuad = false;
    
    // check if mesh is quadratic
    int NE = pm->Elements();
    for (int i=0; i<NE; ++i)
    {
        const FEElement_& el = pm->ElementRef(i);
        if ((el.Type() == FE_HEX20) || (el.Type() == FE_PENTA15) || (el.Type() == FE_QUAD8) || (el.Type()==FE_TRI6))
        {
            isQuad = true;
            break;
        }
    }
    
    // find all nodes that need to be copied.
    pm->TagAllNodes(-1);
    int NF = pm->Faces();
    int ne1 = 0;
    for (int i=0; i<NF; ++i)
    {
        FEFace& f = pm->Face(i);
        
        if (f.IsSelected())
        {
            int nf = f.Nodes();
            for (int j=0; j<nf; ++j){
                pm->Node(f.n[j]).m_ntag = 1;
            }
            ne1++;
            
        }
    }
    
    // get the parameters
    vec3d axis = GetVecValue(0); axis.Normalize();
    vec3d center = GetVecValue(1);
    double w = GetFloatValue(2) * DEG2RAD;
    int nseg = GetIntValue(3);
    if (nseg < 1) return 0;
    
   	// count the tagged nodes
    vector<int> nodeList;
    int NN0 = pm->Nodes();
    int nn = 0;
    
    for (int i=0; i<NN0; ++i)
    {
        FENode& node = pm->Node(i);
        if (node.m_ntag == 1)
        {
            // if a node lies on the axis of rotation we do not need to duplicate it
            vec3d r = pm->NodePosition(i);
            if (pointOnAxis(center, axis, r) == false)
            {
                double l = (r - center)*axis;
                node.m_ntag = nn++;
                nodeList.push_back(i);
                // add extra node for quadratic meshes
                if(isQuad){
                    nodeList.push_back(i);
                    nn++;
                }
            }
            else node.m_ntag = -2;
        }
        
    }
    assert(nn == (int) nodeList.size());
    if (nn == 0) return 0;
    
    // allocate new mesh
    int NN1 = NN0 + nn*nseg;
    FEMesh* pmnew = new FEMesh(*pm);
    pmnew->Create(NN1, 0);
    
    // loop over all tagged nodes
    for (int i = 0; i<nn; ++i)
    {
        // stores position of nodes on the same rotation arc (used for quadratic meshes)
        vector<vec3d> npos;
        
        // revolve the tagged nodes
        for (int l = 1; l <= nseg; ++l)
        {
            // setup rotation
            double wl = w * l / nseg;
            quatd Q(wl, axis);
            
            FENode& node = pmnew->Node(nodeList[i]);
            FENode& node2 = pmnew->Node(NN0 + (l - 1)*nn + node.m_ntag);
            
            vec3d r = pm->LocalToGlobal(node.r) - center;
            Q.RotateVector(r);
            node2.r = pm->GlobalToLocal(center + r);
            node2.m_ntag = node.m_ntag;
            
            npos.push_back(node2.r);
            
            if(isQuad){
                // create middle node for quadratic meshes
                FENode& node3 = pmnew->Node(1 + NN0 + (l - 1)*nn + node.m_ntag);
                
                if (l == 1) node3.r = pm->GlobalToLocal((node2.r + node.r) / 2.0);
                else node3.r = pm->GlobalToLocal((node2.r + npos[l - 2]) / 2.0);
                
                node3.m_ntag = node.m_ntag;
            }
        }
    }
    
    // create new elements
    int NE0 = pm->Elements();
    pmnew->Create(0, NE0 + nseg*ne1);
    
    // count element partitions
    int nid = pmnew->CountElementPartitions();
    
    int n = NE0;
    for (int l = 1; l <= nseg; ++l)
    {
        for (int i=0; i<pm->Faces(); ++i)
        {
            FEFace& face = pmnew->Face(i);
            if (pm->Face(i).IsSelected())
            {
                int nf = face.Nodes();
                FEElement& el = pmnew->Element(n);
                
                if(nf == 6)
                {
                    el.SetType(FE_PENTA15);
                    el.m_gid = nid;
                    
                    el.m_node[0] = face.n[0];
                    el.m_node[1] = face.n[1];
                    el.m_node[2] = face.n[2];
                    
                    el.m_node[3] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                    el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                    el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                    
                    el.m_node[6] = face.n[3];
                    el.m_node[7] = face.n[4];
                    el.m_node[8] = face.n[5];
                    
                    el.m_node[ 9] = NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                    el.m_node[10] = NN0 + (l - 1)*nn + pmnew->Node(face.n[4]).m_ntag;
                    el.m_node[11] = NN0 + (l - 1)*nn + pmnew->Node(face.n[5]).m_ntag;
                    
                    el.m_node[12] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                    el.m_node[13] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                    el.m_node[14] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                    
                    // move the face
                    face.n[0] = el.m_node[ 3];
                    face.n[1] = el.m_node[ 4];
                    face.n[2] = el.m_node[ 5];
                    
                    face.n[3] = el.m_node[ 9];
                    face.n[4] = el.m_node[10];
                    face.n[5] = el.m_node[11];
                    face.m_elem[0] = n;
                    face.m_elem[1] = -1;
                    ++n;
                    
                }
                else if(nf == 3)
                {
                    bool wedge = true;
                    int NOA = 0;
                    for (int j=0; j<nf; ++j)
                    {
                        if (pmnew->Node(face.n[j]).m_ntag == -2){
                            wedge = false;
                            NOA++;
                        }
                    }
                    
                    if(wedge == true){
                        el.SetType(FE_PENTA6);
                        el.m_gid = nid;
                        
                        el.m_node[0] = face.n[0];
                        el.m_node[1] = face.n[1];
                        el.m_node[2] = face.n[2];
                        
                        el.m_node[3] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                        el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                        el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                        
                        // move the face
                        face.n[0] = el.m_node[3];
                        face.n[1] = el.m_node[4];
                        face.n[2] = el.m_node[5];
                        face.m_elem[0] = n;
                        face.m_elem[1] = -1;
                        ++n;
                    }
                    else{
                        for(int j = 0; j < nf; j++){
                            int n0 = face.n[j];
                            int n1 = face.n[(j + 1) % nf];
                            int n2 = face.n[(j + 2) % nf];
                            if(NOA==1){
                                el.SetType(FE_PYRA5);
                                el.m_gid = nid;
                                
                                if (pmnew->Node(n0).m_ntag == -2){
                                    el.m_node[0] = NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[1] = n2;
                                    el.m_node[2] = n1;
                                    el.m_node[3] = NN0 + (l - 1)*nn + pmnew->Node(n1).m_ntag;
                                    el.m_node[4] = n0;
                                    
                                    // move the face
                                    face.n[(j + 2) % nf] = el.m_node[0];
                                    face.n[(j + 1) % nf] = el.m_node[3];
                                    
                                    break;
                                }
                                
                            }
                            else{
                                el.SetType(FE_TET4);
                                el.m_gid = nid;
                                
                                if ((pmnew->Node(n0).m_ntag == -2) && (pmnew->Node(n1).m_ntag == -2)){
                                    el.m_node[0] = n2;
                                    el.m_node[1] = NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[2] = n1;
                                    el.m_node[3] = n0;
                                    
                                    // move the face
                                    face.n[(j + 2) % nf] = el.m_node[1];
                                    break;
                                }
                            }
                        }
                        ++n;
                    }
                }
                
                else if (nf)
                {
                    // see if we need to make a hex or a wedge
                    bool bhex = true;
                    for (int j=0; j<nf; ++j) if (pmnew->Node(face.n[j]).m_ntag == -2) bhex = false;
                    
                    if (bhex)
                    {
                        if(isQuad){
                            el.SetType(FE_HEX20);
                            el.m_gid = nid;
                            
                            el.m_node[0] = face.n[0];
                            el.m_node[1] = face.n[1];
                            el.m_node[2] = face.n[2];
                            el.m_node[3] = face.n[3];
                            
                            el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                            el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                            el.m_node[6] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                            el.m_node[7] = NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                            
                            el.m_node[ 8] = face.n[4];
                            el.m_node[ 9] = face.n[5];
                            el.m_node[10] = face.n[6];
                            el.m_node[11] = face.n[7];
                            
                            el.m_node[12] = NN0 + (l - 1)*nn + pmnew->Node(face.n[4]).m_ntag;
                            el.m_node[13] = NN0 + (l - 1)*nn + pmnew->Node(face.n[5]).m_ntag;
                            el.m_node[14] = NN0 + (l - 1)*nn + pmnew->Node(face.n[6]).m_ntag;
                            el.m_node[15] = NN0 + (l - 1)*nn + pmnew->Node(face.n[7]).m_ntag;
                            
                            el.m_node[16] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                            el.m_node[17] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                            el.m_node[18] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                            el.m_node[19] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                            
                            // move part of face
                            face.n[4] = el.m_node[12];
                            face.n[5] = el.m_node[13];
                            face.n[6] = el.m_node[14];
                            face.n[7] = el.m_node[15];
                            
                        }
                        else{
                            el.SetType(FE_HEX8);
                            el.m_gid = nid;
                            
                            el.m_node[0] = face.n[0];
                            el.m_node[1] = face.n[1];
                            el.m_node[2] = face.n[2];
                            el.m_node[3] = face.n[3];
                            
                            el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                            el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                            el.m_node[6] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                            el.m_node[7] = NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                            
                        }
                        // move the face
                        face.n[0] = el.m_node[4];
                        face.n[1] = el.m_node[5];
                        face.n[2] = el.m_node[6];
                        face.n[3] = el.m_node[7];
                        face.m_elem[0] = n;
                        face.m_elem[1] = -1;
                    }
                    else
                    {
                        // we need to figure out which edge is the rotation axis
                        for (int j=0; j<4; ++j)
                        {
                            int n0 = face.n[j];
                            int n1 = face.n[(j + 1) % 4];
                            int n2 = face.n[(j + 2) % 4];
                            int n3 = face.n[(j + 3) % 4];
                            int n4 = face.n[j + 4];
                            int n5 = face.n[((j + 1) % 4) + 4];
                            int n6 = face.n[((j + 2) % 4) + 4];
                            int n7 = face.n[((j + 3) % 4) + 4];
                            
                            
                            if ((pmnew->Node(n0).m_ntag == -2) && (pmnew->Node(n1).m_ntag == -2))
                            {
                                
                                if(isQuad){
                                    el.SetType(FE_PENTA15);
                                    el.m_gid = nid;
                                    
                                    el.m_node[0] = n0;
                                    el.m_node[1] = n3;
                                    el.m_node[2] = NN0 + (l - 1)*nn + pmnew->Node(n3).m_ntag;
                                    
                                    el.m_node[3] = n1;
                                    el.m_node[4] = n2;
                                    el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    
                                    el.m_node[6] = n7;
                                    el.m_node[7] = 1 + NN0 + (l - 1)*nn + pmnew->Node(n3).m_ntag;
                                    el.m_node[8] = NN0 + (l - 1)*nn + pmnew->Node(n7).m_ntag;
                                    
                                    el.m_node[ 9] = n5;
                                    el.m_node[10] = 1 + NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[11] = NN0 + (l - 1)*nn + pmnew->Node(n5).m_ntag;
                                    
                                    el.m_node[12] = n4;
                                    el.m_node[13] = n6;
                                    el.m_node[14] = NN0 + (l - 1)*nn + pmnew->Node(n6).m_ntag;
                                    
                                    // move the face
                                    face.n[(j + 2) % 4] = el.m_node[5];
                                    face.n[(j + 3) % 4] = el.m_node[2];
                                    face.n[((j + 1) % 4) + 4] = el.m_node[11];
                                    face.n[((j + 2) % 4) + 4] = el.m_node[14];
                                    face.n[((j + 3) % 4) + 4] = el.m_node[8];
                                    face.m_elem[0] = n;
                                    face.m_elem[1] = -1;
                                    
                                    break;
                                }
                                else{
                                    el.SetType(FE_PENTA6);
                                    el.m_gid = nid;
                                    
                                    el.m_node[0] = n0;
                                    el.m_node[1] = n3;
                                    el.m_node[2] = NN0 + (l - 1)*nn + pmnew->Node(face.n[(j + 3) % nf]).m_ntag;
                                    
                                    el.m_node[3] = n1;
                                    el.m_node[4] = n2;
                                    el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[(j + 2) % nf]).m_ntag;
                                    
                                    // move the face
                                    face.n[(j + 2) % nf] = el.m_node[5];
                                    face.n[(j + 3) % nf] = el.m_node[2];
                                    face.m_elem[0] = n;
                                    face.m_elem[1] = -1;
                                    
                                    break;
                                }
                            }
                        }
                    }
                    
                    ++n;
                }
            }
        }
    }
    
    // clear face selection
    // TODO: Ideally, the corresponding faces in the mesh should be selected
    pmnew->ClearFaceSelection();
    
    // gets rid of nodes that are not used in any elements (mainly for quadratic meshes)
    pmnew->RemoveIsolatedNodes();
    
    // rebuild the object
    pmnew->RebuildMesh();
    
    // ensures all element have positive volume
    for (int i = 0; i<pmnew->Elements(); ++i) {
        double Ve = FEMeshMetrics::ElementVolume(*pmnew, pmnew->Element(i));
        if (Ve < 0)
            pmnew->Element(i).m_ntag = -1;
        else
            pmnew->Element(i).m_ntag = 1;
    }
    pmnew->InvertTaggedElements(-1);
    
    return pmnew;
}

FEMesh* FERevolveFaces::RevolveShellMesh(FEMesh* pm)
{
    // for now, only quad4 meshes
    //if (pm->IsType(FE_QUAD4) == false) return 0;
    
    // get the parameters
    vec3d axis = GetVecValue(0); axis.Normalize();
    vec3d center = GetVecValue(1);
    double w = GetFloatValue(2) * DEG2RAD;
    int nseg = GetIntValue(3);
    if (nseg < 1) return 0;
    
    // count the tagged nodes
    vector<int> nodeList;
    int NN0 = pm->Nodes();
    int nn = 0;
    for (int i = 0; i<NN0; ++i)
    {
        FENode& node = pm->Node(i);
        
        // if a node lies on the axis of rotation we do not need to duplicate it
        vec3d r = pm->NodePosition(i);
        if (pointOnAxis(center, axis, r) == false)
        {
            double l = (r - center)*axis;
            node.m_ntag = nn++;
            nodeList.push_back(i);
        }
        else node.m_ntag = -2;
    }
    assert(nn == (int)nodeList.size());
    if (nn == 0) return 0;
    
    // allocate new mesh
    int NN1 = NN0 + nn*nseg;
    FEMesh* pmnew = new FEMesh(*pm);
    pmnew->Create(NN1, 0);
    
    // revolve the tagged nodes
    for (int l = 1; l <= nseg; ++l)
    {
        // setup rotation
        double wl = w * l / nseg;
        quatd Q(wl, axis);
        
        // loop over all tagged nodes
        for (int i = 0; i<nn; ++i)
        {
            FENode& node = pmnew->Node(nodeList[i]);
            
            FENode& node2 = pmnew->Node(NN0 + (l - 1)*nn + node.m_ntag);
            
            vec3d r = pm->LocalToGlobal(node.r) - center;
            Q.RotateVector(r);
            node2.r = pm->GlobalToLocal(center + r);
            node2.m_ntag = node.m_ntag;
        }
    }
    
    // create new elements
    int NE0 = pm->Elements();
    pmnew->Create(0, nseg*NE0);
    
    int n = 0;
    for (int l = 1; l <= nseg; ++l)
    {
        for (int i = 0; i<pm->Faces(); ++i)
        {
            FEFace& face = pmnew->Face(i);
            
            int nf = face.Nodes();
            FEElement& el = pmnew->Element(n);
            
            if (nf == 3)
            {
                el.SetType(FE_PENTA6);
                
                el.m_node[0] = face.n[0];
                el.m_node[1] = face.n[1];
                el.m_node[2] = face.n[2];
                
                el.m_node[3] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                
                // move the face
                face.n[0] = el.m_node[3];
                face.n[1] = el.m_node[4];
                face.n[2] = el.m_node[5];
                face.m_elem[0] = n;
                face.m_elem[1] = -1;
                
                ++n;
            }
            else if (nf)
            {
                // see if we need to make a hex or a wedge
                bool bhex = true;
                for (int j = 0; j<nf; ++j) if (pmnew->Node(face.n[j]).m_ntag == -2) bhex = false;
                
                if (bhex)
                {
                    el.SetType(FE_HEX8);
                    
                    el.m_node[0] = face.n[0];
                    el.m_node[1] = face.n[1];
                    el.m_node[2] = face.n[2];
                    el.m_node[3] = face.n[3];
                    
                    el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                    el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                    el.m_node[6] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                    el.m_node[7] = NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                    
                    // move the face
                    face.n[0] = el.m_node[4];
                    face.n[1] = el.m_node[5];
                    face.n[2] = el.m_node[6];
                    face.n[3] = el.m_node[7];
                    face.m_elem[0] = n;
                    face.m_elem[1] = -1;
                }
                else
                {
                    // we need to figure out which edge is the rotation axis
                    for (int j = 0; j<nf; ++j)
                    {
                        int n0 = face.n[j];
                        int n1 = face.n[(j + 1) % nf];
                        int n2 = face.n[(j + 2) % nf];
                        int n3 = face.n[(j + 3) % nf];
                        if ((pmnew->Node(n0).m_ntag == -2) && (pmnew->Node(n1).m_ntag == -2))
                        {
                            el.SetType(FE_PENTA6);
                            
                            el.m_node[0] = n0;
                            el.m_node[1] = n3;
                            el.m_node[2] = NN0 + (l - 1)*nn + pmnew->Node(face.n[(j + 3) % nf]).m_ntag;
                            
                            el.m_node[3] = n1;
                            el.m_node[4] = n2;
                            el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[(j + 2) % nf]).m_ntag;
                            
                            // move the face
                            face.n[(j + 2) % nf] = el.m_node[5];
                            face.n[(j + 3) % nf] = el.m_node[2];
                            face.m_elem[0] = n;
                            face.m_elem[1] = -1;
                            
                            break;
                        }
                    }
                }
                
                ++n;
            }
        }
    }
    
    // rebuild the object
    pmnew->RebuildMesh();
    pmnew->RemoveIsolatedNodes();
    
    
    return pmnew;
}
