#include "parse_ini.h"

#include <cstdio>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

namespace {

std::string WriteTempFile(const std::string& body) {
    char tmpl[L_tmpnam];
    std::tmpnam(tmpl);
    std::string path = tmpl;
    std::ofstream f(path);
    f << body;
    f.close();
    return path;
}

} // namespace

TEST(ParseIni, ValidKeyValuePair) {
    auto path = WriteTempFile("[Main]\nfoo=bar\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    auto it = p.entries.find("Main.foo");
    ASSERT_NE(it, p.entries.end());
    ASSERT_EQ(it->second.size(), 1u);
    EXPECT_EQ(it->second[0], "bar");
}

TEST(ParseIni, LeadingAndTrailingWhitespaceIsTrimmed) {
    auto path = WriteTempFile("[Main]\n   key   =   value   \n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    auto it = p.entries.find("Main.key");
    ASSERT_NE(it, p.entries.end());
    EXPECT_EQ(it->second.front(), "value");
}

TEST(ParseIni, SemicolonComment) {
    auto path = WriteTempFile("[Main]\nfoo=bar ; trailing comment\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    EXPECT_EQ(p.entries.at("Main.foo").front(), "bar");
}

TEST(ParseIni, HashComment) {
    auto path = WriteTempFile("[Main]\nfoo=bar # trailing comment\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    EXPECT_EQ(p.entries.at("Main.foo").front(), "bar");
}

TEST(ParseIni, DuplicateKeysAccumulate) {
    auto path = WriteTempFile("[Main]\nip=1.2.3.4\nip=5.6.7.8\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    auto& vals = p.entries.at("Main.ip");
    ASSERT_EQ(vals.size(), 2u);
    EXPECT_EQ(vals[0], "1.2.3.4");
    EXPECT_EQ(vals[1], "5.6.7.8");
}

TEST(ParseIni, MalformedLineNoEquals) {
    auto path = WriteTempFile("[Main]\nthis line has no equals\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    EXPECT_TRUE(p.entries.empty());
}

TEST(ParseIni, MalformedLineEmptyKey) {
    auto path = WriteTempFile("[Main]\n=value\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    EXPECT_TRUE(p.entries.empty());
}

TEST(ParseIni, MissingFileReturnsFalse) {
    ms::ini::Parsed p;
    EXPECT_FALSE(ms::ini::Parse("/nonexistent/path/never-created.ini", p));
}

TEST(ParseIni, LogSinkReceivesMalformedLineMessages) {
    auto path = WriteTempFile("[Main]\nlonely\n");
    ms::ini::Parsed p;
    int callCount = 0;
    auto sink = [&](const char* msg) {
        ++callCount;
        EXPECT_NE(std::string(msg).find("malformed"), std::string::npos);
    };
    ASSERT_TRUE(ms::ini::Parse(path, p, sink));
    EXPECT_EQ(callCount, 1);
}
