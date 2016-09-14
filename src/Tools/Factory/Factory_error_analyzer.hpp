#ifndef FACTORY_ERROR_ANALYZER_HPP
#define FACTORY_ERROR_ANALYZER_HPP

#include "Tools/params.h"

#include "Module/Error/Error_analyzer.hpp"

template <typename B>
struct Factory_error_analyzer
{
	static Error_analyzer<B>* build(const parameters &params, const int n_frames = 1);
};

#endif /* FACTORY_ERROR_ANALYZER_HPP */
