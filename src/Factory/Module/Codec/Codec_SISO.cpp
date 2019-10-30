#include "Factory/Module/Codec/LDPC/Codec_LDPC.hpp"
#include "Factory/Module/Codec/Polar/Codec_polar.hpp"
#include "Factory/Module/Codec/RSC/Codec_RSC.hpp"
#include "Factory/Module/Codec/RSC_DB/Codec_RSC_DB.hpp"
#include "Factory/Module/Codec/Uncoded/Codec_uncoded.hpp"
#include "Factory/Module/Codec/Turbo_product/Codec_turbo_product.hpp"
#include "Factory/Module/Codec/Codec_SISO.hpp"

using namespace aff3ct;
using namespace aff3ct::factory;

const std::string aff3ct::factory::Codec_SISO_name   = "Codec SISO";
const std::string aff3ct::factory::Codec_SISO_prefix = "cdc";

Codec_SISO
::Codec_SISO(const std::string &prefix)
: Codec(Codec_SISO_name, prefix)
{
}

Codec_SISO
::Codec_SISO(const std::string &name, const std::string &prefix)
: Codec(name, prefix)
{
}

template <typename B, typename Q>
module::Codec_SISO<B,Q>* Codec_SISO
::build(module::CRC<B>* crc) const
{
	if (get_name() == Codec_LDPC_name         ) return dynamic_cast<const Codec_LDPC         &>(*this).template build<B,Q>(crc);
	if (get_name() == Codec_polar_name        ) return dynamic_cast<const Codec_polar        &>(*this).template build<B,Q>(crc);
	if (get_name() == Codec_RSC_name          ) return dynamic_cast<const Codec_RSC          &>(*this).template build<B,Q>(crc);
	if (get_name() == Codec_RSC_DB_name       ) return dynamic_cast<const Codec_RSC_DB       &>(*this).template build<B,Q>(crc);
	if (get_name() == Codec_uncoded_name      ) return dynamic_cast<const Codec_uncoded      &>(*this).template build<B,Q>(crc);
	if (get_name() == Codec_turbo_product_name) return dynamic_cast<const Codec_turbo_product&>(*this).template build<B,Q>(crc);

	throw tools::cannot_allocate(__FILE__, __LINE__, __func__);
}

// ==================================================================================== explicit template instantiation
#include "Tools/types.h"
#ifdef AFF3CT_MULTI_PREC
template aff3ct::module::Codec_SISO<B_8 ,Q_8 >* aff3ct::factory::Codec_SISO::build<B_8 ,Q_8 >(aff3ct::module::CRC<B_8 >*) const;
template aff3ct::module::Codec_SISO<B_16,Q_16>* aff3ct::factory::Codec_SISO::build<B_16,Q_16>(aff3ct::module::CRC<B_16>*) const;
template aff3ct::module::Codec_SISO<B_32,Q_32>* aff3ct::factory::Codec_SISO::build<B_32,Q_32>(aff3ct::module::CRC<B_32>*) const;
template aff3ct::module::Codec_SISO<B_64,Q_64>* aff3ct::factory::Codec_SISO::build<B_64,Q_64>(aff3ct::module::CRC<B_64>*) const;
#else
template aff3ct::module::Codec_SISO<B,Q>* aff3ct::factory::Codec_SISO::build<B,Q>(aff3ct::module::CRC<B>*) const;
#endif
// ==================================================================================== explicit template instantiation
