#include <numeric>
#include <iostream>
#include <stdexcept>

#include "Tools/Display/bash_tools.h"
#include "Tools/Math/matrix.h"

#include "Encoder_LDPC.hpp"

using namespace aff3ct;
using namespace module;

template <typename B>
Encoder_LDPC<B>
::Encoder_LDPC(const int K, const int N, const int n_frames, const std::string name)
: Encoder<B>(K, N, n_frames, name), tG(N * K, 0)
{
}

template <typename B>
Encoder_LDPC<B>
::Encoder_LDPC(const int K, const int N, const tools::Sparse_matrix &G, const int n_frames,
               const std::string name)
: Encoder<B>(K, N, n_frames, name), tG(N * K, 0)
{
	if (K != (int)G.get_n_cols() || N != (int)G.get_n_rows())
		throw std::invalid_argument("aff3ct::module::Encoder_LDPC: \"G\" matrix dimensions are not compatible with "
		                            "\"K\" and \"N\".");

	auto CN_to_VN = G.get_col_to_rows();

	mipp::vector<B> full_G(K * N,0);
	for (auto i = 0; i < K; i++)
		for (size_t j = 0; j < CN_to_VN[i].size(); ++j)
			full_G[i * N + CN_to_VN[i][j]] = 1;

	tools::real_transpose(K, N, full_G, tG); // transposed for computation matter
}

template <typename B>
Encoder_LDPC<B>
::~Encoder_LDPC()
{
}

template <typename B>
void Encoder_LDPC<B>
::get_info_bits_pos(mipp::vector<B>& info_bits_pos)
{
	if (this->K > (int)info_bits_pos.size())
		throw std::length_error("aff3ct::module::Encoder_LDPC: \"info_bits_pos.size()\" has to be equal or greater "
		                        "than \"K\".");

	std::iota(info_bits_pos.begin(), info_bits_pos.begin() + this->K, 0);
}

template <typename B>
void Encoder_LDPC<B>
::_encode(const B *U_K, B *X_N, const int frame_id)
{
	// Real General Matrix Multiplication
	tools::rgemm(1, this->N, this->K, U_K, tG.data(), X_N);

	for (auto j = 0; j < this->N; ++j)
		X_N[j] %= 2;
}

// ==================================================================================== explicit template instantiation 
#include "Tools/types.h"
#ifdef MULTI_PREC
template class aff3ct::module::Encoder_LDPC<B_8>;
template class aff3ct::module::Encoder_LDPC<B_16>;
template class aff3ct::module::Encoder_LDPC<B_32>;
template class aff3ct::module::Encoder_LDPC<B_64>;
#else
template class aff3ct::module::Encoder_LDPC<B>;
#endif
// ==================================================================================== explicit template instantiation
