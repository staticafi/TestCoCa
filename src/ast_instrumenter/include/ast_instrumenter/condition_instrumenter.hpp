#pragma once

#include <string>
#include <vector>

namespace ast_instrumenter {

bool instrument_conditions(const std::string& input_path,
                           const std::string& output_path,
                           const std::vector<std::string>& compiler_args);

}
