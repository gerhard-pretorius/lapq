#ifndef UTEST_FUNCTION_RUNNER_H
#define UTEST_FUNCTION_RUNNER_H

#include <functional>
#include <map>
#include <string_view>

#define ADDFUNC(func) add(#func, func) 

namespace utest {

class FunctionRunner
{
public:
  using Function = std::function<void(int, char**)>;

  void add(const std::string& name, Function func);
  bool run(const std::string_view name, int argc, char** argv);

private:
  std::map<std::string, Function, std::less<>> m_function;
};

} // namespace utest

#endif // UTEST_FUNCTION_RUNNER_H
