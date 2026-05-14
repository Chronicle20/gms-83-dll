/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see
 <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <cstdio>
#include <exception>

class CTerminateException : public std::exception {
  public:
    explicit CTerminateException(int nCode) noexcept : m_nCode(nCode) {
        std::snprintf(m_szBuf, sizeof(m_szBuf), "CTerminateException(%d)", nCode);
    }

    const char* what() const noexcept override {
        return m_szBuf;
    }

    int m_nCode;

  private:
    char m_szBuf[64];
};

class CPatchException : public std::exception {
  public:
    const char* what() const noexcept override {
        return "CPatchException";
    }
};
