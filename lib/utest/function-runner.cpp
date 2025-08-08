#include "function-runner.h"

namespace utest {

void FunctionRunner::add(const std::string& name, Function func)
{
  m_function[name] = func;
}

bool FunctionRunner::run(const std::string_view name, int argc, char** argv)
{
  auto it = m_function.find(name);

  if (it != m_function.end()) {
    it->second(argc, argv);
    return true;
  }

  return false;
}

} // namespace utest
