#pragma once


struct ParticleData
{
private:
#pragma pack(push, 1)

	struct PODState
	{
		double x;
		double y;
		double vx;
		double vy;
	};

	struct PODAuxState
	{
		double mass;
	};

	struct PODDeriv
	{
		double vx;
		double vy;
		double ax;
		double ay;
	};

#pragma pack(pop)

public:
	ParticleData();
	ParticleData(PODState* m_pState, PODAuxState* m_pAuxState);
	ParticleData(const ParticleData& ref);
	ParticleData& operator=(const ParticleData& ref);

	void Reset();
	bool IsNull() const;

	PODState* m_pState;
	PODAuxState* m_pAuxState;
};
