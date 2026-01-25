#pragma once
#ifndef CLOUDHTTP_H
#define CLOUDHTTP_H

#include "stdafx.h"
#include <cpr/cpr.h>

// ========================================================
//  CloudHttp.h
//  Tüm cloud isteklerinde TEK referans header katmanı
// ========================================================

// ---------------- BASIC ----------------

inline cpr::Header MakeJsonHeader()
{
    return {
        { "Content-Type", "application/json" }
    };
}

inline cpr::Header MakeFormHeader()
{
    return {
        { "Content-Type", "application/x-www-form-urlencoded" }
    };
}

inline cpr::Header MakeOctetHeader()
{
    return {
        { "Content-Type", "application/octet-stream" }
    };
}

// ---------------- AUTH ----------------

inline cpr::Header MakeAuthHeader(const std::string& accessToken)
{
    return {
        { "Authorization", std::string("Bearer ") + accessToken }
    };
}

inline cpr::Header MakeAuthJsonHeader(const std::string& accessToken)
{
    return {
        { "Authorization", std::string("Bearer ") + accessToken },
        { "Content-Type", "application/json" }
    };
}

inline cpr::Header MakeAuthFormHeader(const std::string& accessToken)
{
    return {
        { "Authorization", std::string("Bearer ") + accessToken },
        { "Content-Type", "application/x-www-form-urlencoded" }
    };
}

inline cpr::Header MakeAuthOctetHeader(const std::string& accessToken)
{
    return {
        { "Authorization", std::string("Bearer ") + accessToken },
        { "Content-Type", "application/octet-stream" }
    };
}

// ---------------- MERGE (opsiyonel ama çok işe yarar) ----------------

inline cpr::Header MergeHeaders(const cpr::Header& a, const cpr::Header& b)
{
    cpr::Header out = a;
    for (auto& it : b)
        out[it.first] = it.second;
    return out;
}

#endif // CLOUDHTTP_H
