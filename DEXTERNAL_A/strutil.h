#pragma once

#include <iostream>
#include <windows.h>
#include <iomanip>
#include <sstream>
#include <vector>
#include <thread>

#define keylen 32

void h2aob(char* hex, char* output, size_t reslen)
{
    for (size_t i = 0; i < reslen * 2; i += 2)
    {
        char hexbyte[3] = { hex[i], hex[i + 1], '\0' };
        output[i / 2] = static_cast<char>(std::stoi(hexbyte, nullptr, 16));
    }
}

void aob2h(char* aob, char* hex, size_t aoblen) {
    for (size_t i = 0; i < aoblen; ++i) {
        sprintf_s(hex + i * 2, 3, "%02X", static_cast<unsigned char>(aob[i]));
    }
}

bool b16(std::string ed)
{
    for (size_t i = 0; i < ed.length(); i++)
    {
        // if (not hex)
        if (!((ed[i] >= '0' && ed[i] <= '9') || (ed[i] >= 'a' && ed[i] <= 'f') || (ed[i] >= 'A' && ed[i] <= 'F') || (ed[i] == ':')))
        {
            return false;
        }
    }
    return true;
}

size_t GetHexLen(char* string, size_t len)
{
    int returnbuffer = 0;
    int colon = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (string[i] == ':')
        {
            colon = i;
            break;
        }
    }

    returnbuffer = (len - colon) - 1;
    return returnbuffer;
}

void splitter(char* in, size_t len, char* out1, char* out2)
{
    bool passedcolon = false;
    int colonspot = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (in[i] == ':')
        {
            passedcolon = true;
            colonspot = i;
            continue; // skip the below and continue the loop
        }

        if (passedcolon == true)
        {
            out2[(i - colonspot) - 1] = in[i];
        }
        else
        {
            out1[i] = in[i];
        }
    }
}

void joiner(char* in1, size_t in1len, char* in2, size_t in2len, char* out)
{
    for (size_t i = 0; i < in1len; i++)
    {
        out[i] = in1[i];
    }
    out[in1len] = ':';
    for (size_t i = 0; i < in2len; i++)
    {
        out[i + in1len + 1] = in2[i];
    }
}

void DTXR(char* encr, char* key, size_t Len)
{
    for (size_t i = 0; i < Len; i++)
    {
        encr[i] = encr[i] ^ ((key[i % keylen] ^ (key[(i << 3 * 5) % keylen] ^ key[(i % 0x400 >> 3 * key[(i >> 8) % keylen]) % keylen] | key[(i << 63) % keylen] ^ key[(i + 0x40 + key[(i + 0x40) % keylen]) % keylen] ^ key[(i >> 1 + i << 5 + key[(key[(i << 16) % keylen]) % keylen]) % keylen])));
    }
}

uint64_t XHASH(uint64_t input)
{
    uint64_t buffer = input;

    std::cout << buffer;

    for (size_t i = 0; i < 6; i++)
    {
        buffer += ~buffer + buffer - buffer >> 3 + buffer << 2 + buffer + buffer & 0x100 + buffer ^ (buffer & 40);
    }

    buffer += input;
    // for our purposes we only want a char worth of bytes to gen the sudorandom key
    buffer = buffer >> (buffer >> (64 * 8 - 8));

    return buffer;
}
