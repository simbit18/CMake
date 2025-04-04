/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/**
 * @brief Result of the locking/unlocking file.
 * @note See @c cmFileLock
 */
class cmFileLockResult
{
public:
  using Error = int;

  /**
   * @brief Successful lock/unlock.
   */
  static cmFileLockResult MakeOk();

  /**
   * @brief Lock/Unlock failed. Read error/GetLastError.
   */
  static cmFileLockResult MakeSystem();

  /**
   * @brief Lock/Unlock failed. Timeout reached.
   */
  static cmFileLockResult MakeTimeout();

  /**
   * @brief File already locked.
   */
  static cmFileLockResult MakeAlreadyLocked();

  /**
   * @brief Internal error.
   */
  static cmFileLockResult MakeInternal();

  /**
   * @brief Try to lock with function guard outside of the function
   */
  static cmFileLockResult MakeNoFunction();

  bool IsOk() const;
  std::string GetOutputMessage() const;

private:
  enum ErrorType
  {
    OK,
    SYSTEM,
    TIMEOUT,
    ALREADY_LOCKED,
    INTERNAL,
    NO_FUNCTION
  };

  cmFileLockResult(ErrorType type, Error errorValue);

  ErrorType Type;
  Error ErrorValue;
};
