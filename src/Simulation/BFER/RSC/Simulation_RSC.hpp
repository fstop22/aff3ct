#ifndef SIMULATION_RSC_HPP_
#define SIMULATION_RSC_HPP_

#include "../Simulation_BFER.hpp"

template <typename B, typename R, typename Q, typename QD>
class Simulation_RSC : public Simulation_BFER<B,R,Q>
{
protected:
	// the trellis representation
	std::vector<std::vector<int>> trellis;

public:
	Simulation_RSC(const parameters& params);
	virtual ~Simulation_RSC();

protected:
	void          launch_precompute();
	void          snr_precompute   ();
	Encoder<B>*   build_encoder    (const int tid = 0);
	Decoder<B,Q>* build_decoder    (const int tid = 0);
};

#endif /* SIMULATION_RSC_HPP_ */
