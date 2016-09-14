#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iostream>

#include "Tools/bash_tools.h"
#include "Tools/simu_tools.h"
#include "Tools/Math/utils.h"

#include "Tools/Factory/RSC/Factory_encoder_RSC.hpp"
#include "Tools/Factory/RSC/Factory_decoder_RSC.hpp"

#include "Simulation_EXIT_RSC.hpp"

template <typename B, typename R, typename Q, typename QD>
Simulation_EXIT_RSC<B,R,Q,QD>
::Simulation_EXIT_RSC(const parameters& params)
: Simulation_EXIT<B,R,Q>(params)
{
}

template <typename B, typename R, typename Q, typename QD>
Simulation_EXIT_RSC<B,R,Q,QD>
::~Simulation_EXIT_RSC()
{
}

template <typename B, typename R, typename Q, typename QD>
void Simulation_EXIT_RSC<B,R,Q,QD>
::extract_sys_par(const mipp::vector<Q> &Lch_N, 
                  const mipp::vector<Q> &La_K, 
                        mipp::vector<Q> &sys, 
                        mipp::vector<Q> &par)
{
	// extract systematic and parity information
	for (auto i = 0; i < this->params.code.K + this->params.code.tail_length/2; i++)
	{
		sys[i] = Lch_N[i*2 +0];
		par[i] = Lch_N[i*2 +1];
	}

	// add other siso's extrinsic
	for(auto i = 0 ; i < this->params.code.K ; i ++)
		sys[i] += La_K[i];
}

template <typename B, typename R, typename Q, typename QD>
void Simulation_EXIT_RSC<B,R,Q,QD>
::launch_precompute()
{
}

template <typename B, typename R, typename Q, typename QD>
void Simulation_EXIT_RSC<B,R,Q,QD>
::snr_precompute()
{
}

template <typename B, typename R, typename Q, typename QD>
Encoder<B>* Simulation_EXIT_RSC<B,R,Q,QD>
::build_encoder()
{
	auto encoder = Factory_encoder_RSC<B>::build(this->params);
	trellis = encoder->get_trellis();
	return encoder;
}

template <typename B, typename R, typename Q, typename QD>
SISO<Q>* Simulation_EXIT_RSC<B,R,Q,QD>
::build_siso()
{
	return Factory_decoder_RSC<B,Q,QD>::build_siso(this->params, trellis);
}

// ==================================================================================== explicit template instantiation 
#include "Tools/types.h"
#ifdef MULTI_PREC
template class Simulation_EXIT_RSC<B_8,R_8,Q_8,QD_8>;
template class Simulation_EXIT_RSC<B_16,R_16,Q_16,QD_16>;
template class Simulation_EXIT_RSC<B_32,R_32,Q_32,QD_32>;
template class Simulation_EXIT_RSC<B_64,R_64,Q_64,QD_64>;
#else
template class Simulation_EXIT_RSC<B,R,Q,QD>;
#endif
// ==================================================================================== explicit template instantiation
