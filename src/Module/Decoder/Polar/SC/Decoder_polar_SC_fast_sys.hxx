#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include "Tools/Perf/Reorderer/Reorderer.hpp"
#include "Tools/Perf/Transpose/transpose_selector.h"

#include "Decoder_polar_SC_fast_sys.hpp"

#include "Tools/Code/Polar/Patterns/Pattern_polar_r0.hpp"
#include "Tools/Code/Polar/Patterns/Pattern_polar_r0_left.hpp"
#include "Tools/Code/Polar/Patterns/Pattern_polar_r1.hpp"
#include "Tools/Code/Polar/Patterns/Pattern_polar_rep.hpp"
#include "Tools/Code/Polar/Patterns/Pattern_polar_rep_left.hpp"
#include "Tools/Code/Polar/Patterns/Pattern_polar_spc.hpp"
#include "Tools/Code/Polar/Patterns/Pattern_polar_std.hpp"

namespace aff3ct
{
namespace module
{
constexpr int static_level = 6; // 2^6 = 64

template <typename B, typename R, class API_polar, int REV_D>
struct Decoder_polar_SC_fast_sys_static
{
	static void decode(const tools::Pattern_polar_parser<B> &polar_patterns, mipp::vector<B> &s,
	                   mipp::vector<R> &l, const int off_l, const int off_s, int &node_id,
	                   const bool k_extract, mipp::vector<B> &sk, int &sk_idx)
	{
		constexpr int reverse_depth = REV_D;
		constexpr int n_elmts = 1 << reverse_depth;
		constexpr int n_elm_2 = n_elmts >> 1;

		const tools::polar_node_t node_type = polar_patterns.get_node_type(node_id);

		const bool is_terminal_pattern = (node_type == tools::polar_node_t::RATE_0) ||
		                                 (node_type == tools::polar_node_t::RATE_1) ||
		                                 (node_type == tools::polar_node_t::REP)    ||
		                                 (node_type == tools::polar_node_t::SPC);

		if (!is_terminal_pattern && reverse_depth)
		{
			// f
			switch (node_type)
			{
				case tools::STANDARD: API_polar::template f<n_elm_2>(l, off_l, off_l + n_elm_2, off_l + n_elmts, n_elm_2); break;
				case tools::REP_LEFT: API_polar::template f<n_elm_2>(l, off_l, off_l + n_elm_2, off_l + n_elmts, n_elm_2); break;
				default:
					break;
			}

			Decoder_polar_SC_fast_sys_static<B,R,API_polar,REV_D-1>
			::decode(polar_patterns, s, l, off_l + n_elmts, off_s, ++node_id, k_extract, sk, sk_idx); // recursive call left

			// g
			switch (node_type)
			{
				case tools::STANDARD:    API_polar::template g <n_elm_2>(s, l, off_l, off_l + n_elm_2, off_s, off_l + n_elmts, n_elm_2); break;
				case tools::RATE_0_LEFT: API_polar::template g0<n_elm_2>(   l, off_l, off_l + n_elm_2,        off_l + n_elmts, n_elm_2); break;
				case tools::REP_LEFT:    API_polar::template gr<n_elm_2>(s, l, off_l, off_l + n_elm_2, off_s, off_l + n_elmts, n_elm_2); break;
				default:
					break;
			}

			Decoder_polar_SC_fast_sys_static<B,R,API_polar,REV_D-1>
			::decode(polar_patterns, s, l, off_l + n_elmts, off_s + n_elm_2, ++node_id, k_extract, sk, sk_idx); // recursive call right

			// xor
			switch (node_type)
			{
				case tools::STANDARD:    API_polar::template xo <n_elm_2>(s, off_s, off_s + n_elm_2, off_s, n_elm_2); break;
				case tools::RATE_0_LEFT: API_polar::template xo0<n_elm_2>(s,        off_s + n_elm_2, off_s, n_elm_2); break;
				case tools::REP_LEFT:    API_polar::template xo <n_elm_2>(s, off_s, off_s + n_elm_2, off_s, n_elm_2); break;
				default:
					break;
			}
		}
		else
		{
			// h
			switch (node_type)
			{
				case tools::RATE_0:
					// if (!polar_patterns.exist_node_type(tools::polar_node_t::RATE_0_LEFT, REV_D+1))
					API_polar::template h0<n_elmts>(s, off_s, n_elmts);
					break;
				case tools::RATE_1:
					API_polar::template h<n_elmts>(s, l, off_l, off_s, n_elmts);
					if (k_extract)
					{
						std::copy(s.begin() +  off_s            * API_polar::get_n_frames(),
						          s.begin() + (off_s + n_elmts) * API_polar::get_n_frames(),
						          sk.begin() + sk_idx);

						sk_idx += n_elmts * API_polar::get_n_frames();
					}
					break;
				case tools::REP:
					API_polar::template rep<n_elmts>(s, l, off_l, off_s, n_elmts);
					if (k_extract)
					{
						std::copy(s.begin() + (off_s + n_elmts -2) * API_polar::get_n_frames(),
						          s.begin() + (off_s + n_elmts +0) * API_polar::get_n_frames(),
						          sk.begin() + sk_idx);

						sk_idx += API_polar::get_n_frames();
					}
					break;
				case tools::SPC:
					API_polar::template spc<n_elmts>(s, l, off_l, off_s, n_elmts);
					if (k_extract)
					{
						std::copy(s.begin() + (off_s + 1      ) * API_polar::get_n_frames(),
						          s.begin() + (off_s + n_elmts) * API_polar::get_n_frames(),
						          sk.begin() + sk_idx);

						sk_idx += (n_elmts -1) * API_polar::get_n_frames();
					}
					break;
				default:
					break;
			}
		}
	}
};

template <typename B, typename R, class API_polar>
struct Decoder_polar_SC_fast_sys_static<B,R,API_polar,0>
{
	static void decode(const tools::Pattern_polar_parser<B> &polar_patterns, mipp::vector<B> &s, mipp::vector<R> &l,
	                   const int off_l, const int off_s, int &node_id,
	                   const bool k_extract, mipp::vector<B> &sk, int &sk_idx)
	{
		constexpr int reverse_depth = 0;
		constexpr int n_elmts = 1 << reverse_depth;

		const tools::polar_node_t node_t = polar_patterns.get_node_type(node_id);

		switch (node_t)
		{
			case tools::RATE_0:
				// if (!polar_patterns.exist_node_type(tools::polar_node_t::RATE_0_LEFT, 1))
				API_polar::template h0<n_elmts>(s, off_s, n_elmts);
				break;
			case tools::RATE_1:
				API_polar::template h<n_elmts>(s, l, off_l, off_s, n_elmts);
				if (k_extract)
				{
					std::copy(s.begin() +  off_s            * API_polar::get_n_frames(),
					          s.begin() + (off_s + n_elmts) * API_polar::get_n_frames(),
					          sk.begin() + sk_idx);

					sk_idx += n_elmts * API_polar::get_n_frames();
				}
				break;
			default:
				break;
		}
	}
};

template <typename B, typename R, class API_polar>
Decoder_polar_SC_fast_sys<B,R,API_polar>
::Decoder_polar_SC_fast_sys(const int& K, const int& N, const mipp::vector<B>& frozen_bits, const int n_frames,
                            const bool k_extract, const std::string name)
: Decoder<B,R>  (K, N, n_frames, API_polar::get_n_frames(), name),
  m             ((int)std::log2(N)),
  l             (2 * N * this->simd_inter_frame_level + mipp::nElmtsPerRegister<R>()),
  s             (1 * N * this->simd_inter_frame_level + mipp::nElmtsPerRegister<B>()),
  s_bis         (1 * N * this->simd_inter_frame_level + mipp::nElmtsPerRegister<B>()),
  frozen_bits   (frozen_bits),
  k_extract     (k_extract),
  sk            (1 * K * this->simd_inter_frame_level),
  sk_idx        (0),
  polar_patterns(N,
                 frozen_bits,
                 {new tools::Pattern_polar_std,
                  new tools::Pattern_polar_r0_left,
                  new tools::Pattern_polar_r0,
                  new tools::Pattern_polar_r1,
                  new tools::Pattern_polar_rep_left,
                  new tools::Pattern_polar_rep,
                  new tools::Pattern_polar_spc},
                 2,
                 3)
{
	static_assert(sizeof(B) == sizeof(R), "");

	if (!tools::is_power_of_2(this->N))
		throw std::invalid_argument("aff3ct::module::Decoder_polar_SC_fast_sys: \"N\" has to be a power of 2.");

	if (this->N != (int)frozen_bits.size())
		throw std::length_error("aff3ct::module::Decoder_polar_SC_fast_sys: \"frozen_bits.size()\" has to be equal to "
		                        "\"N\".");

	std::fill(s.begin(), s.end(), (B)0);
}

template <typename B, typename R, class API_polar>
Decoder_polar_SC_fast_sys<B,R,API_polar>
::Decoder_polar_SC_fast_sys(const int& K, const int& N, const mipp::vector<B>& frozen_bits,
                            const std::vector<tools::Pattern_polar_i*> polar_patterns,
                            const int idx_r0, const int idx_r1,
                            const int n_frames, const bool k_extract, const std::string name)
: Decoder<B,R>  (K, N, n_frames, API_polar::get_n_frames(), name),
  m             ((int)std::log2(N)),
  l             (2 * N * this->simd_inter_frame_level + mipp::nElmtsPerRegister<R>()),
  s             (1 * N * this->simd_inter_frame_level + mipp::nElmtsPerRegister<B>()),
  s_bis         (1 * N * this->simd_inter_frame_level + mipp::nElmtsPerRegister<B>()),
  frozen_bits   (frozen_bits),
  k_extract     (k_extract),
  sk            (1 * K * this->simd_inter_frame_level),
  sk_idx        (0),
  polar_patterns(N, frozen_bits, polar_patterns, idx_r0, idx_r1)
{
	static_assert(sizeof(B) == sizeof(R), "");

	if (!tools::is_power_of_2(this->N))
		throw std::invalid_argument("aff3ct::module::Decoder_polar_SC_fast_sys: \"N\" has to be a power of 2.");

	if (this->N != (int)frozen_bits.size())
		throw std::length_error("aff3ct::module::Decoder_polar_SC_fast_sys: \"frozen_bits.size()\" has to be equal to "
		                        "\"N\".");

	std::fill(s.begin(), s.end(), (B)0);
}

template <typename B, typename R, class API_polar>
Decoder_polar_SC_fast_sys<B,R,API_polar>
::~Decoder_polar_SC_fast_sys()
{
	polar_patterns.release_patterns();
}

template <typename B, typename R, class API_polar>
void Decoder_polar_SC_fast_sys<B,R,API_polar>
::_load(const mipp::vector<R>& Y_N)
{
	constexpr int n_frames = API_polar::get_n_frames();

	if (n_frames == 1)
		std::copy(Y_N.begin(), Y_N.end(), l.begin());
	else
	{
		bool fast_interleave = false;
		if (typeid(B) == typeid(signed char))
			fast_interleave = tools::char_transpose((signed char*)Y_N.data(), (signed char*)l.data(), (int)this->N);

		if (!fast_interleave)
		{
			std::vector<const R*> frames(n_frames);
			for (auto f = 0; f < n_frames; f++)
				frames[f] = Y_N.data() + f*this->N;
			tools::Reorderer_static<R,n_frames>::apply(frames, l.data(), this->N);
		}
	}

	sk_idx = 0;
}

template <typename B, typename R, class API_polar>
void Decoder_polar_SC_fast_sys<B,R,API_polar>
::_hard_decode()
{
	if (m < static_level)
		throw std::runtime_error("aff3ct::module::Decoder_polar_SC_fast_sys: \"m\" has to be equal or greater than "
		                         "\"static_level\".");

	int first_id = 0, off_l = 0, off_s = 0;
	this->recursive_decode(off_l, off_s, m, first_id);
}

template <typename B, typename R, class API_polar>
void Decoder_polar_SC_fast_sys<B,R,API_polar>
::recursive_decode(const int off_l, const int off_s, const int reverse_depth, int &node_id)
{
	if (reverse_depth == static_level)
	{
		// use the static version of the decoder
		Decoder_polar_SC_fast_sys_static<B,R,API_polar,static_level>
		::decode(polar_patterns, this->s, this->l, off_l, off_s, node_id, k_extract, this->sk, sk_idx);
	}
	else
	{
		const int n_elmts = 1 << reverse_depth;
		const int n_elm_2 = n_elmts >> 1;
		const auto node_type = polar_patterns.get_node_type(node_id);

		const bool is_terminal_pattern = (node_type == tools::polar_node_t::RATE_0) ||
		                                 (node_type == tools::polar_node_t::RATE_1) ||
		                                 (node_type == tools::polar_node_t::REP)    ||
		                                 (node_type == tools::polar_node_t::SPC);

		if (!is_terminal_pattern && reverse_depth)
		{
			// f
			switch (node_type)
			{
				case tools::STANDARD: API_polar::f(l, off_l, off_l + n_elm_2, off_l + n_elmts, n_elm_2); break;
				case tools::REP_LEFT: API_polar::f(l, off_l, off_l + n_elm_2, off_l + n_elmts, n_elm_2); break;
				default:
					break;
			}

			this->recursive_decode(off_l + n_elmts, off_s, reverse_depth -1, ++node_id); // recursive call left

			// g
			switch (node_type)
			{
				case tools::STANDARD:    API_polar::g (s, l, off_l, off_l + n_elm_2, off_s, off_l + n_elmts, n_elm_2); break;
				case tools::RATE_0_LEFT: API_polar::g0(   l, off_l, off_l + n_elm_2,        off_l + n_elmts, n_elm_2); break;
				case tools::REP_LEFT:    API_polar::gr(s, l, off_l, off_l + n_elm_2, off_s, off_l + n_elmts, n_elm_2); break;
				default:
					break;
			}

			this->recursive_decode(off_l + n_elmts, off_s + n_elm_2, reverse_depth -1, ++node_id); // recursive call right

			// xor
			switch (node_type)
			{
				case tools::STANDARD:    API_polar::xo (s, off_s, off_s + n_elm_2, off_s, n_elm_2); break;
				case tools::RATE_0_LEFT: API_polar::xo0(s,        off_s + n_elm_2, off_s, n_elm_2); break;
				case tools::REP_LEFT:    API_polar::xo (s, off_s, off_s + n_elm_2, off_s, n_elm_2); break;
				default:
					break;
			}
		}
		else
		{
			// h
			switch (node_type)
			{
				case tools::RATE_0:
					// if (!polar_patterns.exist_node_type(tools::polar_node_t::RATE_0_LEFT, REV_D+1))
					API_polar::h0(s, off_s, n_elmts);
					break;
				case tools::RATE_1:
					API_polar::h(s, l, off_l, off_s, n_elmts);
					if (k_extract)
					{
						std::copy(s.begin() +  off_s            * API_polar::get_n_frames(),
						          s.begin() + (off_s + n_elmts) * API_polar::get_n_frames(),
						          sk.begin() + sk_idx);

						sk_idx += n_elmts * API_polar::get_n_frames();
					}
					break;
				case tools::REP:
					API_polar::rep(s, l, off_l, off_s, n_elmts);
					if (k_extract)
					{
						std::copy(s.begin() + (off_s + n_elmts -2) * API_polar::get_n_frames(),
						          s.begin() + (off_s + n_elmts +0) * API_polar::get_n_frames(),
						          sk.begin() + sk_idx);

						sk_idx += API_polar::get_n_frames();
					}
					break;
				case tools::SPC:
					API_polar::spc(s, l, off_l, off_s, n_elmts);
					if (k_extract)
					{
						std::copy(s.begin() + (off_s + 1      ) * API_polar::get_n_frames(),
						          s.begin() + (off_s + n_elmts) * API_polar::get_n_frames(),
						          sk.begin() + sk_idx);

						sk_idx += (n_elmts -1) * API_polar::get_n_frames();
					}
					break;
				default:
					break;
			}
		}
	}
}

template <typename B, typename R, class API_polar>
void Decoder_polar_SC_fast_sys<B,R,API_polar>
::_store(mipp::vector<B>& V_K) const
{
	constexpr int n_frames = API_polar::get_n_frames();

	if (n_frames == 1)
	{
		if (k_extract)
			V_K = sk;
		else
		{
			auto k = 0;
			for (auto j = 0; j < this->N; j++)
				if (!frozen_bits[j])
					V_K[k++] =  s[j] ? 1 : 0;
		}
	}
	else
	{
		bool fast_deinterleave = false;
#if defined(ENABLE_BIT_PACKING)
		if (typeid(B) == typeid(signed char))
		{
			if (!(fast_deinterleave = tools::char_itranspose((signed char*)s.data(),
			                                                 (signed char*)s_bis.data(),
			                                                 (int)this->N)))
			{

				throw std::runtime_error("aff3ct::module::Decoder_polar_SC_fast_sys: unsupported \"N\" value for "
				                         "itransposition (N have to be greater or equal to 128 for SSE/NEON or to "
				                         "256 for AVX)");
			}
			else
			{
				// bit unpacking
				auto idx = n_frames * this->K -1;
				for (auto i = n_frames * this->N -1; i > 0; i -= this->N)
				for (unsigned j = (unsigned) 0; j < (unsigned) this->N; j += sizeof(B) * 8)
				{
					unsigned char packed_vals = (unsigned char) s_bis[(i -j) / (sizeof(B) * 8)];
					for (auto k = 0; k < 8; k++)
						if (!frozen_bits[(this->N -1 -j) -k])
							V_K[idx--] = ((packed_vals >> (7-k)) & 0x01);
				}
			}
		}
#endif
		if (!fast_deinterleave)
		{
			// transpose without bit packing (vectorized)
			std::vector<B*> frames(n_frames);
			for (auto f = 0; f < n_frames; f++)
				frames[f] = (B*)(s_bis.data() + f*this->N);
			tools::Reorderer_static<B,n_frames>::apply_rev(s.data(), frames, this->N);

			for (auto i = 0; i < n_frames; i++)
			{
				auto k = 0;
				for (auto j = 0; j < this->N; j++)
					if (!frozen_bits[j])
						V_K[i * this->K + (k++)] = s_bis[i * this->N + j] ? 1 : 0;
			}
		}
	}
}

template <typename B, typename R, class API_polar>
void Decoder_polar_SC_fast_sys<B,R,API_polar>
::_store_fast(mipp::vector<B>& V_N) const
{
	if (V_N.size() < (unsigned) (this->N * this->get_simd_inter_frame_level()))
		throw std::length_error("aff3ct::module::Decoder_polar_SC_fast_sys: \"V_N.size()\" has to be equal or greater "
		                        "than \"N\" * \"get_simd_inter_frame_level()\".");

	constexpr int n_frames = API_polar::get_n_frames();

	if (n_frames == 1)
	{
		std::copy(s.begin(), s.end() - mipp::nElmtsPerRegister<B>(), V_N.begin());
	}
	else
	{
		bool fast_deinterleave = false;
#if defined(ENABLE_BIT_PACKING)
		if (typeid(B) == typeid(signed char))
		{
			if (!(fast_deinterleave = tools::char_itranspose((signed char*)s.data(),
			                                                 (signed char*)V_N.data(),
			                                                 (int)this->N)))
			{
				throw std::runtime_error("aff3ct::module::Decoder_polar_SC_fast_sys: unsupported \"N\" value for "
				                         "itransposition (\"N\" has to be greater or equal to 128 for SSE/NEON or to "
				                         "256 for AVX)");
			}
		}
#endif
		if (!fast_deinterleave)
		{
			// transpose without bit packing (vectorized)
			std::vector<B*> frames(n_frames);
			for (auto f = 0; f < n_frames; f++)
				frames[f] = V_N.data() + f*this->N;
			tools::Reorderer_static<B,n_frames>::apply_rev(s.data(), frames, this->N);
		}
	}
}

template <typename B, typename R, class API_polar>
void Decoder_polar_SC_fast_sys<B,R,API_polar>
::_unpack(mipp::vector<B>& V_N) const
{
	if (V_N.size() < (unsigned) (this->N * this->get_simd_inter_frame_level()))
		throw std::length_error("aff3ct::module::Decoder_polar_SC_fast_sys: \"V_N.size()\" has to be equal or greater "
		                        "than \"N\" * \"get_simd_inter_frame_level()\".");

	constexpr int n_frames = API_polar::get_n_frames();

	bool unpack = false;
#if defined(ENABLE_BIT_PACKING)
	if (typeid(B) == typeid(signed char) && n_frames == mipp::nElReg<R>())
	{
		// bit unpacking
		auto idx = n_frames * this->N -1;
		for (auto i = n_frames * this->N -1; i > 0; i -= this->N)
			for (unsigned j = (unsigned) 0; j < (unsigned) this->N; j += sizeof(B) * 8)
			{
				unsigned char packed_vals = (unsigned char) V_N[(i -j) / (sizeof(B) * 8)];
				for (auto k = 0; k < 8; k++)
					V_N[idx--] = !frozen_bits[(this->N -1 -j) -k] && ((packed_vals >> (7-k)) & 0x01);
			}

		unpack = true;
	}
#endif

	if (!unpack)
		for (auto i = 0; i < n_frames; i++)
			for (auto j = 0; j < this->N; j++)
				V_N[i * this->N + j] = !frozen_bits[j] && V_N[i * this->N + j];
}
}
}
