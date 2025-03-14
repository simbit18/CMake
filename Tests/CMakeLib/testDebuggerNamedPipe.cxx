/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <chrono>
#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/cppdap/io.h>

#include "cmsys/RegularExpression.hxx"

#ifdef _WIN32
#  include "cmDebuggerWindowsPipeConnection.h"
#else
#  include "cmDebuggerPosixPipeConnection.h"
#endif

#include "cmSystemTools.h"

#ifdef _WIN32
#  include "cmCryptoHash.h"
#endif

static void sendCommands(std::shared_ptr<dap::ReaderWriter> const& debugger,
                         int delayMs,
                         std::vector<std::string> const& initCommands)
{
  for (auto const& command : initCommands) {
    std::string contentLength = "Content-Length:";
    contentLength += std::to_string(command.size()) + "\r\n\r\n";
    debugger->write(contentLength.c_str(), contentLength.size());
    if (!debugger->write(command.c_str(), command.size())) {
      std::cout << "debugger write error" << std::endl;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
  }
}

/** \brief Test CMake debugger named pipe.
 *
 * Test CMake debugger named pipe by
 * 1. Create a named pipe for DAP traffic between the client and the debugger.
 * 2. Create a client thread to wait for the debugger connection.
 *    - Once the debugger is connected, send the minimum required commands to
 *      get debugger going.
 *    - Wait for the CMake to complete the cache generation
 *    - Send the disconnect command.
 *    - Read and store the debugger's responses for validation.
 * 3. Run the CMake command with debugger on and wait for it to complete.
 * 4. Validate the response to ensure we are getting the expected responses.
 *
 */
int runTest(int argc, char* argv[])
{
  if (argc < 4) {
    std::cout << "Usage:\n";
    std::cout << "\t(project mode) TestDebuggerNamedPipe <CMakePath> -S "
                 "<SourceFolder> -B <OutputFolder> ...\n";
    std::cout << "\t(script mode) TestDebuggerNamedPipe <CMakePath> -P "
                 "<ScriptPath>\n";
    return 1;
  }

#ifdef _WIN32
  std::string namedPipe = R"(\\.\pipe\LOCAL\CMakeDebuggerPipe_)" +
    cmCryptoHash(cmCryptoHash::AlgoSHA256).HashString(argv[2]);
#else
  bool const scriptMode = argv[2] == "-P"_s;
  std::string namedPipe =
    std::string("CMakeDebuggerPipe") + (scriptMode ? "Script" : "Project");
#endif

  std::vector<std::string> cmakeCommand;
  cmakeCommand.emplace_back(argv[1]);
  cmakeCommand.emplace_back("--debugger");
  cmakeCommand.emplace_back("--debugger-pipe");
  cmakeCommand.emplace_back(namedPipe);
  cmakeCommand.insert(cmakeCommand.end(), argv + 2, argv + argc);

  // Capture debugger response stream.
  std::stringstream debuggerResponseStream;

  // Start the debugger client process.
  std::thread clientThread([&]() {
    // Poll until the pipe server is running. Clients can also look for a magic
    // string in the CMake output, but this is easier for the test case.
    std::shared_ptr<cmDebugger::cmDebuggerPipeClient> client;
    int attempt = 0;
    do {
      attempt++;
      try {
        client = std::make_shared<cmDebugger::cmDebuggerPipeClient>(namedPipe);

        client->WaitForConnection();
        std::cout << "cmDebuggerPipeClient connected.\n";
        break;
      } catch (std::runtime_error&) {
        std::cout << "Failed attempt " << attempt
                  << " to connect to pipe server. Retrying.\n";
        client.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
      }
    } while (attempt < 50); // 10 seconds

    if (attempt >= 50) {
      return -1;
    }

    // Send init commands to get debugger going.
    sendCommands(
      client, 400,
      { "{\"arguments\":{\"adapterID\":\"\"},\"command\":\"initialize\","
        "\"seq\":"
        "1,\"type\":\"request\"}",
        "{\"arguments\":{},\"command\":\"launch\",\"seq\":2,\"type\":"
        "\"request\"}",
        "{\"arguments\":{},\"command\":\"configurationDone\",\"seq\":3,"
        "\"type\":"
        "\"request\"}" });

    // Look for "exitCode" as a sign that configuration has completed and
    // it's now safe to disconnect.
    for (;;) {
      char buffer[1];
      size_t result = client->read(buffer, 1);
      if (result != 1) {
        std::cout << "debugger read error: " << result << std::endl;
        break;
      }
      debuggerResponseStream << buffer[0];
      if (debuggerResponseStream.str().find("exitCode") != std::string::npos) {
        break;
      }
    }

    // Send disconnect command.
    sendCommands(
      client, 200,
      { "{\"arguments\":{},\"command\":\"disconnect\",\"seq\":4,\"type\":"
        "\"request\"}" });

    // Read any remaining debugger responses.
    for (;;) {
      char buffer[1];
      size_t result = client->read(buffer, 1);
      if (result != 1) {
        std::cout << "debugger read error: " << result << std::endl;
        break;
      }
      debuggerResponseStream << buffer[0];
    }

    client->close();

    return 0;
  });

  if (!cmSystemTools::RunSingleCommand(cmakeCommand, nullptr, nullptr, nullptr,
                                       nullptr, cmSystemTools::OUTPUT_MERGE)) {
    std::cout << "Error running command" << std::endl;
    return -1;
  }

  clientThread.join();

  auto debuggerResponse = debuggerResponseStream.str();

  std::vector<std::string> expectedResponses = {
    R"("event" *: *"initialized".*"type" *: *"event")",
    R"("command" *: *"launch".*"success" *: *true.*"type" *: *"response")",
    R"("command" *: *"configurationDone".*"success" *: *true.*"type" *: *"response")",
    R"("reason" *: *"started".*"threadId" *: *1.*"event" *: *"thread".*"type" *: *"event")",
    R"("reason" *: *"exited".*"threadId" *: *1.*"event" *: *"thread".*"type" *: *"event")",
    R"("exitCode" *: *0.*"event" *: *"exited".*"type" *: *"event")",
    R"("command" *: *"disconnect".*"success" *: *true.*"type" *: *"response")"
  };

  for (auto const& regexString : expectedResponses) {
    cmsys::RegularExpression regex(regexString);
    if (!regex.find(debuggerResponse)) {
      std::cout << "Expected response not found: " << regexString << std::endl;
      std::cout << debuggerResponse << std::endl;
      return -1;
    }
  }

  return 0;
}

int main(int argc, char* argv[])
{
  try {
    return runTest(argc, argv);
  } catch (std::exception const& ex) {
    std::cout << "An exception occurred: " << ex.what() << std::endl;
    return -1;
  } catch (std::string const& ex) {
    std::cout << "An exception occurred: " << ex << std::endl;
    return -1;
  } catch (...) {
    std::cout << "An unknown exception occurred" << std::endl;
    return -1;
  }
}
