// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

#pragma once
#include <codecvt>
#include <pdhmsg.h>
#include <regex>

static Common::RwLock boostTaefBuffersLock;
static Common::RwLock boostVerifyLock;
static std::map<DWORD, std::string> boostTaefBuffers;

inline static boost::unit_test::const_string BoostTaef_UnicodeToAnsi(WCHAR const * ws = nullptr)
{
    if (ws == nullptr) return boost::unit_test::const_string();

#ifdef PLATFORM_UNIX
    std::string ansiString = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes((const char16_t*)ws);
#else
    std::string ansiString = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>().to_bytes((const unsigned short*)ws);
#endif
    auto tid = ::GetCurrentThreadId();

    Common::AcquireWriteLock grab(boostTaefBuffersLock);
    boostTaefBuffers[tid] = std::move(ansiString);
    return boost::unit_test::const_string();
}

inline static bool boost_taef_IsDebug()
{
#ifdef DBG
        return true;
#else
        return false;
#endif
}

#define VERIFY_IS_TRUE(e, ...) \
{\
    auto testMsg_MustNotCollideWithMacroParameterName = BoostTaef_UnicodeToAnsi(__VA_ARGS__); \
    if (testMsg_MustNotCollideWithMacroParameterName.empty()) {\
        if (boost_taef_IsDebug()) {\
            if (!(e)) { Common::Assert::CodingError("e"); }\
        } else {\
            BOOST_REQUIRE(e);\
        }\
    } else {\
         if (boost_taef_IsDebug()) {\
            if (!(e)) { Common::Assert::CodingError("e"); }\
        } else {\
            BOOST_REQUIRE_MESSAGE(e, testMsg_MustNotCollideWithMacroParameterName);\
        }\
    }\
}

#define VERIFY_IS_FALSE(e, ...) VERIFY_IS_TRUE(!(e), __VA_ARGS__)

#define VERIFY_ARE_EQUAL(a, b, ...) VERIFY_IS_TRUE((a) == (b), __VA_ARGS__)
#define VERIFY_ARE_EQUAL2(a, b) { auto vl = (a); auto vr = (b); VERIFY_IS_TRUE(vl == vr, Common::wformatString("VERIFY_ARE_EQUAL({0}={1}, {2}={3})", #a, vl, #b, vr).c_str()); }

#define VERIFY_ARE_NOT_EQUAL(a, b, ...) VERIFY_IS_TRUE((a) != (b), __VA_ARGS__)
#define VERIFY_ARE_NOT_EQUAL2(a, b) { auto vl = (a); auto vr = (b); VERIFY_IS_TRUE(vl != vr, Common::wformatString("VERIFY_ARE_NOT_EQUAL({0}={1}, {2}={3})", #a, vl, #b, vr).c_str()); }

#define VERIFY_IS_NULL(p, ...) VERIFY_IS_TRUE((p) == nullptr, __VA_ARGS__)
#define VERIFY_IS_NOT_NULL(p, ...) VERIFY_IS_TRUE((p) != nullptr, __VA_ARGS__)

#define VERIFY_SUCCEEDED(hr, ...) VERIFY_IS_TRUE(SUCCEEDED(hr), __VA_ARGS__)
#define VERIFY_FAILED(hr, ...) VERIFY_IS_TRUE(FAILED(hr), __VA_ARGS__)

#define VERIFY_FAIL(m, ...) VERIFY_IS_TRUE(false, m)

#define VERIFY_THROWS(o, e, ...) BOOST_REQUIRE_THROW(o, e)


#define TEST_METHOD_SETUP(f) bool f();
#define TEST_METHOD_CLEANUP(f) bool f();

#define TEST_CLASS_SETUP(f) bool f();
#define TEST_CLASS_CLEANUP(f) bool f();

#define BOOST_WMESSAGE(m) \
{\
    auto msg = BoostTaef_UnicodeToAnsi(m); \
    BOOST_TEST_MESSAGE(msg);\
}

#define BOOST_WMESSAGE_FORMAT(...) \
{\
    auto msg = BoostTaef_UnicodeToAnsi(wformatString(__VA_ARGS__).c_str()); \
    BOOST_TEST_MESSAGE(msg);\
}

#define VERIFY_IS_TRUE_FMT(condition, ...) VERIFY_IS_TRUE(condition, Common::wformatString(__VA_ARGS__).c_str())
#define VERIFY_IS_FALSE_FMT(condition, ...) VERIFY_IS_FALSE(condition, Common::wformatString(__VA_ARGS__).c_str())
#define VERIFY_ARE_EQUAL_FMT(a, b, ...) VERIFY_IS_TRUE((a) == (b), Common::wformatString(__VA_ARGS__).c_str())
#define VERIFY_FAIL_FMT(...) VERIFY_FAIL(Common::wformatString(__VA_ARGS__).c_str())

// The following is defined for manual conversion of taef tests, not needed for code generated by the conversion tool
#define TEST_CLASS(testClass)
#define TEST_METHOD(testMethod)

#define TRIGGER_CONFIG_LOADING \
namespace { struct TriggerConfigLoading { TriggerConfigLoading() { Config config; } }; } \
BOOST_GLOBAL_FIXTURE(TriggerConfigLoading)

#define BOOST_AUTO_TEST_SUITE2(testSuite) \
    Common::StringLiteral const TraceType(#testSuite); \
    TRIGGER_CONFIG_LOADING; \
    BOOST_AUTO_TEST_SUITE(testSuite)

#define BOOST_FIXTURE_TEST_SUITE2(testSuite, testCaseBaseClass) \
    Common::StringLiteral const TraceType(#testSuite); \
    TRIGGER_CONFIG_LOADING; \
    BOOST_FIXTURE_TEST_SUITE(testSuite, testCaseBaseClass)

#define BoostTestTrace "Test"

#ifdef PLATFORM_UNIX
#define FUNCTION_TRACE __PRETTY_FUNCTION__ // clang __FUNCTION__ does not contain class name
#else
#define FUNCTION_TRACE __FUNCTION__
#endif

#define ENTER Trace.WriteInfo(BoostTestTrace, "Enter test: {0}", FUNCTION_TRACE); {
#define LEAVE } Trace.WriteInfo(BoostTestTrace, "Leave test: {0}", FUNCTION_TRACE)

#ifdef PLATFORM_UNIX

#define RetryOnEIntr(retval, f) \
    for(;;) \
    { \
        retval = (f); \
        if (retval < 0) \
        { \
            Trace.WriteInfo("RetryOnEIntr", "retval = {0}, errno = {1}", retval, errno); \
            Invariant(errno == EINTR); \
            continue; \
        }\ 
        break;\
    }

#endif
