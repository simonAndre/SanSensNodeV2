/*
File: SanCodedStrings.h
Author: Simon ANDRE
Version: 0.1
Purpose:
*/

#ifndef SanCodedStrings_H_INCLUDED
#define SanCodedStrings_H_INCLUDED

// #include "SanTuple.h"
#include <map>

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

public:
    // build a dictionary from a coded string formatted as `key1:value1;key2:value2;...`
    SanCodedStrings(const char *c_stringmap);
    // build an empty dictionary (use AddKeyValue to populate)
    SanCodedStrings();

    // size of the dictionary
    int Size;

    // // get the whole tuple array
    // SanTuple *getTupleArray();

    // get string value for the key
    const char *getValue(const char *key);

    // get the whole concatenated string coding for all the tuples
    const char *getValue();

    // try to parse as int the value for the key
    bool TryParseValue_i(const char *key, int *outvalue);
    bool TryParseValue_i(const char *key, uint32_t *outvalue);
    bool TryParseValue_i(const char *key, uint8_t *outvalue);

    // try to parse as float the value for the key
    bool TryParseValue_d(const char *key, double *outvalue);

    // try to parse as bool the value for the key (bool value are either coded as 0/1 or true/false)
    bool TryParseValue_b(const char *key, bool *outvalue);

    //ajoute à la structure le nouveau tuple et retourne la nouvelle chaine avec une nouvelle entrée ajoute : `[key]:[value];`
    void AddKeyValue(const char *key, const char *value);
    void AddKeyValue(const char *key, int value);
    void AddKeyValue(const char *key, double value);
};

#endif // SanCodedStrings_H_INCLUDED