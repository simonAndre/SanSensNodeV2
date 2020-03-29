/*
File: SanCodedStrings.h
Author: Simon ANDRE
Version: 0.1
Purpose:
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <map>
#include "SanCodedStrings.h"
std::map<std::string, std::string> ParseString(const char *c_str);

SanCodedStrings::SanCodedStrings(const char *c_stringmap)
{
    this->_sizestr = strlen(c_stringmap);
    this->_map = ParseString(c_stringmap);
    Size = _map.size();
}

SanCodedStrings::SanCodedStrings()
{
    this->_sizestr = 0;
    Size = 0;
}

short sizeKvCodedString(const char *k, const char *v)
{
    return (short)strlen(k) + (short)strlen(v) + 3;
}

char *codeKeyValue(const char *k, const char *v, int *kvsize)
{
    *kvsize = sizeKvCodedString(k, v);
    char *kvstr = (char *)malloc(sizeof(char) * *kvsize);
    char *p = kvstr;

    for (size_t i = 0; i < strlen(k); i++)
        *p++ = k[i];
    *p++ = ':';
    for (size_t i = 0; i < strlen(v); i++)
        *p++ = v[i];
    *p++ = ';';
    *p = '\0';
    return kvstr;
}

const char *SanCodedStrings::getValue()
{
    if (this->_stringmap.empty())
    {
        for (std::map<std::string, std::string>::iterator it = _map.begin(); it != _map.end(); ++it)
        {
            this->_stringmap.append(it->first);
            this->_stringmap.append(1, ':');
            this->_stringmap.append(it->second);
            this->_stringmap.append(1, ';');
        }
    }
    return this->_stringmap.c_str();
}

std::map<std::string, std::string> ParseString(const char *c_str)
{
    std::map<std::string, std::string> _umap;
    std::string mystr(c_str);
    size_t p = 0, lastp = 0;
    short ix = 0, poskvsep = 0;
    p = mystr.find(';', lastp);
    while (p != std::string::npos)
    {
        std::string tok = mystr.substr(lastp, p - lastp);
        poskvsep = tok.find(':');
        if (poskvsep < std::string::npos)
        {
            std::string key = tok.substr(0, poskvsep);
            std::string val = tok.substr(poskvsep + 1, tok.size() - poskvsep - 1);
            _umap[key] = val;
        }
        // std::cout << "pos" << ix << " : " << p << " --> " << tok << '\n';
        lastp = p + 1;
        p = mystr.find(';', lastp + 1);
        ix++;
    }
    return _umap;
}

void SanCodedStrings::AddKeyValue(const char *key, const char *value)
{
    Size++;
    _map[key] = value;
    this->_sizestr += sizeKvCodedString(key, value);
    this->_stringmap.clear(); // on modifie les entres=> la chaine de codage sous-jacante devra être recalculée
}

void SanCodedStrings::AddKeyValue(const char *key, int value)
{
    char s_value[10];
    sprintf(s_value, "%i", value);
    AddKeyValue(key, s_value);
}
void SanCodedStrings::AddKeyValue(const char *key, double value)
{
    char s_value[15];
    sprintf(s_value, "%.3f", value);
    AddKeyValue(key, s_value);
}
// get value for the key "key"
const char *SanCodedStrings::getValue(const char *key)
{
    std::map<std::string, std::string>::iterator it;
    std::string s(key);
    it = _map.find(key);
    if (it != _map.end())
        return it->second.c_str();
    return NULL;
}
bool SanCodedStrings::TryParseValue_i(const char *key, uint32_t *outvalue)
{
    TryParseValue_i(key, (int *)&outvalue);
}
bool SanCodedStrings::TryParseValue_i(const char *key, uint8_t *outvalue)
{
    TryParseValue_i(key, (int *)&outvalue);
}
bool SanCodedStrings::TryParseValue_i(const char *key, int *outvalue)
{
    const char *s_value = this->getValue(key);
    if (s_value != NULL && strlen(s_value) > 0)
    {
        *outvalue = atoi(s_value);
        return true;
    }
    return false;
}

bool SanCodedStrings::TryParseValue_d(const char *key, double *outvalue)
{
    const char *s_value = this->getValue(key);
    if (s_value != NULL && strlen(s_value) > 0)
    {
        *outvalue = atof(s_value);
        return true;
    }
    return false;
}

bool SanCodedStrings::TryParseValue_b(const char *key, bool *outvalue)
{
    const char *s_value = this->getValue(key);
    if (s_value != NULL && strlen(s_value) > 0)
    {
        if (strcmp(s_value, "1") == 0 || strcmp(s_value, "true") == 0)
        {
            *outvalue = true;
            return true;
        }
        if (strcmp(s_value, "0") == 0 || strcmp(s_value, "false") == 0)
        {
            *outvalue = false;
            return true;
        }
    }
    return false;
}
