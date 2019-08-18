//
//  BivariateSpline.cpp
//
//  Created by Gerard Ateshian on 9/18/14.
//
//

#include "BivariateSpline.h"

//--------------------------------------------------------------------------------------
// Evaluate the surface normal at parametric coordinates (u,v)
vec3d BivariateSpline::SurfaceNormal(const double u, const double v)
{
    // get tangent along u
    vec3d pu = SurfaceDerivative(u, v, 1, 0);
    // get tangent along v
    vec3d pv = SurfaceDerivative(u, v, 0, 1);
    // evaluate cross-product
    vec3d pn = pu ^ pv;
    // normalize
    pn.Normalize();
    return pn;
}

//--------------------------------------------------------------------------------------
// Evaluate the surface principal curvatures kappa and directions theta at parametric coordinates (u,v)
void BivariateSpline::SurfaceCurvature(const double u, const double v, const vec3d xn, vec2d& kappa, vec3d* theta)
{
    // get derivatives along u
    vec3d pu = SurfaceDerivative(u, v, 1, 0);
    vec3d puu = SurfaceDerivative(u, v, 2, 0);
    // get derivatives along v
    vec3d pv = SurfaceDerivative(u, v, 0, 1);
    vec3d pvv = SurfaceDerivative(u, v, 0, 2);
    // get cross-derivative
    vec3d puv = SurfaceDerivative(u, v, 1, 1);
    // evaluate normal
    vec3d pn = pu ^ pv;
    pn.Normalize();
    
    // evaluate coefficients of fundamental forms
    double se = pu*pu;
    double sf = pu*pv;
    double sg = pv*pv;
    double sl = pn*puu;
    double sm = pn*puv;
    double sn = pn*pvv;
    
    // evaluate mean and gaussian curvatures
    double tmp = se*sg - sf*sf;
    // gaussian
    double sk = (sl*sn - sm*sm)/tmp;
    // mean
    double sh = (2*sf*sm - se*sn - sg*sl)/2./tmp;
    
    // evaluate principal curvatures
    double d = sqrt(sh*sh - sk);
    // max curvature
    double kmax = sh + d;
    // min curvature
    double kmin = sh - d;
    
    // evaluate principal directions of curvature
    double ha = sf*sn - sg*sm;
    double hb = se*sn - sg*sl;
    double hc = se*sm - sf*sl;
    d = hb*hb - 4*ha*hc; if (fabs(d) < 0) d = 0;
    double hd = sqrt(d);
    double thmax = atan2(-hb + hd, 2*ha);
    double thmin = (d > 0) ? atan2(-hb - hd, 2*ha) : thmax + PI/2;
    vec3d xmax = pu*cos(thmax) + pv*sin(thmax); xmax.Normalize();
    vec3d xmin = pu*cos(thmin) + pv*sin(thmin); xmin.Normalize();

    // check spline normal versus face normal
    if (xn*pn >= 0) {
        kappa.x = kmax; kappa.y = kmin;
        theta[0] = xmax; theta[1] = xmin;
    }
    else {
        kappa.x = kmin; kappa.y = kmax;
        theta[0] = xmin; theta[1] = xmax;
    }
    // fix handedness if neeeded
    if ((theta[0]^theta[1])*xn < 0) theta[0] = -theta[0];
    
    return;
}

//--------------------------------------------------------------------------------------
// Evaluate surface points at parametric coordinates of point cloud data
void BivariateSpline::FittedPoints(PointCloud3d& pc, double& rmsres)
{
    pc = new PointCloud3d(m_pc);
    
    rmsres = 0;
    int np = m_pc->Points();
    
    for (int i=0; i<np; ++i) {
        pc.m_p[i] = SurfacePoint(pc.m_u[i].x, pc.m_u[i].y);
        vec3d d = m_pc->m_p[i] - pc.m_p[i];
        rmsres += d*d;
    }
    
    rmsres = sqrt(rmsres/np);
    
    return;
}

