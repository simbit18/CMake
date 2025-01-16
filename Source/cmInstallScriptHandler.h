/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "cmUVHandlePtr.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"

class cmInstrumentation;

class cmInstallScriptHandler
{
public:
  cmInstallScriptHandler() = default;
  cmInstallScriptHandler(std::string, std::string, std::string,
                         std::vector<std::string>&);
  bool IsParallel();
  int Install(unsigned int j, cmInstrumentation& instrumentation);
  std::vector<std::vector<std::string>> GetCommands() const;
  class InstallScript
  {
  public:
    InstallScript(const std::vector<std::string>&);
    void start(cm::uv_loop_ptr&, std::function<void()>);
    void printResult(std::size_t n, std::size_t total);

  private:
    std::vector<std::string> command;
    std::vector<std::string> output;
    std::string name;
    std::unique_ptr<cmUVProcessChain> chain;
    std::unique_ptr<cmUVStreamReadHandle> streamHandler;
    cm::uv_pipe_ptr pipe;
  };

private:
  std::vector<std::vector<std::string>> commands;
  std::vector<std::string> directories;
  std::vector<std::string> configs;
  std::string binaryDir;
  std::string component;
  bool parallel;
};
