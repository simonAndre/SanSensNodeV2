

class SanDataCollector
{
private:
    size_t _buffersize;

public:
    SanDataCollector();
    ~SanDataCollector();
    void reset();
    // add key / value as string
    void Add_s(const char *key, const char *value);
    // add key / value as int
    void Add_i(const char *key, int value);
    // add key / value as float
    void Add_f(const char *key, float value);
    void Add_d(const char *key, double value);
    // add key / value as bool
    void Add_b(const char *key, bool value);
    unsigned int getbufferSize();
    //serialize content to string
    unsigned int Serialize(char *outbuff);
};
