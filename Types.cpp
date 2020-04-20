#include "Types.h"
#include <cassert>
#include <cstdlib>

ParticleData::ParticleData() : m_pState(nullptr), m_pAuxState(nullptr) {}

//----------------------------------------------------------------------------------------
ParticleData::ParticleData(PODState *pState, PODAuxState *pAuxState)
    : m_pState(pState), m_pAuxState(pAuxState) {
  assert(m_pState);
  assert(m_pAuxState);
}

//----------------------------------------------------------------------------------------
ParticleData::ParticleData(const ParticleData &ref)
    : m_pState(ref.m_pState), m_pAuxState(ref.m_pAuxState) {}

//----------------------------------------------------------------------------------------
ParticleData &ParticleData::operator=(const ParticleData &ref) {
  if (this != &ref) {
    m_pState = ref.m_pState;
    m_pAuxState = ref.m_pAuxState;
  }

  return *this;
}

//----------------------------------------------------------------------------------------
void ParticleData::Reset() {
  m_pState = nullptr;
  m_pAuxState = nullptr;
}

//----------------------------------------------------------------------------------------
bool ParticleData::IsNull() const { return m_pState && m_pAuxState; }
