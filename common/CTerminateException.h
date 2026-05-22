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
