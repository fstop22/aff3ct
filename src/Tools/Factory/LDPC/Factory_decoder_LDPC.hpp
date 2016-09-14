#ifndef FACTORY_DECODER_LDPC_HPP
#define FACTORY_DECODER_LDPC_HPP

#include <string>

#include "Module/Decoder/Decoder.hpp"
#include "Module/Decoder/Decoder_SISO.hpp"

#include "Tools/params.h"

template <typename B, typename R>
struct Factory_decoder_LDPC
{
	static Decoder_SISO<B,R>* build(const parameters                  &params,
	                                const std ::vector<unsigned char> &n_variables_per_parity,
	                                const std ::vector<unsigned char> &n_parities_per_variable,
	                                const std ::vector<unsigned int > &transpose,
	                                const mipp::vector<B            > &X_N);
};

#endif /* FACTORY_DECODER_LDPC_HPP */
