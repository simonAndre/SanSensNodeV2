#pragma once
#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace SANSENSNODE_NAMESPACE
{
    template <typename T>
    class SanPair : public std::pair<std::string, T>
    {
    private:
    public:
        SanPair() : std::pair<std::string, T>(){};
        SanPair(const std::string &key, const T &value) : std::pair<std::string, T>(key, value){};

        template <typename V>
        friend std::ostream &operator<<(std::ostream &os, const SanPair<V> &pkv);

        uint16_t forcastedjsonsize();
    };

    void prependStream(std::ostream &os, const std::string &key)
    {
        if (os.tellp() == 0)
            os << '{';
        else if (os.tellp() > 1)
            os << ',';
        os << '\'' << key << "':";
    }

    template <typename V>
    std::ostream &operator<<(std::ostream &os, const SanPair<V> &pkv)
    {
        prependStream(os, pkv.first);
        os << pkv.second;
        return os;
    }

    template <>
    std::ostream &operator<<(std::ostream &os, const SanPair<std::string> &pkv)
    {
        prependStream(os, pkv.first);
        os << '\'' << pkv.second << '\'';
        return os;
    }
    template <>
    std::ostream &operator<<(std::ostream &os, const SanPair<const char *> &pkv)
    {
        prependStream(os, pkv.first);
        os << '\'' << pkv.second << '\'';
        return os;
    }

    template <typename V>
    uint16_t SanPair<V>::forcastedjsonsize()
    {
        std::ostringstream oss;
        oss << this->second;
        uint16_t skey = this->first.size();
        uint16_t svvalue = oss.tellp();
        uint16_t siz = skey + svvalue + 5;
        if (typeid(this->second) == typeid(const char *) || typeid(this->second) == typeid(std::string))
            siz += 2;
        return siz;
    }

    /**
 * @brief build a collection of json string from key-value pairs. Each json string beeing big enough to fit the JsonMaxCapacity template parameter.
 * 
 * @tparam size max of the json string. If needed, several string will be created wrapped in a vactor.  
 */
    template <size_t JsonMaxCapacity = 106>
    class JsonStream
    {
    private:
        std::ostringstream *o_currentjsonstream;
        std::vector<std::string> jsoncollection;

    public:
        JsonStream()
        {
            o_currentjsonstream = new std::ostringstream();
        }
        ~JsonStream()
        {
            delete o_currentjsonstream;
        }

        std::vector<std::string> getJsons()
        {
            this->flush();
            return this->jsoncollection;
        }

        /**
     * @brief number of json strings holded by the collection
     * 
     * @return uint16_t 
     */
        uint16_t size()
        {
            return jsoncollection.size();
        }

        /**
 * @brief add the key/value pair to the json stream
 * 
 * @tparam T 
 * @param key 
 * @param value 
 * @return true 
 * @return false : add impossible mainly dur to oversizing.
 */
        template <typename T>
        bool add(std::string key, const T &value)
        {
            auto skv = SanPair<T>(key, value);
            uint16_t currentpos = o_currentjsonstream->tellp();
            uint16_t forcastedjsonsize = skv.forcastedjsonsize();
            if (forcastedjsonsize > JsonMaxCapacity)
                return false; // size of the kv pair oversize the capacity.
            if (currentpos + forcastedjsonsize > JsonMaxCapacity - 1)
                if (!this->flush())
                    return false;
            *o_currentjsonstream << skv;
            return true;
        }

        bool flush()
        {
            if (o_currentjsonstream->tellp() > 1)
                *o_currentjsonstream << '}';
            jsoncollection.emplace_back(o_currentjsonstream->str());
            o_currentjsonstream = new std::ostringstream();
            return true;
        }
    };

    class SanCodedStr
    {
    private:
        std::string _codedstring;

        template <typename T>
        bool extractKV(std::string const &kvpair, SanPair<T> &sanpair)
        {
            std::istringstream ss_kvpair(kvpair);
            std::getline(ss_kvpair, sanpair.first, ':') >> sanpair.second;
            std::string last;
            bool res = !ss_kvpair.fail();
            if (std::getline(ss_kvpair, last, ':'))
                return false;
            return res;
        }

    public:
        SanCodedStr(const char *codedstring)
        {
            _codedstring = std::string(codedstring);
        }
        ~SanCodedStr()
        {
        }
        std::string getCodedString()
        {
            return _codedstring;
        }

        template <typename T>
        SanPair<T> get(const char *key)
        {
            std::istringstream iss(_codedstring);
            std::string coddedkv;
            std::string strkey(key);
            SanPair<T> kv;
            while (std::getline(iss, coddedkv, ';'))
            {
                if (extractKV(coddedkv, kv))
                {
                    if (kv.first == strkey)
                    {
                        return kv;
                    }
                }
            }
            return kv;
        }

        template <typename T>
        bool tryGetValue(const char *key, T &outvalue)
        {
            std::istringstream iss(_codedstring);
            std::string coddedkv;
            std::string strkey(key);
            SanPair<T> kv;
            while (std::getline(iss, coddedkv, ';'))
            {
                if (extractKV(coddedkv, kv))
                {
                    if (kv.first == strkey)
                    {
                        outvalue = kv.second;
                        return true;
                    }
                }
            }
            return false;
        }

        template <typename T>
        bool extractKVmap(std::map<std::string, T> &data)
        {
            std::istringstream iss(_codedstring);
            std::string coddedkv;
            SanPair<T> kv;
            while (std::getline(iss, coddedkv, ';'))
            {
                if (extractKV(coddedkv, kv))
                {
                    data.emplace(kv.first, kv.second);
                }
            }
            return true;
        }
    };
} // namespace SANSENSNODE_NAMESPACE
