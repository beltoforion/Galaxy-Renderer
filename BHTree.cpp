#include "BHTree.h"

//--- Standard includes --------------------------------------------------------
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <sstream>

//------------------------------------------------------------------------------
// static variables
double BHTreeNode::s_theta = 0.9;
std::vector<ParticleData> BHTreeNode::s_renegades;
BHTreeNode::DebugStat BHTreeNode::s_stat = {0};
double BHTreeNode::s_gamma = 0;       // gravitational constant is set from the outside
double BHTreeNode::s_soft = 0; //0.1*0.1;        // approx. 3 light year

//------------------------------------------------------------------------------
BHTreeNode::BHTreeNode(const Vec2D &min,
                       const Vec2D &max,
                       BHTreeNode *parent)
  :m_particle()
  ,m_mass(0)
  ,m_cm()
  ,m_min(min)
  ,m_max(max)
  ,m_center(min.x+(max.x-min.x)/2.0, min.y+(max.y-min.y)/2.0)
  ,m_parent(parent)
  ,m_num(0)
  ,m_bSubdivided(false)
{
  m_quadNode[0] = m_quadNode[1] = m_quadNode[2] = m_quadNode[3] = NULL;
}

//------------------------------------------------------------------------------
bool BHTreeNode::IsRoot() const
{
  return m_parent==NULL;
}

//------------------------------------------------------------------------------
bool BHTreeNode::IsExternal() const
{
  return  m_quadNode[0]==NULL &&
          m_quadNode[1]==NULL &&
          m_quadNode[2]==NULL &&
          m_quadNode[3]==NULL;
}

//------------------------------------------------------------------------------
bool BHTreeNode::WasTooClose() const
{
  return m_bSubdivided;
}

//------------------------------------------------------------------------------
const Vec2D& BHTreeNode::GetMin() const
{
  return m_min;
}

//------------------------------------------------------------------------------
const Vec2D& BHTreeNode::GetMax() const
{
  return m_max;
}

//------------------------------------------------------------------------------
const Vec2D& BHTreeNode::GetCenterOfMass() const
{
  return m_cm;
}

//------------------------------------------------------------------------------
double BHTreeNode::GetTheta() const
{
  return s_theta;
}

//------------------------------------------------------------------------------
void BHTreeNode::SetTheta(double theta)
{
  s_theta = theta;
}

//------------------------------------------------------------------------------
int BHTreeNode::StatGetNumCalc() const
{
  return s_stat.m_nNumCalc;
}

//------------------------------------------------------------------------------
/** \brief Returns the number of particles not assigned to any node. */
int BHTreeNode::GetNumRenegades() const
{
  return s_renegades.size();
}

//------------------------------------------------------------------------------
/** \brief Returns the number of particles inside this node. */
int BHTreeNode::GetNum() const
{
  return m_num;
}

//------------------------------------------------------------------------------
void BHTreeNode::StatReset()
{
  if (!IsRoot())
    throw std::runtime_error("Only the root node may reset statistics data.");

  s_stat.m_nNumCalc = 0;

  struct ResetSubdivideFlags
  {
    ResetSubdivideFlags(BHTreeNode *pRoot)
    {
      ResetFlag(pRoot);
    }

    void ResetFlag(BHTreeNode *pNode)
    {
      pNode->m_bSubdivided = false;
      for (int i=0; i<4;++i)
      {
        if (pNode->m_quadNode[i])
          ResetFlag(pNode->m_quadNode[i]);
      }
    }
  } ResetFlagNow(this);
}

//------------------------------------------------------------------------------
void BHTreeNode::Reset(const Vec2D &min,
                       const Vec2D &max)
{
  if (!IsRoot())
    throw std::runtime_error("Only the root node may reset the tree.");

  for (int i=0; i<4; ++i)
  {
    delete m_quadNode[i];
    m_quadNode[i] = NULL;
  }

  m_min = min;
  m_max = max;
  m_center = Vec2D(min.x + (max.x-min.x)/2.0,
                   min.y + (max.y-min.y)/2.0);
  m_num = 0;
  m_mass = 0;
  m_cm = Vec2D(0, 0);

  s_renegades.clear();
}

//------------------------------------------------------------------------------
BHTreeNode::EQuadrant BHTreeNode::GetQuadrant(double x, double y) const
{
  if (x<=m_center.x && y<=m_center.y)
  {
    return SW;
  }
  else if (x<=m_center.x && y>=m_center.y)
  {
    return NW;
  }
  else if (x>=m_center.x && y>=m_center.y)
  {
    return NE;
  }
  else if (x>=m_center.x && y<=m_center.y)
  {
    return SE;
  }
  else if (x>m_max.x || y>m_max.y || x<m_min.x || y<m_min.y)
  {
    std::stringstream ss;
    ss << "Can't determine quadrant!\n"
       << "particle  : " << "(" << x          << ", " << y          << ")\n"
       << "quadMin   : " << "(" << m_min.x    << ", " << m_min.y    << ")\n"
       << "quadMax   : " << "(" << m_max.y    << ", " << m_max.y    << ")\n"
       << "quadCenter: " << "(" << m_center.x << ", " << m_center.y << ")\n";
    throw std::runtime_error(ss.str().c_str());
  }
  else
  {
    throw std::runtime_error("Can't determine quadrant!");
  }
}

//------------------------------------------------------------------------------
BHTreeNode* BHTreeNode::CreateQuadNode(EQuadrant eQuad)
{
  switch (eQuad)
  {
  case SW: return new BHTreeNode(m_min, m_center, this);
  case NW: return new BHTreeNode(Vec2D(m_min.x, m_center.y),
                                 Vec2D(m_center.x, m_max.y),
                                 this);
  case NE: return new BHTreeNode(m_center, m_max, this);
  case SE: return new BHTreeNode(Vec2D(m_center.x, m_min.y),
                                 Vec2D(m_max.x, m_center.y),
                                 this);
  default:
        {
          std::stringstream ss;
          ss << "Can't determine quadrant!\n";
/*
           << "particle  : " << "(" << x          << ", " << y          << ")\n"
             << "quadMin   : " << "(" << m_min.x    << ", " << m_min.y    << ")\n"
             << "quadMax   : " << "(" << m_max.y    << ", " << m_max.y    << ")\n"
             << "quadCenter: " << "(" << m_center.x << ", " << m_center.y << ")\n";
*/
          throw std::runtime_error(ss.str().c_str());
        }
  }
}

//------------------------------------------------------------------------------
void BHTreeNode::ComputeMassDistribution()
{

  if (m_num==1)
  {
    PODState *ps = m_particle.m_pState;
    PODAuxState *pa = m_particle.m_pAuxState;
    assert(ps);
    assert(pa);

    m_mass = pa->mass;
    m_cm = Vec2D(ps->x, ps->y);
  }
  else
  {
    m_mass = 0;
    m_cm = Vec2D(0, 0);

    for (int i=0; i<4; ++i)
    {
      if (m_quadNode[i])
      {
        m_quadNode[i]->ComputeMassDistribution();
        m_mass += m_quadNode[i]->m_mass;
        m_cm.x += m_quadNode[i]->m_cm.x * m_quadNode[i]->m_mass;
        m_cm.y += m_quadNode[i]->m_cm.y * m_quadNode[i]->m_mass;
      }
    }

    m_cm.x /= m_mass;
    m_cm.y /= m_mass;
  }
}

//------------------------------------------------------------------------------
/** \brief Calculate the accelleration caused by gravitaion of p2 on p1. */
Vec2D BHTreeNode::CalcAcc(const ParticleData &p1, const ParticleData &p2) const
{
  Vec2D acc;

  if (&p1==&p2)
    return acc;

  // assign references to the variables in a readable form
  const double &x1(p1.m_pState->x),
               &y1(p1.m_pState->y);
  const double &x2(p2.m_pState->x),
               &y2(p2.m_pState->y),
               &m2(p2.m_pAuxState->mass);


  double r = sqrt( (x1 - x2) * (x1 - x2) +
                   (y1 - y2) * (y1 - y2) + s_soft);
  if (r>0)
  {
    double k = s_gamma * m2 / (r*r*r);

    acc.x += k * (x2 - x1);
    acc.y += k * (y2 - y1);
  } // if distance is greater zero
  else
  {
    // two particles on the same spot is physical nonsense!
    // nevertheless it may happen. I just need to make sure
    // there is no singularity...
    acc.x = acc.y = 0;
  }

  return acc;
}

//------------------------------------------------------------------------------
Vec2D BHTreeNode::CalcForce(const ParticleData &p1) const
{
  // calculate the force from the barnes hut tree to the particle p1
  Vec2D acc = CalcTreeForce(p1);

  // calculate the force from particles not in the barnes hut tree on particle p
  if (s_renegades.size())
  {
    for (std::size_t i=0; i<s_renegades.size(); ++i)
    {
      Vec2D buf = CalcAcc(p1, s_renegades[i]);
      acc.x += buf.x;
      acc.y += buf.y;
    }
  }

  return acc;
}

//------------------------------------------------------------------------------
/**  \brief Compute the force acting from this node and it's child
            to a particle p.
*/
Vec2D BHTreeNode::CalcTreeForce(const ParticleData &p1) const
{
  Vec2D acc;

  double r(0), k(0), d(0);
  if (m_num==1)
  {
    acc = CalcAcc(p1, m_particle);
    s_stat.m_nNumCalc++;
  }
  else
  {
    r = sqrt( (p1.m_pState->x - m_cm.x) * (p1.m_pState->x - m_cm.x) +
              (p1.m_pState->y - m_cm.y) * (p1.m_pState->y - m_cm.y) );
    d = m_max.x - m_min.x;
    if (d/r <= s_theta)
    {
      m_bSubdivided = false;
      k = s_gamma * m_mass / (r*r*r);
      acc.x = k * (m_cm.x - p1.m_pState->x);
      acc.y = k * (m_cm.y - p1.m_pState->y);

      // keep track of the number of calculations
      s_stat.m_nNumCalc++;
    }
    else
    {

      m_bSubdivided = true;
      Vec2D buf;
      for (int q=0; q<4; ++q)
      {
        if (m_quadNode[q])
        {
//          const PODState &state = *(p1.m_pState);
          buf = m_quadNode[q]->CalcTreeForce(p1);
          acc.x += buf.x;
          acc.y += buf.y;
        } // if node exists
      } // for all child nodes
    }
  }

  return acc;
}

//------------------------------------------------------------------------------
void BHTreeNode::DumpNode(int quad, int level)
{
  std::string space;
  for (int i=0; i<level; ++i)
      space+= "  ";

  std::cout << space << "Quadrant " << quad << ": ";
  std::cout << space << "(num=" << m_num << "; ";
  std::cout << space << "mass=" << m_mass << ";";
  std::cout << space << "cx=" << m_cm.x << ";";
  std::cout << space << "cy=" << m_cm.y << ")\n";

  for (int i=0; i<4;++i)
  {
    if (m_quadNode[i])
    {
      m_quadNode[i]->DumpNode(i, level+1);
    }
  }
}

//------------------------------------------------------------------------------
void BHTreeNode::Insert(const ParticleData &newParticle, int level)
{
  const PODState &p1 = *(newParticle.m_pState);
  if ( (p1.x < m_min.x || p1.x > m_max.x) || (p1.y < m_min.y || p1.y > m_max.y) )
  {
    std::stringstream ss;
    ss << "Particle position (" << p1.x << ", " << p1.y << ") "
       << "is outside tree node ("
       << "min.x=" << m_min.x << ", "
       << "max.x=" << m_max.x << ", "
       << "min.y=" << m_min.y << ", "
       << "max.y=" << m_max.y << ")";
    throw std::runtime_error(ss.str());
  }

  if (m_num>1)
  {
    EQuadrant eQuad = GetQuadrant(p1.x, p1.y);
    if (!m_quadNode[eQuad])
      m_quadNode[eQuad] = CreateQuadNode(eQuad);

    m_quadNode[eQuad]->Insert(newParticle, level+1);
  }
  else if (m_num==1)
  {
    assert(IsExternal() || IsRoot());

    const PODState &p2 = *(m_particle.m_pState);

    // This is physically impossible: There are
    // two bodies at the exact same coordinates. In these
    // cases do not add the second body and place
    // it in the renegade vector.
    if ( (p1.x == p2.x) && (p1.y == p2.y) )
    {
      s_renegades.push_back(newParticle);
    }
    else
    {
      // There is already a particle
      // subdivide the node and relocate that particle
      EQuadrant eQuad = GetQuadrant(p2.x, p2.y);
      if (m_quadNode[eQuad]==NULL)
        m_quadNode[eQuad] = CreateQuadNode(eQuad);
      m_quadNode[eQuad]->Insert(m_particle, level+1);
      m_particle.Reset();

      eQuad = GetQuadrant(p1.x, p1.y);
      if (!m_quadNode[eQuad])
        m_quadNode[eQuad] = CreateQuadNode(eQuad);
      m_quadNode[eQuad]->Insert(newParticle, level+1);
    }
  }
  else if (m_num==0)
  {
    m_particle = newParticle;
  }

  m_num++;
}

//------------------------------------------------------------------------------
BHTreeNode::~BHTreeNode()
{
  for (int i=0; i<4; ++i)
    delete m_quadNode[i];
}
