#pragma once

namespace SANSENSNODE_NAMESPACE
{

// parse la chaine stringmap en un tableau de clef/valeurs (dictionaire) : [outdic]
// retourne la taille du tableau (nb de key/value)
// la chaine doit être formatée ainsi : key1:value1;key2:value2;...
class SanCodedStrings
{

private:
    std::map<std::string, std::string> _map;
    std::string _stringmap;

    //size of the underlying coded string
    size_t _sizestr;

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

    short sizeKvCodedString(const char *k, const char *v)
    {
        return (short)strlen(k) + (short)strlen(v) + 3;
    }

public:
    // build a dictionary from a coded string formatted as `key1:value1;key2:value2;...`
    SanCodedStrings(const char *c_stringmap)
    {
        this->_sizestr = strlen(c_stringmap);
        this->_map = ParseString(c_stringmap);
        Size = _map.size();
    }

    // build an empty dictionary (use AddKeyValue to populate)
    SanCodedStrings()
    {
        this->_sizestr = 0;
        Size = 0;
    }

    // size of the dictionary
    int Size;

    // // get the whole tuple array
    // SanTuple *getTupleArray();

    // get string value for the key
    const char *getValue(const char *key)
    {
        std::map<std::string, std::string>::iterator it;
        std::string s(key);
        it = _map.find(key);
        if (it != _map.end())
            return it->second.c_str();
        return NULL;
    }

    // get the whole concatenated string coding for all the tuples
    const char *getValue()
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

    // try to parse as int the value for the key
    bool TryParseValue_i(const char *key, int *outvalue)
    {
        const char *s_value = this->getValue(key);
        if (s_value != NULL && strlen(s_value) > 0)
        {
            *outvalue = atoi(s_value);
            return true;
        }
        return false;
    }

    bool TryParseValue_i(const char *key, uint32_t *outvalue)
    {
        TryParseValue_i(key, (int *)&outvalue);
    }

    bool TryParseValue_i(const char *key, uint8_t *outvalue)
    {
        TryParseValue_i(key, (int *)&outvalue);
    }

    // try to parse as float the value for the key
    bool TryParseValue_d(const char *key, double *outvalue)
    {
        const char *s_value = this->getValue(key);
        if (s_value != NULL && strlen(s_value) > 0)
        {
            *outvalue = atof(s_value);
            return true;
        }
        return false;
    }

    // try to parse as bool the value for the key (bool value are either coded as 0/1 or true/false)
    bool TryParseValue_b(const char *key, bool *outvalue)
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

    //ajoute à la structure le nouveau tuple et retourne la nouvelle chaine avec une nouvelle entrée ajoute : `[key]:[value];`
    void AddKeyValue(const char *key, const char *value)
    {
        Size++;
        _map[key] = value;
        this->_sizestr += sizeKvCodedString(key, value);
        this->_stringmap.clear(); // on modifie les entres=> la chaine de codage sous-jacante devra être recalculée
    }

    void AddKeyValue(const char *key, int value)
    {
        char s_value[10];
        sprintf(s_value, "%i", value);
        AddKeyValue(key, s_value);
    }

    void AddKeyValue(const char *key, double value)
    {
        char s_value[15];
        sprintf(s_value, "%.3f", value);
        AddKeyValue(key, s_value);
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
};
} // namespace SANSENSNODE_NAMESPACE
